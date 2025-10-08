/**************************************************************************/ /**
                                                                              * @file rule_manager.c
                                                                              * @brief Implementation of high-level rule management for hexhex game
                                                                              *****************************************************************************/

#include "game/rule_manager.h"
#include "game/game.h"
#include "grid/hex_geometry.h"
#include "tile/pool_map.h"
#include "tile/tile_map.h"
#include <assert.h>
#include <stdio.h>

// --- Helper Functions ---

/**
 * @brief Get tile type specific rule behavior
 */
static void get_tile_rule_config(tile_type_t type, float *neighbor_bonus,
                                 tile_type_t *perception_override,
                                 bool *has_perception) {
  *neighbor_bonus = 0.0f;
  *perception_override = TILE_UNDEFINED;
  *has_perception = false;

  switch (type) {
  case TILE_MAGENTA:
    *neighbor_bonus = 1.0f; // +1 per same-color neighbor
    break;

  case TILE_CYAN:
    *perception_override = TILE_CYAN;
    *has_perception = true; // Makes neighbors appear cyan
    break;

  case TILE_YELLOW:
    *neighbor_bonus = 0.5f; // +0.5 per same-color neighbor
    break;

  case TILE_GREEN:
    *neighbor_bonus = 2.0f; // +2 per same-color neighbor
    break;

  default:
    break;
  }
}

/**
 * @brief Count neighbors of a specific type around a tile
 */
static int count_neighbors_of_type(const tile_t *tile, tile_type_t target_type,
                                   const tile_map_t *tile_map,
                                   const rule_context_t *context) {
  grid_cell_t *neighbors;
  size_t neighbor_count;

  hex_get_neighbors(tile->cell, &neighbors, &neighbor_count);

  int count = 0;
  for (size_t i = 0; i < neighbor_count; i++) {
    const tile_t *neighbor = tile_map_get(tile_map, neighbors[i]);
    if (neighbor) {
      tile_type_t perceived_type = neighbor->data.type;

      // Check for perception overrides
      if (context && context->perceived_types) {
        // TODO: Implement perception lookup by cell
        // For now, use actual type
      }

      if (perceived_type == target_type) {
        count++;
      }
    }
  }

  free(neighbors);
  return count;
}

// --- Rule Manager Lifecycle ---

bool rule_manager_init(rule_manager_t *manager, const game_t *game) {
  if (!manager || !game)
    return false;

  memset(manager, 0, sizeof(rule_manager_t));

  if (!rule_registry_init(&manager->registry)) {
    return false;
  }

  if (!rule_context_init(&manager->context, game->board)) {
    rule_registry_cleanup(&manager->registry);
    return false;
  }

  manager->initialized = true;
  manager->dirty = false;
  manager->evaluations_this_frame = 0;
  manager->total_evaluations = 0;

  return true;
}

void rule_manager_cleanup(rule_manager_t *manager) {
  if (!manager || !manager->initialized)
    return;

  rule_context_cleanup(&manager->context);
  rule_registry_cleanup(&manager->registry);

  memset(manager, 0, sizeof(rule_manager_t));
}

void rule_manager_mark_dirty(rule_manager_t *manager) {
  if (manager) {
    manager->dirty = true;
  }
}

// --- Tile-Based Rule Creation ---

void rule_manager_on_tile_placed(rule_manager_t *manager, const tile_t *tile,
                                 const board_t *board) {
  if (!manager || !tile || !board)
    return;

  rule_manager_create_tile_rules(manager, tile, board);
  rule_manager_mark_dirty(manager);
}

void rule_manager_on_tile_removed(rule_manager_t *manager, const tile_t *tile) {
  if (!manager || !tile)
    return;

  rule_registry_remove_by_source(&manager->registry, tile->cell, true);
  rule_manager_mark_dirty(manager);
}

void rule_manager_on_pool_changed(rule_manager_t *manager, const pool_t *pool,
                                  const board_t *board) {
  if (!manager || !pool || !board)
    return;

  // Remove old pool rules first
  rule_registry_remove_by_source(
    &manager->registry, (grid_cell_t){pool->id, 0}, // Use pool ID as coordinate
    false);

  rule_manager_create_pool_rules(manager, pool, board);
  rule_manager_mark_dirty(manager);
}

void rule_manager_on_pool_destroyed(rule_manager_t *manager,
                                    const pool_t *pool) {
  if (!manager || !pool)
    return;

  rule_registry_remove_by_source(&manager->registry, (grid_cell_t){pool->id, 0},
                                 false);
  rule_manager_mark_dirty(manager);
}

// --- Production Calculation Integration ---

