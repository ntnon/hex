/**************************************************************************/ /**
                                                                              * @file rule_system.c
                                                                              * @brief Implementation of player-driven rule system with priority-based evaluation
                                                                              *****************************************************************************/

#include "game/rule_system.h"
#include "grid/hex_geometry.h"
#include "tile/tile_map.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- Helper Functions ---

static int rule_priority_compare(const void *a, const void *b) {
  const rule_t *rule_a = *(const rule_t **)a;
  const rule_t *rule_b = *(const rule_t **)b;
  return rule_a->priority - rule_b->priority;
}

static bool rule_registry_ensure_capacity(rule_registry_t *registry) {
  if (registry->rule_count >= registry->rule_capacity) {
    size_t new_capacity = registry->rule_capacity * 2;
    if (new_capacity == 0)
      new_capacity = 16;

    rule_t **new_rules =
      realloc(registry->all_rules, new_capacity * sizeof(rule_t *));
    if (!new_rules)
      return false;

    registry->all_rules = new_rules;
    registry->rule_capacity = new_capacity;
  }
  return true;
}

static void rule_calculate_affected_cells(rule_t *rule,
                                          const tile_t *source_tile) {
  if (!rule || !source_tile)
    return;

  switch (rule->scope) {
  case RULE_SCOPE_SELF: {
    rule->affected_cells = malloc(sizeof(grid_cell_t));
    if (rule->affected_cells) {
      rule->affected_cells[0] = source_tile->cell;
      rule->affected_count = 1;
    }
    break;
  }

  case RULE_SCOPE_NEIGHBORS: {
    grid_cell_t *neighbors;
    size_t neighbor_count;
    hex_get_neighbors(source_tile->cell, &neighbors, &neighbor_count);

    rule->affected_cells = neighbors;
    rule->affected_count = neighbor_count;
    break;
  }

  case RULE_SCOPE_POOL: {
    // TODO: Get all cells in the same pool
    rule->affected_cells = malloc(sizeof(grid_cell_t));
    if (rule->affected_cells) {
      rule->affected_cells[0] = source_tile->cell;
      rule->affected_count = 1;
    }
    break;
  }

  case RULE_SCOPE_TYPE_GLOBAL:
  case RULE_SCOPE_GLOBAL: {
    // Global rules don't need specific affected cells
    rule->affected_cells = NULL;
    rule->affected_count = 0;
    break;
  }
  }
}

// --- Rule Registry Implementation ---

bool rule_registry_init(rule_registry_t *registry) {
  if (!registry)
    return false;

  memset(registry->buckets, 0, sizeof(registry->buckets));
  registry->all_rules = NULL;
  registry->rule_count = 0;
  registry->rule_capacity = 0;
  registry->next_rule_id = 1;

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
      free(rule->effects);
      free(rule);
    }
  }

  free(registry->all_rules);
  memset(registry, 0, sizeof(*registry));
}

uint32_t rule_registry_add(rule_registry_t *registry,
                           const rule_t *rule_template) {
  if (!registry || !rule_template)
    return 0;

  if (!rule_registry_ensure_capacity(registry))
    return 0;

  // Create a copy of the rule
  rule_t *rule = malloc(sizeof(rule_t));
  if (!rule)
    return 0;

  *rule = *rule_template;
  rule->id = registry->next_rule_id++;
  rule->is_active = true;
  rule->executed = false;

  // Copy effects array
  if (rule_template->effects && rule_template->effect_count > 0) {
    rule->effects = malloc(rule_template->effect_count * sizeof(rule_effect_t));
    if (!rule->effects) {
      free(rule);
      return 0;
    }
    memcpy(rule->effects, rule_template->effects,
           rule_template->effect_count * sizeof(rule_effect_t));
  }

  // Add to registry
  registry->all_rules[registry->rule_count] = rule;
  registry->rule_count++;

  // Add to spatial hash if needed
  if (rule->affected_cells) {
    for (size_t i = 0; i < rule->affected_count; i++) {
      uint32_t hash = rule_hash_cell(rule->affected_cells[i]);
      uint32_t bucket = hash % RULE_HASH_SIZE;

      rule->next = registry->buckets[bucket];
      registry->buckets[bucket] = rule;
    }
  }

  return rule->id;
}

