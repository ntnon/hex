/**************************************************************************/ /**
                                                                              * @file rule_system.c
                                                                              * @brief Implementation of comprehensive rule system for hexhex game
                                                                              *****************************************************************************/

#include "game/rule_system.h"
#include "game/board.h"
#include "grid/grid_geometry.h"
#include "tile/pool.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

// --- Internal Helper Functions ---

/**
 * @brief Resize rule array if needed
 */
static bool rule_registry_ensure_capacity(rule_registry_t *registry) {
  if (registry->rule_count >= registry->rule_capacity) {
    size_t new_capacity =
      registry->rule_capacity == 0 ? 32 : registry->rule_capacity * 2;
    rule_t **new_rules =
      realloc(registry->all_rules, new_capacity * sizeof(rule_t *));
    if (!new_rules) {
      return false;
    }
    registry->all_rules = new_rules;
    registry->rule_capacity = new_capacity;
  }
  return true;
}

/**
 * @brief Calculate affected cells for a rule based on scope
 */
static void rule_calculate_affected_cells(rule_t *rule, const board_t *board) {
  switch (rule->scope) {
  case RULE_SCOPE_TILE:
    // Only affects the source cell
    rule->affected_cells = malloc(sizeof(grid_cell_t));
    if (rule->affected_cells) {
      rule->affected_cells[0] = rule->source_cell;
      rule->affected_count = 1;
    }
    break;

  case RULE_SCOPE_NEIGHBORS: {
    // Get neighbors of source cell
    grid_cell_t *neighbors;
    size_t neighbor_count;
    grid_get_neighbors(tile->cell, &neighbors, &neighbor_count);

    rule->affected_cells = neighbors;
    rule->affected_count = neighbor_count;
    break;
  }

  case RULE_SCOPE_POOL:
    // TODO: Get all cells in the pool containing source_cell
    rule->affected_cells = malloc(sizeof(grid_cell_t));
    if (rule->affected_cells) {
      rule->affected_cells[0] = rule->source_cell;
      rule->affected_count = 1;
    }
    break;

  case RULE_SCOPE_GLOBAL:
    // Global rules don't need spatial indexing
    rule->affected_cells = NULL;
    rule->affected_count = 0;
    break;
  }
}

// --- Rule Registry Implementation ---

bool rule_registry_init(rule_registry_t *registry) {
  if (!registry)
    return false;

  memset(registry, 0, sizeof(rule_registry_t));
  registry->next_rule_id = 1; // Start IDs at 1 (0 reserved for invalid)

  return true;
}

void rule_registry_cleanup(rule_registry_t *registry) {
  if (!registry)
    return;

  // Free all rules
  for (size_t i = 0; i < registry->rule_count; i++) {
    rule_t *rule = registry->all_rules[i];
    if (rule) {
      free(rule->affected_cells);
      free(rule);
    }
  }

  // Free arrays
  free(registry->all_rules);

  // Clear hash buckets
  memset(registry->buckets, 0, sizeof(registry->buckets));

  memset(registry, 0, sizeof(rule_registry_t));
}

uint32_t rule_registry_add(rule_registry_t *registry, const rule_t *rule) {
  if (!registry || !rule)
    return 0;

  // Ensure capacity
  if (!rule_registry_ensure_capacity(registry)) {
    return 0;
  }

  // Create rule copy
  rule_t *new_rule = malloc(sizeof(rule_t));
  if (!new_rule)
    return 0;

  memcpy(new_rule, rule, sizeof(rule_t));
  new_rule->id = registry->next_rule_id++;
  new_rule->next = NULL;
  new_rule->affected_cells = NULL;
  new_rule->affected_count = 0;

  // Calculate affected cells for spatial indexing
  rule_calculate_affected_cells(new_rule, NULL); // TODO: Pass board

  // Add to main array
  registry->all_rules[registry->rule_count] = new_rule;
  registry->rule_count++;

  // Add to spatial hash buckets
  if (new_rule->affected_cells) {
    for (size_t i = 0; i < new_rule->affected_count; i++) {
      uint32_t hash = rule_hash_cell(new_rule->affected_cells[i]);
      uint32_t bucket_idx = hash % RULE_HASH_SIZE;

      // Add to front of bucket chain
      rule_t *old_head = registry->buckets[bucket_idx];
      registry->buckets[bucket_idx] = new_rule;
      new_rule->next = old_head;
    }
  }

  return new_rule->id;
}