float rule_manager_calculate_tile_production(rule_manager_t *manager,
                                             const tile_t *tile,
                                             const board_t *board) {
  if (!manager || !tile || !board)
    return tile_get_effective_production(tile);

  float base_production = tile_get_effective_production(tile);
  float rule_bonus = 0.0f;

  // Set context for this tile
  manager->context.current_tile = tile;
  manager->context.current_cell = tile->cell;
  manager->context.board = board;

  // Get rules affecting this tile
  rule_t **rules;
  size_t rule_count;
  rule_registry_get_rules_for_cell(&manager->registry, tile->cell, &rules,
                                   &rule_count);

  // Apply calculation phase rules
  for (size_t i = 0; i < rule_count; i++) {
    rule_t *rule = rules[i];
    if (rule->lifecycle == RULE_LIFECYCLE_PERSISTENT &&
        rule->phase == RULE_PHASE_CALCULATION &&
        rule_check_condition(rule, &manager->context)) {

      // Calculate rule bonus based on type
      switch (rule->effect.type) {
      case RULE_EFFECT_ADD_FLAT:
        if (rule->condition.type == RULE_CONDITION_NEIGHBOR_TYPE) {
          // Count neighbors and multiply by bonus
          int neighbor_count = count_neighbors_of_type(
            tile, rule->condition.params.neighbor_type.required_type,
            board->tiles, &manager->context);
          rule_bonus += rule->effect.params.flat_value * neighbor_count;
        } else {
          rule_bonus += rule->effect.params.flat_value;
        }
        break;

      case RULE_EFFECT_ADD_PERCENT:
        rule_bonus +=
          base_production * (rule->effect.params.percentage / 100.0f);
        break;

      case RULE_EFFECT_MULTIPLY:
        rule_bonus += base_production * (rule->effect.params.multiplier - 1.0f);
        break;

      default:
        break;
      }
    }
  }

  free(rules);
  manager->evaluations_this_frame++;
  manager->total_evaluations++;

  return base_production + rule_bonus;
}

float rule_manager_calculate_pool_modifier(rule_manager_t *manager,
                                           const pool_t *pool,
                                           const board_t *board) {
  if (!manager || !pool || !board)
    return pool_get_modifier(pool);

  float base_modifier = pool_get_modifier(pool);
  float rule_bonus = 0.0f;

  // Set context for this pool
  manager->context.current_pool = pool;
  manager->context.current_cell = (grid_cell_t){pool->id, 0};
  manager->context.board = board;

  // Get rules affecting this pool (using pool ID as coordinate)
  rule_t **rules;
  size_t rule_count;
  rule_registry_get_rules_for_cell(
    &manager->registry, (grid_cell_t){pool->id, 0}, &rules, &rule_count);

  // Apply calculation phase rules
  for (size_t i = 0; i < rule_count; i++) {
    rule_t *rule = rules[i];
    if (rule->lifecycle == RULE_LIFECYCLE_PERSISTENT &&
        rule->phase == RULE_PHASE_CALCULATION &&
        rule_check_condition(rule, &manager->context)) {

      switch (rule->effect.type) {
      case RULE_EFFECT_ADD_FLAT:
        rule_bonus += rule->effect.params.flat_value;
        break;

      case RULE_EFFECT_ADD_PERCENT:
        rule_bonus += base_modifier * (rule->effect.params.percentage / 100.0f);
        break;

      case RULE_EFFECT_MULTIPLY:
        rule_bonus += base_modifier * (rule->effect.params.multiplier - 1.0f);
        break;

      default:
        break;
      }
    }
  }

  free(rules);
  manager->evaluations_this_frame++;
  manager->total_evaluations++;

  return base_modifier + rule_bonus;
}

void rule_manager_update_production(rule_manager_t *manager, board_t *board) {
  if (!manager || !board)
    return;

  // Reset frame counter
  manager->evaluations_this_frame = 0;

  // Execute any pending instant rules first
  rule_execute_instant_rules(&manager->registry, &manager->context);

  // Apply perception phase rules
  rule_evaluate_phase(&manager->registry, &manager->context,
                      RULE_PHASE_PERCEPTION);

  // Apply calculation phase rules (done per-tile in calculate functions)

  manager->dirty = false;
}

// --- Specific Rule Factories ---

void rule_manager_create_tile_rules(rule_manager_t *manager, const tile_t *tile,
                                    const board_t *board) {
  if (!manager || !tile || !board)
    return;

  float neighbor_bonus;
  tile_type_t perception_override;
  bool has_perception;

  get_tile_rule_config(tile->data.type, &neighbor_bonus, &perception_override,
                       &has_perception);

  // Add neighbor bonus rule if applicable
  if (neighbor_bonus > 0.0f) {
    rule_t rule =
      rule_factory_neighbor_bonus(tile->cell, tile->data.type, neighbor_bonus);
    rule_registry_add(&manager->registry, &rule);
  }

  // Add perception rule if applicable
  if (has_perception) {
    rule_t rule =
      rule_factory_perception_override(tile->cell, perception_override,
                                       1); // Range of 1
    rule_registry_add(&manager->registry, &rule);
  }
}