bool rule_registry_remove(rule_registry_t *registry, uint32_t rule_id) {
  if (!registry)
    return false;

  // Find and remove from all_rules array
  for (size_t i = 0; i < registry->rule_count; i++) {
    if (registry->all_rules[i]->id == rule_id) {
      rule_t *rule = registry->all_rules[i];

      // Remove from spatial hash
      if (rule->affected_cells) {
        for (size_t j = 0; j < rule->affected_count; j++) {
          uint32_t hash = rule_hash_cell(rule->affected_cells[j]);
          uint32_t bucket = hash % RULE_HASH_SIZE;

          rule_t **current = &registry->buckets[bucket];
          while (*current) {
            if (*current == rule) {
              *current = (*current)->next;
              break;
            }
            current = &(*current)->next;
          }
        }
      }

      // Free rule memory
      free(rule->affected_cells);
      free(rule->effects);
      free(rule);

      // Shift array elements
      for (size_t j = i; j < registry->rule_count - 1; j++) {
        registry->all_rules[j] = registry->all_rules[j + 1];
      }
      registry->rule_count--;

      return true;
    }
  }

  return false;
}

void rule_registry_get_rules_for_cell(const rule_registry_t *registry,
                                      grid_cell_t cell, rule_t ***out_rules,
                                      size_t *out_count) {
  if (!registry || !out_rules || !out_count) {
    if (out_rules)
      *out_rules = NULL;
    if (out_count)
      *out_count = 0;
    return;
  }

  uint32_t hash = rule_hash_cell(cell);
  uint32_t bucket = hash % RULE_HASH_SIZE;

  // Count matching rules
  size_t count = 0;
  rule_t *current = registry->buckets[bucket];
  while (current) {
    if (current->is_active) {
      count++;
    }
    current = current->next;
  }

  if (count == 0) {
    *out_rules = NULL;
    *out_count = 0;
    return;
  }

  // Allocate array for results
  rule_t **rules = malloc(count * sizeof(rule_t *));
  if (!rules) {
    *out_rules = NULL;
    *out_count = 0;
    return;
  }

  // Fill array with matching rules
  size_t index = 0;
  current = registry->buckets[bucket];
  while (current && index < count) {
    if (current->is_active) {
      rules[index++] = current;
    }
    current = current->next;
  }

  // Sort by priority
  qsort(rules, count, sizeof(rule_t *), rule_priority_compare);

  *out_rules = rules;
  *out_count = count;
}

void rule_registry_remove_by_source(rule_registry_t *registry,
                                    grid_cell_t source_cell,
                                    bool is_tile_source) {
  if (!registry)
    return;

  // Mark rules for removal (can't remove during iteration)
  for (size_t i = 0; i < registry->rule_count; i++) {
    rule_t *rule = registry->all_rules[i];
    if (rule->source_cell.coord.hex.q == source_cell.coord.hex.q &&
        rule->source_cell.coord.hex.r == source_cell.coord.hex.r &&
        rule->is_tile_source == is_tile_source) {
      rule->is_active = false;
    }
  }

  // Actually remove inactive rules
  for (size_t i = 0; i < registry->rule_count;) {
    if (!registry->all_rules[i]->is_active) {
      rule_registry_remove(registry, registry->all_rules[i]->id);
    } else {
      i++;
    }
  }
}

// --- Rule Catalog Implementation ---

bool rule_catalog_init(rule_catalog_t *catalog, size_t player_count) {
  if (!catalog || player_count == 0)
    return false;

  catalog->rule_catalog = NULL;
  catalog->catalog_size = 0;
  catalog->player_count = player_count;

  // Allocate per-player arrays
  catalog->player_available = calloc(player_count, sizeof(player_rule_t *));
  catalog->player_active = calloc(player_count, sizeof(player_rule_t *));
  catalog->player_available_count = calloc(player_count, sizeof(size_t));
  catalog->player_active_count = calloc(player_count, sizeof(size_t));
  catalog->player_rule_points = calloc(player_count, sizeof(int));

  if (!catalog->player_available || !catalog->player_active ||
      !catalog->player_available_count || !catalog->player_active_count ||
      !catalog->player_rule_points) {
    rule_catalog_cleanup(catalog);
    return false;
  }

  // Initialize rule points for each player
  for (size_t i = 0; i < player_count; i++) {
    catalog->player_rule_points[i] = 100; // Starting rule points
  }

  return true;
}