bool rule_registry_remove(rule_registry_t *registry, uint32_t rule_id) {
  if (!registry || rule_id == 0)
    return false;

  // Find rule in main array
  size_t rule_idx = SIZE_MAX;
  rule_t *target_rule = NULL;

  for (size_t i = 0; i < registry->rule_count; i++) {
    if (registry->all_rules[i]->id == rule_id) {
      rule_idx = i;
      target_rule = registry->all_rules[i];
      break;
    }
  }

  if (!target_rule)
    return false;

  // Remove from hash buckets
  if (target_rule->affected_cells) {
    for (size_t i = 0; i < target_rule->affected_count; i++) {
      uint32_t hash = rule_hash_cell(target_rule->affected_cells[i]);
      uint32_t bucket_idx = hash % RULE_HASH_SIZE;

      rule_t **current = &registry->buckets[bucket_idx];
      while (*current) {
        if (*current == target_rule) {
          *current = (*current)->next;
          break;
        }
        current = &(*current)->next;
      }
    }
  }

  // Remove from main array (swap with last)
  registry->rule_count--;
  if (rule_idx < registry->rule_count) {
    registry->all_rules[rule_idx] = registry->all_rules[registry->rule_count];
  }
  registry->all_rules[registry->rule_count] = NULL;

  // Free rule memory
  free(target_rule->affected_cells);
  free(target_rule);

  return true;
}

void rule_registry_get_rules_for_cell(const rule_registry_t *registry,
                                      grid_cell_t cell, rule_t ***out_rules,
                                      size_t *out_count) {
  if (!registry || !out_rules || !out_count)
    return;

  *out_rules = NULL;
  *out_count = 0;

  uint32_t hash = rule_hash_cell(cell);
  uint32_t bucket_idx = hash % RULE_HASH_SIZE;

  // Count rules in bucket that affect this cell
  size_t count = 0;
  rule_t *current = registry->buckets[bucket_idx];
  while (current) {
    // Check if rule actually affects this cell
    bool affects_cell = false;
    for (size_t i = 0; i < current->affected_count; i++) {
      if (current->affected_cells[i].coord.hex.q == cell.coord.hex.q &&
          current->affected_cells[i].coord.hex.r == cell.coord.hex.r) {
        affects_cell = true;
        break;
      }
    }
    if (affects_cell)
      count++;
    current = current->next;
  }

  if (count == 0)
    return;

  // Allocate array and populate
  rule_t **rules = malloc(count * sizeof(rule_t *));
  if (!rules)
    return;

  size_t idx = 0;
  current = registry->buckets[bucket_idx];
  while (current && idx < count) {
    bool affects_cell = false;
    for (size_t i = 0; i < current->affected_count; i++) {
      if (current->affected_cells[i].coord.hex.q == cell.coord.hex.q &&
          current->affected_cells[i].coord.hex.r == cell.coord.hex.r) {
        affects_cell = true;
        break;
      }
    }
    if (affects_cell) {
      rules[idx++] = current;
    }
    current = current->next;
  }

  *out_rules = rules;
  *out_count = idx;
}

void rule_registry_remove_by_source(rule_registry_t *registry,
                                    grid_cell_t source_cell,
                                    bool is_tile_source) {
  if (!registry)
    return;

  // Build list of rule IDs to remove
  uint32_t *to_remove = malloc(registry->rule_count * sizeof(uint32_t));
  size_t remove_count = 0;

  for (size_t i = 0; i < registry->rule_count; i++) {
    rule_t *rule = registry->all_rules[i];
    if (rule->source_cell.coord.hex.q == source_cell.coord.hex.q &&
        rule->source_cell.coord.hex.r == source_cell.coord.hex.r &&
        rule->is_tile_source == is_tile_source) {
      to_remove[remove_count++] = rule->id;
    }
  }

  // Remove rules
  for (size_t i = 0; i < remove_count; i++) {
    rule_registry_remove(registry, to_remove[i]);
  }

  free(to_remove);
}

// --- Rule Factory Implementation ---

rule_t rule_factory_neighbor_bonus(grid_cell_t source_cell,
                                   tile_type_t neighbor_type,
                                   float bonus_per_neighbor) {
  rule_t rule = {0};

  rule.lifecycle = RULE_LIFECYCLE_PERSISTENT;
  rule.phase = RULE_PHASE_CALCULATION;
  rule.scope = RULE_SCOPE_TILE;
  rule.target = RULE_TARGET_MODIFIER;

  rule.condition.type = RULE_CONDITION_NEIGHBOR_TYPE;
  rule.condition.params.neighbor_type.required_type = neighbor_type;

  rule.effect.type = RULE_EFFECT_ADD_FLAT;
  rule.effect.params.flat_value = bonus_per_neighbor;

  rule.source_cell = source_cell;
  rule.is_tile_source = true;
  rule.priority = 100; // Default priority

  return rule;
}