void rule_manager_create_pool_rules(rule_manager_t *manager, const pool_t *pool,
                                    const board_t *board) {
  if (!manager || !pool || !board)
    return;

  // Get pool size from tile count
  size_t pool_size = tile_map_size(pool->tiles);

  // Add pool size bonus for larger pools
  if (pool_size >= 5) {
    rule_t rule = rule_factory_pool_size_bonus((grid_cell_t){pool->id, 0},
                                               5,      // Min size
                                               20.0f); // 20% bonus
    rule_registry_add(&manager->registry, &rule);
  }

  // Add compactness bonus for well-formed pools
  if (pool->compactness_score > 0.8f) {
    rule_t rule = rule_factory_instant_modifier((grid_cell_t){pool->id, 0},
                                                RULE_TARGET_MODIFIER,
                                                0.5f); // +0.5 modifier
    rule_registry_add(&manager->registry, &rule);
  }
}

// --- Example Rule Definitions ---

void rule_manager_add_neighbor_bonus_rule(rule_manager_t *manager,
                                          const tile_t *source_tile,
                                          float bonus_per_neighbor) {
  if (!manager || !source_tile)
    return;

  rule_t rule = rule_factory_neighbor_bonus(
    source_tile->cell, source_tile->data.type, bonus_per_neighbor);
  rule_registry_add(&manager->registry, &rule);
  rule_manager_mark_dirty(manager);
}

void rule_manager_add_perception_rule(rule_manager_t *manager,
                                      const tile_t *source_tile,
                                      tile_type_t override_type) {
  if (!manager || !source_tile)
    return;

  rule_t rule =
    rule_factory_perception_override(source_tile->cell, override_type,
                                     1); // Range of 1
  rule_registry_add(&manager->registry, &rule);
  rule_manager_mark_dirty(manager);
}

void rule_manager_add_pool_size_rule(rule_manager_t *manager,
                                     const pool_t *source_pool, int min_size,
                                     float bonus_percentage) {
  if (!manager || !source_pool)
    return;

  rule_t rule = rule_factory_pool_size_bonus((grid_cell_t){source_pool->id, 0},
                                             min_size, bonus_percentage);
  rule_registry_add(&manager->registry, &rule);
  rule_manager_mark_dirty(manager);
}

void rule_manager_add_instant_modifier(rule_manager_t *manager,
                                       grid_cell_t target_cell,
                                       rule_target_t target, float amount) {
  if (!manager)
    return;

  rule_t rule = rule_factory_instant_modifier(target_cell, target, amount);
  rule_registry_add(&manager->registry, &rule);
  rule_manager_mark_dirty(manager);
}

// --- Query Functions ---

void rule_manager_get_rules_for_cell(const rule_manager_t *manager,
                                     grid_cell_t cell, rule_t ***out_rules,
                                     size_t *out_count) {
  if (!manager || !out_rules || !out_count) {
    if (out_rules)
      *out_rules = NULL;
    if (out_count)
      *out_count = 0;
    return;
  }

  rule_registry_get_rules_for_cell(&manager->registry, cell, out_rules,
                                   out_count);
}

size_t rule_manager_get_rule_count(const rule_manager_t *manager) {
  if (!manager)
    return 0;

  return manager->registry.rule_count;
}

bool rule_manager_needs_evaluation(const rule_manager_t *manager) {
  if (!manager)
    return false;

  return manager->dirty;
}

// --- Debug Functions ---

void rule_manager_print_all_rules(const rule_manager_t *manager) {
  if (!manager) {
    printf("Rule manager is NULL\n");
    return;
  }

  printf("=== All Rules ===\n");
  for (size_t i = 0; i < manager->registry.rule_count; i++) {
    rule_print(manager->registry.all_rules[i]);
    printf("\n");
  }
}

void rule_manager_print_rules_for_cell(const rule_manager_t *manager,
                                       grid_cell_t cell) {
  if (!manager)
    return;

  rule_t **rules;
  size_t rule_count;
  rule_manager_get_rules_for_cell(manager, cell, &rules, &rule_count);

  printf("=== Rules for Cell (%d, %d) ===\n", cell.q, cell.r);
  printf("Found %zu rules:\n", rule_count);

  for (size_t i = 0; i < rule_count; i++) {
    rule_print(rules[i]);
    printf("\n");
  }

  free(rules);
}

void rule_manager_print_stats(const rule_manager_t *manager) {
  if (!manager) {
    printf("Rule manager is NULL\n");
    return;
  }

  printf("=== Rule Manager Statistics ===\n");
  printf("Initialized: %s\n", manager->initialized ? "Yes" : "No");
  printf("Dirty: %s\n", manager->dirty ? "Yes" : "No");
  printf("Evaluations this frame: %u\n", manager->evaluations_this_frame);
  printf("Total evaluations: %u\n", manager->total_evaluations);
  printf("\n");

  rule_registry_print_stats(&manager->registry);
}

void rule_manager_reset_stats(rule_manager_t *manager) {
  if (!manager)
    return;

  manager->evaluations_this_frame = 0;
  manager->total_evaluations = 0;
}