void rule_catalog_cleanup(rule_catalog_t *catalog) {
  if (!catalog)
    return;

  free(catalog->rule_catalog);
  free(catalog->player_available);
  free(catalog->player_active);
  free(catalog->player_available_count);
  free(catalog->player_active_count);
  free(catalog->player_rule_points);

  memset(catalog, 0, sizeof(*catalog));
}

bool rule_catalog_add_rule(rule_catalog_t *catalog, const player_rule_t *rule) {
  if (!catalog || !rule)
    return false;

  // Expand catalog if needed
  size_t new_size = catalog->catalog_size + 1;
  player_rule_t *new_catalog =
    realloc(catalog->rule_catalog, new_size * sizeof(player_rule_t));
  if (!new_catalog)
    return false;

  catalog->rule_catalog = new_catalog;
  catalog->rule_catalog[catalog->catalog_size] = *rule;
  catalog->catalog_size = new_size;

  return true;
}

bool rule_catalog_can_acquire(const rule_catalog_t *catalog, uint32_t player_id,
                              uint32_t rule_id) {
  if (!catalog || player_id >= catalog->player_count)
    return false;

  // Find rule in catalog
  const player_rule_t *rule = NULL;
  for (size_t i = 0; i < catalog->catalog_size; i++) {
    if (catalog->rule_catalog[i].rule_template.id == rule_id) {
      rule = &catalog->rule_catalog[i];
      break;
    }
  }

  if (!rule)
    return false;

  // Check status
  if (rule->status != RULE_STATUS_AVAILABLE)
    return false;

  // Check cost
  if (catalog->player_rule_points[player_id] < rule->cost_to_acquire)
    return false;

  // Check instance limit
  if (rule->max_instances > 0 &&
      rule->current_instances >= rule->max_instances) {
    return false;
  }

  // Check prerequisites
  for (size_t i = 0; i < rule->prerequisite_count; i++) {
    uint32_t prereq_id = rule->prerequisites[i];
    bool has_prereq = false;

    for (size_t j = 0; j < catalog->player_active_count[player_id]; j++) {
      if (catalog->player_active[player_id][j].rule_template.id == prereq_id) {
        has_prereq = true;
        break;
      }
    }

    if (!has_prereq)
      return false;
  }

  // Check conflicts
  for (size_t i = 0; i < rule->conflict_count; i++) {
    uint32_t conflict_id = rule->conflicts_with[i];

    for (size_t j = 0; j < catalog->player_active_count[player_id]; j++) {
      if (catalog->player_active[player_id][j].rule_template.id ==
          conflict_id) {
        return false; // Has conflicting rule
      }
    }
  }

  return true;
}

bool rule_catalog_acquire_rule(rule_catalog_t *catalog, uint32_t player_id,
                               uint32_t rule_id) {
  if (!rule_catalog_can_acquire(catalog, player_id, rule_id))
    return false;

  // Find rule in catalog
  player_rule_t *rule = NULL;
  for (size_t i = 0; i < catalog->catalog_size; i++) {
    if (catalog->rule_catalog[i].rule_template.id == rule_id) {
      rule = &catalog->rule_catalog[i];
      break;
    }
  }

  if (!rule)
    return false;

  // Deduct cost
  catalog->player_rule_points[player_id] -= rule->cost_to_acquire;

  // Add to player's active rules
  // TODO: Implement dynamic array expansion for player rules
  // For now, assume fixed maximum

  rule->current_instances++;

  return true;
}

// --- Rule Condition Checking ---