rule_t rule_factory_perception_override(grid_cell_t source_cell,
                                        tile_type_t override_type, int range) {
  rule_t rule = {0};

  rule.lifecycle = RULE_LIFECYCLE_PERSISTENT;
  rule.phase = RULE_PHASE_PERCEPTION;
  rule.scope = RULE_SCOPE_NEIGHBORS;
  rule.target = RULE_TARGET_PERCEPTION;

  rule.condition.type = RULE_CONDITION_ALWAYS;

  rule.effect.type = RULE_EFFECT_OVERRIDE_TYPE;
  rule.effect.params.override_type = override_type;

  rule.source_cell = source_cell;
  rule.is_tile_source = true;
  rule.priority = 50; // Earlier priority for perception

  return rule;
}

rule_t rule_factory_pool_size_bonus(grid_cell_t source_cell, int min_size,
                                    float bonus_percentage) {
  rule_t rule = {0};

  rule.lifecycle = RULE_LIFECYCLE_PERSISTENT;
  rule.phase = RULE_PHASE_CALCULATION;
  rule.scope = RULE_SCOPE_POOL;
  rule.target = RULE_TARGET_MODIFIER;

  rule.condition.type = RULE_CONDITION_POOL_SIZE;
  rule.condition.params.pool_size.min_size = min_size;
  rule.condition.params.pool_size.max_size = INT32_MAX;

  rule.effect.type = RULE_EFFECT_ADD_PERCENT;
  rule.effect.params.percentage = bonus_percentage;

  rule.source_cell = source_cell;
  rule.is_tile_source = false; // Pool source
  rule.priority = 100;

  return rule;
}

rule_t rule_factory_instant_modifier(grid_cell_t source_cell,
                                     rule_target_t target, float flat_bonus) {
  rule_t rule = {0};

  rule.lifecycle = RULE_LIFECYCLE_INSTANT;
  rule.phase = RULE_PHASE_CALCULATION; // Not used for instant rules
  rule.scope = RULE_SCOPE_TILE;
  rule.target = target;

  rule.condition.type = RULE_CONDITION_ALWAYS;

  rule.effect.type = RULE_EFFECT_ADD_FLAT;
  rule.effect.params.flat_value = flat_bonus;

  rule.source_cell = source_cell;
  rule.is_tile_source = true;
  rule.priority = 0; // Instant rules execute immediately

  return rule;
}

// --- Rule Evaluation Implementation ---

bool rule_context_init(rule_context_t *context, const board_t *board) {
  if (!context || !board)
    return false;

  memset(context, 0, sizeof(rule_context_t));
  context->board = board;

  // Initialize perception arrays (allocate for max possible tiles)
  context->perception_capacity = 10000; // TODO: Calculate based on board size
  context->perceived_types =
    calloc(context->perception_capacity, sizeof(tile_type_t));
  context->perceived_values =
    calloc(context->perception_capacity, sizeof(float));

  if (!context->perceived_types || !context->perceived_values) {
    rule_context_cleanup(context);
    return false;
  }

  return true;
}

void rule_context_cleanup(rule_context_t *context) {
  if (!context)
    return;

  free(context->perceived_types);
  free(context->perceived_values);
  memset(context, 0, sizeof(rule_context_t));
}

bool rule_check_condition(const rule_t *rule, const rule_context_t *context) {
  if (!rule || !context)
    return false;

  switch (rule->condition.type) {
  case RULE_CONDITION_ALWAYS:
    return true;

  case RULE_CONDITION_NEIGHBOR_TYPE: {
    // TODO: Implement neighbor type checking
    // This would require getting neighbors and checking their types
    return true; // Placeholder
  }

  case RULE_CONDITION_NEIGHBOR_COUNT: {
    // TODO: Implement neighbor counting
    return true; // Placeholder
  }

  case RULE_CONDITION_POOL_SIZE: {
    if (!context->current_pool)
      return false;

    // TODO: Get pool size from pool structure
    int pool_size = 1; // Placeholder

    return pool_size >= rule->condition.params.pool_size.min_size &&
           pool_size <= rule->condition.params.pool_size.max_size;
  }

  case RULE_CONDITION_GEOMETRIC: {
    // TODO: Implement geometric conditions
    return true; // Placeholder
  }

  case RULE_CONDITION_VALUE_THRESHOLD: {
    if (!context->current_tile)
      return false;

    float value = tile_get_effective_production(context->current_tile);
    if (rule->condition.params.value_threshold.greater_than) {
      return value > rule->condition.params.value_threshold.threshold;
    } else {
      return value < rule->condition.params.value_threshold.threshold;
    }
  }

  default:
    return false;
  }
}