static int count_neighbors_of_type(const tile_t *tile,
                                   tile_type_t neighbor_type, int range,
                                   const rule_context_t *context) {
  if (!tile || !context)
    return 0;

  // TODO: Implement range-based neighbor counting
  // For now, just check immediate neighbors
  grid_cell_t *neighbors;
  size_t neighbor_count;
  hex_get_neighbors(tile->cell, &neighbors, &neighbor_count);

  int count = 0;
  for (size_t i = 0; i < neighbor_count; i++) {
    const tile_t *neighbor =
      tile_map_get(context->board->tile_map, neighbors[i]);
    if (neighbor) {
      tile_type_t perceived_type = neighbor->data.type;

      // Check for perception overrides
      if (context->perceived_types) {
        // TODO: Implement perception lookup by cell
      }

      if (perceived_type == neighbor_type) {
        count++;
      }
    }
  }

  free(neighbors);
  return count;
}

bool rule_check_condition(const rule_condition_t *condition,
                          const rule_context_t *context) {
  if (!condition || !context)
    return false;

  switch (condition->type) {
  case RULE_CONDITION_ALWAYS:
    return true;

  case RULE_CONDITION_SELF_TYPE: {
    if (!context->current_tile)
      return false;
    tile_type_t perceived_type = context->current_tile->data.type;

    // Check perception overrides
    if (context->perceived_types) {
      // TODO: Look up perceived type
    }

    return perceived_type == condition->params.self_type.tile_type;
  }

  case RULE_CONDITION_NEIGHBOR_COUNT: {
    if (!context->current_tile)
      return false;

    int count = count_neighbors_of_type(
      context->current_tile, condition->params.neighbor_count.neighbor_type,
      condition->params.neighbor_count.range, context);

    return count >= condition->params.neighbor_count.min_count &&
           count <= condition->params.neighbor_count.max_count;
  }

  case RULE_CONDITION_NEIGHBOR_TYPE: {
    if (!context->current_tile)
      return false;

    int count = count_neighbors_of_type(
      context->current_tile, condition->params.neighbor_type.required_type,
      condition->params.neighbor_type.range, context);

    return count > 0;
  }

  case RULE_CONDITION_POOL_SIZE: {
    if (!context->current_pool)
      return false;

    // TODO: Get actual pool size
    int pool_size = 1; // Placeholder

    return pool_size >= condition->params.pool_size.min_size &&
           pool_size <= condition->params.pool_size.max_size;
  }

  case RULE_CONDITION_TOTAL_TILE_COUNT: {
    // TODO: Count all tiles of type on board
    return true; // Placeholder
  }

  case RULE_CONDITION_VALUE_THRESHOLD: {
    if (!context->current_tile)
      return false;

    float value = tile_get_effective_production(context->current_tile);

    if (condition->params.value_threshold.greater_than) {
      return value > condition->params.value_threshold.threshold;
    } else {
      return value < condition->params.value_threshold.threshold;
    }
  }

  case RULE_CONDITION_RULE_ACTIVE: {
    // TODO: Check if specific rule is active
    return true; // Placeholder
  }

  case RULE_CONDITION_AND: {
    for (size_t i = 0; i < condition->params.logical.condition_count; i++) {
      if (!rule_check_condition(&condition->params.logical.conditions[i],
                                context)) {
        return false;
      }
    }
    return true;
  }

  case RULE_CONDITION_OR: {
    for (size_t i = 0; i < condition->params.logical.condition_count; i++) {
      if (rule_check_condition(&condition->params.logical.conditions[i],
                               context)) {
        return true;
      }
    }
    return false;
  }

  case RULE_CONDITION_NOT: {
    if (condition->params.logical.condition_count > 0) {
      return !rule_check_condition(&condition->params.logical.conditions[0],
                                   context);
    }
    return false;
  }

  default:
    return false;
  }
}

// --- Rule Effect Application ---

void rule_apply_effect(const rule_effect_t *effect, rule_context_t *context) {
  if (!effect || !context)
    return;

  switch (effect->type) {
  case RULE_EFFECT_ADD_FLAT: {
    // TODO: Apply flat bonus based on target
    break;
  }

  case RULE_EFFECT_ADD_PERCENT: {
    // TODO: Apply percentage bonus
    break;
  }

  case RULE_EFFECT_MULTIPLY: {
    // TODO: Apply multiplication
    break;
  }

  case RULE_EFFECT_SET_VALUE: {
    // TODO: Set value
    break;
  }

  case RULE_EFFECT_SCALE_BY_COUNT: {
    // TODO: Scale effect by count of something
    break;
  }

  case RULE_EFFECT_OVERRIDE_TYPE: {
    if (effect->target == RULE_TARGET_PERCEPTION && context->current_tile) {
      // TODO: Update perception arrays
    }
    break;
  }

  case RULE_EFFECT_MODIFY_RANGE: {
    if (effect->target == RULE_TARGET_RANGE && context->current_tile) {
      // TODO: Modify tile range in perception state
    }
    break;
  }

  case RULE_EFFECT_CONDITIONAL: {
    // TODO: Evaluate condition and apply appropriate effect
    break;
  }

  default:
    break;
  }
}