void rule_apply_effect(const rule_t *rule, rule_context_t *context) {
  if (!rule || !context)
    return;

  switch (rule->effect.type) {
  case RULE_EFFECT_ADD_FLAT: {
    if (rule->target == RULE_TARGET_MODIFIER && context->current_tile) {
      // For non-const modification, we'd need a different approach
      // This is a placeholder showing the concept
    }
    break;
  }

  case RULE_EFFECT_ADD_PERCENT: {
    // TODO: Implement percentage effects
    break;
  }

  case RULE_EFFECT_MULTIPLY: {
    // TODO: Implement multiplication effects
    break;
  }

  case RULE_EFFECT_SET_VALUE: {
    // TODO: Implement set value effects
    break;
  }

  case RULE_EFFECT_OVERRIDE_TYPE: {
    if (rule->target == RULE_TARGET_PERCEPTION) {
      // TODO: Update perception arrays
    }
    break;
  }

  case RULE_EFFECT_EXTEND_RANGE: {
    // TODO: Implement range extension
    break;
  }

  default:
    break;
  }
}

void rule_evaluate_phase(const rule_registry_t *registry,
                         rule_context_t *context, rule_phase_t phase) {
  if (!registry || !context)
    return;

  // Evaluate all persistent rules for the given phase
  for (size_t i = 0; i < registry->rule_count; i++) {
    rule_t *rule = registry->all_rules[i];

    if (rule->lifecycle == RULE_LIFECYCLE_PERSISTENT && rule->phase == phase) {

      if (rule_check_condition(rule, context)) {
        rule_apply_effect(rule, context);
      }
    }
  }
}

void rule_execute_instant_rules(rule_registry_t *registry,
                                rule_context_t *context) {
  if (!registry || !context)
    return;

  // Build list of instant rules to execute and remove
  uint32_t *to_execute = malloc(registry->rule_count * sizeof(uint32_t));
  size_t execute_count = 0;

  for (size_t i = 0; i < registry->rule_count; i++) {
    rule_t *rule = registry->all_rules[i];
    if (rule->lifecycle == RULE_LIFECYCLE_INSTANT && !rule->executed) {
      if (rule_check_condition(rule, context)) {
        rule_apply_effect(rule, context);
        rule->executed = true;
        to_execute[execute_count++] = rule->id;
      }
    }
  }

  // Remove executed instant rules
  for (size_t i = 0; i < execute_count; i++) {
    rule_registry_remove(registry, to_execute[i]);
  }

  free(to_execute);
}

// --- Utility Implementation ---

uint32_t rule_hash_cell(grid_cell_t cell) {
  // Simple hash combining q and r coordinates
  uint32_t hash = 0;
  hash ^= (uint32_t)cell.coord.hex.q + 0x9e3779b9 + (hash << 6) + (hash >> 2);
  hash ^= (uint32_t)cell.coord.hex.r + 0x9e3779b9 + (hash << 6) + (hash >> 2);
  return hash;
}

void rule_print(const rule_t *rule) {
  if (!rule)
    return;

  printf("Rule ID: %u\n", rule->id);
  printf("  Lifecycle: %s\n",
         rule->lifecycle == RULE_LIFECYCLE_INSTANT ? "Instant" : "Persistent");
  printf("  Phase: %s\n",
         rule->phase == RULE_PHASE_PERCEPTION ? "Perception" : "Calculation");
  printf("  Source: (%d, %d) %s\n", rule->source_cell.coord.hex.q,
         rule->source_cell.coord.hex.r,
         rule->is_tile_source ? "[Tile]" : "[Pool]");
  printf("  Priority: %d\n", rule->priority);
  printf("  Executed: %s\n", rule->executed ? "Yes" : "No");
}

void rule_registry_print_stats(const rule_registry_t *registry) {
  if (!registry)
    return;

  printf("Rule Registry Statistics:\n");
  printf("  Total rules: %zu\n", registry->rule_count);
  printf("  Capacity: %zu\n", registry->rule_capacity);
  printf("  Next ID: %u\n", registry->next_rule_id);

  size_t instant_count = 0, persistent_count = 0;
  size_t perception_count = 0, calculation_count = 0;

  for (size_t i = 0; i < registry->rule_count; i++) {
    rule_t *rule = registry->all_rules[i];
    if (rule->lifecycle == RULE_LIFECYCLE_INSTANT) {
      instant_count++;
    } else {
      persistent_count++;
      if (rule->phase == RULE_PHASE_PERCEPTION) {
        perception_count++;
      } else {
        calculation_count++;
      }
    }
  }

  printf("  Instant rules: %zu\n", instant_count);
  printf("  Persistent rules: %zu\n", persistent_count);
  printf("    Perception: %zu\n", perception_count);
  printf("    Calculation: %zu\n", calculation_count);
}