// --- Rule Evaluation ---

bool rule_context_init(rule_context_t *context, const board_t *board,
                       size_t max_rules) {
  if (!context || !board)
    return false;

  memset(context, 0, sizeof(*context));
  context->board = board;

  // Allocate perception arrays
  size_t max_cells = 1000; // TODO: Get actual board size
  context->perceived_types = calloc(max_cells, sizeof(tile_type_t));
  context->perceived_values = calloc(max_cells, sizeof(float));
  context->perceived_ranges = calloc(max_cells, sizeof(int));
  context->perception_capacity = max_cells;

  // Allocate rule processing tracking
  context->rules_processed = calloc(max_rules, sizeof(bool));
  context->rules_processed_capacity = max_rules;

  if (!context->perceived_types || !context->perceived_values ||
      !context->perceived_ranges || !context->rules_processed) {
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
  free(context->perceived_ranges);
  free(context->rules_processed);

  memset(context, 0, sizeof(*context));
}

void rule_evaluate_all(const rule_registry_t *registry,
                       rule_context_t *context) {
  if (!registry || !context)
    return;

  // Clear processing flags
  memset(context->rules_processed, 0,
         context->rules_processed_capacity * sizeof(bool));

  // Sort all rules by priority
  rule_t **sorted_rules = malloc(registry->rule_count * sizeof(rule_t *));
  if (!sorted_rules)
    return;

  for (size_t i = 0; i < registry->rule_count; i++) {
    sorted_rules[i] = registry->all_rules[i];
  }

  qsort(sorted_rules, registry->rule_count, sizeof(rule_t *),
        rule_priority_compare);

  // Evaluate rules in priority order
  for (size_t i = 0; i < registry->rule_count; i++) {
    rule_t *rule = sorted_rules[i];

    if (!rule->is_active || (rule->id < context->rules_processed_capacity &&
                             context->rules_processed[rule->id])) {
      continue;
    }

    if (rule->lifecycle == RULE_LIFECYCLE_PERSISTENT) {
      if (rule_check_condition(&rule->condition, context)) {
        for (size_t j = 0; j < rule->effect_count; j++) {
          rule_apply_effect(&rule->effects[j], context);
        }
      }
    }

    // Mark as processed
    if (rule->id < context->rules_processed_capacity) {
      context->rules_processed[rule->id] = true;
    }
  }

  free(sorted_rules);
}

void rule_execute_instant_rules(rule_registry_t *registry,
                                rule_context_t *context) {
  if (!registry || !context)
    return;

  // Find and execute instant rules
  for (size_t i = 0; i < registry->rule_count; i++) {
    rule_t *rule = registry->all_rules[i];

    if (rule->lifecycle == RULE_LIFECYCLE_INSTANT && rule->is_active &&
        !rule->executed) {

      if (rule_check_condition(&rule->condition, context)) {
        for (size_t j = 0; j < rule->effect_count; j++) {
          rule_apply_effect(&rule->effects[j], context);
        }
        rule->executed = true;
      }
    }
  }

  // Remove executed instant rules
  for (size_t i = 0; i < registry->rule_count;) {
    if (registry->all_rules[i]->lifecycle == RULE_LIFECYCLE_INSTANT &&
        registry->all_rules[i]->executed) {
      rule_registry_remove(registry, registry->all_rules[i]->id);
    } else {
      i++;
    }
  }
}

// --- Utility Functions ---

uint32_t rule_hash_cell(grid_cell_t cell) {
  // Simple hash function for hex coordinates
  uint32_t hash = 0;
  hash ^= (uint32_t)cell.coord.hex.q + 0x9e3779b9 + (hash << 6) + (hash >> 2);
  hash ^= (uint32_t)cell.coord.hex.r + 0x9e3779b9 + (hash << 6) + (hash >> 2);
  return hash;
}

void rule_print(const rule_t *rule) {
  if (!rule)
    return;

  printf("Rule %u:\n", rule->id);
  printf("  Priority: %d\n", rule->priority);
  printf("  Lifecycle: %s\n",
         rule->lifecycle == RULE_LIFECYCLE_INSTANT ? "INSTANT" : "PERSISTENT");
  printf("  Scope: %d\n", rule->scope);
  printf("  Active: %s\n", rule->is_active ? "YES" : "NO");
  printf("  Effects: %zu\n", rule->effect_count);
}

void player_rule_print(const player_rule_t *rule) {
  if (!rule)
    return;

  printf("Player Rule: %s\n", rule->name);
  printf("  Description: %s\n", rule->description);
  printf("  Cost: %d\n", rule->cost_to_acquire);
  printf("  Status: %d\n", rule->status);
  printf("  Instances: %d/%d\n", rule->current_instances, rule->max_instances);
}

void rule_registry_print_stats(const rule_registry_t *registry) {
  if (!registry)
    return;

  printf("Rule Registry Stats:\n");
  printf("  Total rules: %zu\n", registry->rule_count);
  printf("  Capacity: %zu\n", registry->rule_capacity);
  printf("  Next ID: %u\n", registry->next_rule_id);

  int active_count = 0;
  for (size_t i = 0; i < registry->rule_count; i++) {
    if (registry->all_rules[i]->is_active)
      active_count++;
  }
  printf("  Active rules: %d\n", active_count);
}

void rule_catalog_print_stats(const rule_catalog_t *catalog) {
  if (!catalog)
    return;

  printf("Rule Catalog Stats:\n");
  printf("  Catalog size: %zu\n", catalog->catalog_size);
  printf("  Players: %zu\n", catalog->player_count);

  for (size_t i = 0; i < catalog->player_count; i++) {
    printf("  Player %zu: %zu available, %zu active, %d points\n", i,
           catalog->player_available_count[i], catalog->player_active_count[i],
           catalog->player_rule_points[i]);
  }
}

// --- Rule Factory Functions ---

player_rule_t rule_factory_neighbor_bonus(const char *name,
                                          tile_type_t neighbor_type,
                                          float bonus_per_neighbor, int range) {
  player_rule_t rule = {0};

  strncpy(rule.name, name, MAX_RULE_NAME_LENGTH - 1);

  rule.rule_template.lifecycle = RULE_LIFECYCLE_PERSISTENT;
  rule.rule_template.priority = RULE_PRIORITY_NORMAL;
  rule.rule_template.scope = RULE_SCOPE_SELF;

  rule.rule_template.condition.type = RULE_CONDITION_ALWAYS;

  // TODO: Set up effects for neighbor bonus
  rule.rule_template.effects = malloc(sizeof(rule_effect_t));
  rule.rule_template.effect_count = 1;

  rule.status = RULE_STATUS_AVAILABLE;
  rule.cost_to_acquire = 50;
  rule.cost_to_remove = 10;
  rule.max_instances = 1;

  return rule;
}

bool rule_validate(const player_rule_t *rule) {
  if (!rule)
    return false;

  // Check name
  if (strlen(rule->name) == 0)
    return false;

  // Check costs
  if (rule->cost_to_acquire < 0)
    return false;

  // Check effects exist
  if (rule->rule_template.effect_count == 0 || !rule->rule_template.effects) {
    return false;
  }

  return true;
}

bool rule_check_conflict(const player_rule_t *rule1,
                         const player_rule_t *rule2) {
  if (!rule1 || !rule2)
    return false;

  // Check if rule1 conflicts with rule2
  for (size_t i = 0; i < rule1->conflict_count; i++) {
    if (rule1->conflicts_with[i] == rule2->rule_template.id) {
      return true;
    }
  }

  // Check if rule2 conflicts with rule1
  for (size_t i = 0; i < rule2->conflict_count; i++) {
    if (rule2->conflicts_with[i] == rule1->rule_template.id) {
      return true;
    }
  }

  return false;
}
