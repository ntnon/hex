/**************************************************************************/ /**
                                                                              * @file rule_system.c
                                                                              * @brief High-performance rule system optimized for single-player games with large boards
                                                                              *****************************************************************************/

#include "game/rule_system.h"
#include "game/board.h"
#include "grid/grid_geometry.h"
#include "tile/tile_map.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- Internal Helper Functions ---

static tile_t *tile_map_get_tile(const tile_map_t *map, grid_cell_t cell) {
  if (!map)
    return NULL;
  tile_map_entry_t *entry = tile_map_find(map, cell);
  return entry ? entry->tile : NULL;
}

static inline uint32_t cell_to_tile_index(grid_cell_t cell,
                                          uint32_t board_width) {
  // Convert hex coordinates to linear tile index for O(1) lookup
  return (uint32_t)(cell.coord.hex.r * board_width + cell.coord.hex.q);
}

static inline bool is_valid_range(uint8_t range) {
  return range <= MAX_RULE_RANGE;
}

static int rule_priority_compare(const void *a, const void *b) {
  const rule_t *rule_a = *(const rule_t **)a;
  const rule_t *rule_b = *(const rule_t **)b;
  return (int)rule_a->priority - (int)rule_b->priority;
}

static inline uint32_t rule_cache_hash(uint32_t rule_id, uint32_t tile_index) {
  return (rule_id * 31 + tile_index) % RULE_CACHE_SIZE;
}

// --- Spatial Cache Management ---

static void spatial_cache_init(spatial_cache_t *cache, grid_cell_t center) {
  memset(cache, 0, sizeof(*cache));

  // Pre-allocate cells for each range
  for (uint8_t r = 0; r <= MAX_RULE_RANGE; r++) {
    uint16_t max_cells_at_range = (r == 0) ? 1 : 6 * r;
    cache->cells[r] = malloc(max_cells_at_range * sizeof(grid_cell_t));
    cache->counts[r] = 0;
    cache->dirty[r] = true;
  }
}

static void spatial_cache_cleanup(spatial_cache_t *cache) {
  for (uint8_t r = 0; r <= MAX_RULE_RANGE; r++) {
    free(cache->cells[r]);
    cache->cells[r] = NULL;
  }
}

static void spatial_cache_build_range(spatial_cache_t *cache,
                                      grid_cell_t center, uint8_t range) {
  if (!cache->dirty[range]) {
    return; // Already calculated
  }

  cache->counts[range] = 0;

  if (range == 0) {
    cache->cells[range][0] = center;
    cache->counts[range] = 1;
  } else {
    // Use existing grid geometry function for cells at exact range
    // For now, get all cells in range (we'll filter later if needed)
    grid_cell_t *temp_cells;
    size_t temp_count;
    grid_get_cells_in_range(NULL, center, range, &temp_cells, &temp_count);

    // Copy to cache
    uint16_t copy_count = (temp_count > 6 * range) ? 6 * range : temp_count;
    if (temp_cells && copy_count > 0) {
      memcpy(cache->cells[range], temp_cells, copy_count * sizeof(grid_cell_t));
      cache->counts[range] = copy_count;
    }

    free(temp_cells);
  }

  cache->dirty[range] = false;
}

static void spatial_cache_invalidate(spatial_cache_t *cache,
                                     uint8_t max_range) {
  for (uint8_t r = 0; r <= max_range && r <= MAX_RULE_RANGE; r++) {
    cache->dirty[r] = true;
  }
}

// --- Rule Cache Management ---

static rule_cache_entry_t *rule_cache_get(rule_registry_t *registry,
                                          uint32_t rule_id,
                                          uint32_t tile_index) {
  uint32_t hash = rule_cache_hash(rule_id, tile_index);
  rule_cache_entry_t *entry = &registry->rule_cache[hash];

  if (entry->valid && entry->rule_id == rule_id &&
      entry->cell.coord.hex.q == tile_index &&
      entry->cache_generation == registry->cache_generation) {
    return entry;
  }

  return NULL;
}

static void rule_cache_set(rule_registry_t *registry, uint32_t rule_id,
                           uint32_t tile_index, grid_cell_t cell,
                           float result) {
  uint32_t hash = rule_cache_hash(rule_id, tile_index);
  rule_cache_entry_t *entry = &registry->rule_cache[hash];

  entry->rule_id = rule_id;
  entry->cell = cell;
  entry->result = result;
  entry->cache_generation = registry->cache_generation;
  entry->valid = true;
}

// --- Core Rule System Implementation ---

bool rule_registry_init(rule_registry_t *registry, uint32_t max_tiles) {
  if (!registry || max_tiles == 0) {
    return false;
  }

  memset(registry, 0, sizeof(*registry));

  // Initialize rule storage
  registry->rule_capacity = 256;
  registry->rules = calloc(registry->rule_capacity, sizeof(rule_t));
  if (!registry->rules) {
    return false;
  }

  // Initialize per-tile data
  registry->tile_data_capacity = max_tiles;
  registry->tile_data = calloc(max_tiles, sizeof(tile_rule_data_t));
  if (!registry->tile_data) {
    free(registry->rules);
    return false;
  }

  // Initialize spatial cache for each tile
  for (uint32_t i = 0; i < max_tiles; i++) {
    // Initialize with dummy center - will be set properly when tile is placed
    grid_cell_t dummy_center = {.type = GRID_TYPE_HEXAGON, .coord.hex = {0, 0}};
    spatial_cache_init(&registry->tile_data[i].spatial_cache, dummy_center);
  }

  // Initialize scope-based rule arrays
  registry->self_rules = calloc(registry->rule_capacity, sizeof(uint32_t));
  registry->neighbor_rules = calloc(registry->rule_capacity, sizeof(uint32_t));
  registry->range_rules = calloc(registry->rule_capacity, sizeof(uint32_t));
  registry->pool_rules = calloc(registry->rule_capacity, sizeof(uint32_t));
  registry->global_rules = calloc(registry->rule_capacity, sizeof(uint32_t));

  if (!registry->self_rules || !registry->neighbor_rules ||
      !registry->range_rules || !registry->pool_rules ||
      !registry->global_rules) {
    rule_registry_cleanup(registry);
    return false;
  }

  // Initialize dirty tile tracking
  registry->dirty_tiles = calloc(max_tiles, sizeof(uint32_t));
  if (!registry->dirty_tiles) {
    rule_registry_cleanup(registry);
    return false;
  }

  registry->next_rule_id = 1;
  registry->cache_generation = 1;
  registry->batch_mode = false;

  return true;
}

void rule_registry_cleanup(rule_registry_t *registry) {
  if (!registry) {
    return;
  }

  // Cleanup spatial caches
  if (registry->tile_data) {
    for (uint32_t i = 0; i < registry->tile_data_capacity; i++) {
      spatial_cache_cleanup(&registry->tile_data[i].spatial_cache);
    }
    free(registry->tile_data);
  }

  free(registry->rules);
  free(registry->self_rules);
  free(registry->neighbor_rules);
  free(registry->range_rules);
  free(registry->pool_rules);
  free(registry->global_rules);
  free(registry->dirty_tiles);

  memset(registry, 0, sizeof(*registry));
}

static bool rule_registry_ensure_capacity(rule_registry_t *registry) {
  if (registry->rule_count >= registry->rule_capacity) {
    uint32_t new_capacity = registry->rule_capacity * 2;

    rule_t *new_rules = realloc(registry->rules, new_capacity * sizeof(rule_t));
    if (!new_rules) {
      return false;
    }
    registry->rules = new_rules;

    // Resize scope arrays
    registry->self_rules =
      realloc(registry->self_rules, new_capacity * sizeof(uint32_t));
    registry->neighbor_rules =
      realloc(registry->neighbor_rules, new_capacity * sizeof(uint32_t));
    registry->range_rules =
      realloc(registry->range_rules, new_capacity * sizeof(uint32_t));
    registry->pool_rules =
      realloc(registry->pool_rules, new_capacity * sizeof(uint32_t));
    registry->global_rules =
      realloc(registry->global_rules, new_capacity * sizeof(uint32_t));

    if (!registry->self_rules || !registry->neighbor_rules ||
        !registry->range_rules || !registry->pool_rules ||
        !registry->global_rules) {
      return false;
    }

    registry->rule_capacity = new_capacity;
  }

  return true;
}

static void rule_registry_add_to_scope_list(rule_registry_t *registry,
                                            uint32_t rule_index) {
  rule_t *rule = &registry->rules[rule_index];

  switch (rule->scope) {
  case RULE_SCOPE_SELF:
    registry->self_rules[registry->self_count++] = rule_index;
    break;
  case RULE_SCOPE_NEIGHBORS:
    registry->neighbor_rules[registry->neighbor_count++] = rule_index;
    break;
  case RULE_SCOPE_RANGE:
    registry->range_rules[registry->range_count++] = rule_index;
    break;
  case RULE_SCOPE_POOL:
    registry->pool_rules[registry->pool_count++] = rule_index;
    break;
  case RULE_SCOPE_TYPE_GLOBAL:
  case RULE_SCOPE_BOARD_GLOBAL:
    registry->global_rules[registry->global_count++] = rule_index;
    break;
  }
}

uint32_t rule_registry_add_rule(rule_registry_t *registry, const rule_t *rule) {
  if (!registry || !rule) {
    return 0;
  }

  if (!rule_registry_ensure_capacity(registry)) {
    return 0;
  }

  // Copy rule and assign ID
  uint32_t rule_index = registry->rule_count;
  registry->rules[rule_index] = *rule;
  registry->rules[rule_index].id = registry->next_rule_id++;
  registry->rules[rule_index].is_active = true;
  registry->rules[rule_index].needs_recalc = true;

  // Determine cache-friendliness
  registry->rules[rule_index].cache_friendly =
    (rule->condition_type == RULE_CONDITION_ALWAYS ||
     rule->condition_type == RULE_CONDITION_SELF_TYPE) &&
    (rule->effect_type == RULE_EFFECT_ADD_FLAT ||
     rule->effect_type == RULE_EFFECT_MULTIPLY);

  registry->rule_count++;

  // Add to appropriate scope list
  rule_registry_add_to_scope_list(registry, rule_index);

  // Mark affected area as dirty
  if (rule->scope == RULE_SCOPE_RANGE || rule->scope == RULE_SCOPE_NEIGHBORS) {
    rule_registry_mark_area_dirty(registry, rule->source_cell,
                                  rule->affected_range);
  } else if (rule->scope == RULE_SCOPE_TYPE_GLOBAL ||
             rule->scope == RULE_SCOPE_BOARD_GLOBAL) {
    // Mark entire board dirty
    registry->cache_generation++;
  }

  return registry->rules[rule_index].id;
}

bool rule_registry_remove_rule(rule_registry_t *registry, uint32_t rule_id) {
  if (!registry || rule_id == 0) {
    return false;
  }

  // Find rule
  for (uint32_t i = 0; i < registry->rule_count; i++) {
    if (registry->rules[i].id == rule_id) {
      rule_t *rule = &registry->rules[i];

      // Mark affected area as dirty
      if (rule->scope == RULE_SCOPE_RANGE ||
          rule->scope == RULE_SCOPE_NEIGHBORS) {
        rule_registry_mark_area_dirty(registry, rule->source_cell,
                                      rule->affected_range);
      } else if (rule->scope == RULE_SCOPE_TYPE_GLOBAL ||
                 rule->scope == RULE_SCOPE_BOARD_GLOBAL) {
        registry->cache_generation++;
      }

      // Remove from scope lists (expensive but rare operation)
      // TODO: Optimize this if rule removal becomes frequent

      // Shift remaining rules
      for (uint32_t j = i; j < registry->rule_count - 1; j++) {
        registry->rules[j] = registry->rules[j + 1];
      }
      registry->rule_count--;

      // Rebuild scope lists (expensive but correctness is more important)
      registry->self_count = registry->neighbor_count = registry->range_count =
        registry->pool_count = registry->global_count = 0;

      for (uint32_t j = 0; j < registry->rule_count; j++) {
        rule_registry_add_to_scope_list(registry, j);
      }

      return true;
    }
  }

  return false;
}

void rule_registry_remove_by_source(rule_registry_t *registry,
                                    grid_cell_t source_cell) {
  if (!registry) {
    return;
  }

  // Mark rules for removal (backwards iteration to avoid index issues)
  for (int32_t i = (int32_t)registry->rule_count - 1; i >= 0; i--) {
    rule_t *rule = &registry->rules[i];
    if (rule->source_cell.coord.hex.q == source_cell.coord.hex.q &&
        rule->source_cell.coord.hex.r == source_cell.coord.hex.r) {
      rule_registry_remove_rule(registry, rule->id);
    }
  }
}

// --- Rule Evaluation Context ---

bool rule_context_init(rule_context_t *context, const board_t *board,
                       const rule_registry_t *registry,
                       uint32_t temp_buffer_size) {
  if (!context || !board || !registry) {
    return false;
  }

  memset(context, 0, sizeof(*context));
  context->board = board;
  context->registry = registry;
  context->temp_capacity = temp_buffer_size;

  // Allocate temporary buffers for batch operations
  context->temp_cells = calloc(temp_buffer_size, sizeof(grid_cell_t));
  context->temp_tiles = calloc(temp_buffer_size, sizeof(tile_t *));
  context->temp_values = calloc(temp_buffer_size, sizeof(float));

  if (!context->temp_cells || !context->temp_tiles || !context->temp_values) {
    rule_context_cleanup(context);
    return false;
  }

  return true;
}

void rule_context_cleanup(rule_context_t *context) {
  if (!context) {
    return;
  }

  free(context->temp_cells);
  free(context->temp_tiles);
  free(context->temp_values);

  memset(context, 0, sizeof(*context));
}

// --- Rule Condition Checking ---

static bool rule_check_condition(const rule_t *rule, rule_context_t *context) {
  if (!rule || !context) {
    return false;
  }

  switch (rule->condition_type) {
  case RULE_CONDITION_ALWAYS:
    return true;

  case RULE_CONDITION_SELF_TYPE:
    if (!context->current_tile)
      return false;
    return context->current_tile->data.type == rule->condition_params.tile_type;

  case RULE_CONDITION_NEIGHBOR_COUNT: {
    if (!context->current_tile)
      return false;

    uint32_t count = rule_count_tiles_in_range(
      (rule_registry_t *)context->registry, context, context->current_cell,
      rule->condition_params.neighbor_count.neighbor_type,
      rule->condition_params.neighbor_count.range);

    return count >= rule->condition_params.neighbor_count.min_count &&
           count <= rule->condition_params.neighbor_count.max_count;
  }

  case RULE_CONDITION_POOL_SIZE: {
    // TODO: Implement pool size checking
    return true;
  }

  case RULE_CONDITION_BOARD_COUNT: {
    // TODO: Implement board-wide tile counting
    return true;
  }

  case RULE_CONDITION_PRODUCTION_THRESHOLD: {
    if (!context->current_tile)
      return false;

    float production = tile_get_effective_production(context->current_tile);
    if (rule->condition_params.production_threshold.greater_than) {
      return production > rule->condition_params.production_threshold.threshold;
    } else {
      return production < rule->condition_params.production_threshold.threshold;
    }
  }

  default:
    return false;
  }
}

// --- Rule Effect Application ---

static float rule_apply_effect(const rule_t *rule, rule_context_t *context,
                               float base_value) {
  if (!rule || !context) {
    return base_value;
  }

  switch (rule->effect_type) {
  case RULE_EFFECT_ADD_FLAT:
    return base_value + rule->effect_params.value;

  case RULE_EFFECT_ADD_SCALED: {
    uint32_t count = 0;

    // Get scale count based on scale_source
    switch (rule->effect_params.scaled.scale_source) {
    case RULE_CONDITION_NEIGHBOR_COUNT:
      count = rule_count_tiles_in_range(
        (rule_registry_t *)context->registry, context, context->current_cell,
        rule->effect_params.scaled.scale_params.neighbor_count.neighbor_type,
        rule->effect_params.scaled.scale_params.neighbor_count.range);
      break;
    default:
      count = 1;
    }

    return base_value + rule->effect_params.scaled.base_value +
           (count * rule->effect_params.scaled.scale_factor);
  }

  case RULE_EFFECT_MULTIPLY:
    return base_value * rule->effect_params.value;

  case RULE_EFFECT_SET_VALUE:
    return rule->effect_params.value;

  default:
    return base_value;
  }
}

// --- High-Performance Rule Calculation ---

float rule_calculate_tile_production(rule_registry_t *registry,
                                     rule_context_t *context,
                                     const tile_t *tile) {
  if (!registry || !context || !tile) {
    return 0.0f;
  }

  uint32_t tile_index =
    cell_to_tile_index(tile->cell, 100); // TODO: Get real board width

  if (tile_index >= registry->tile_data_capacity) {
    return tile_get_effective_production(tile); // Fallback
  }

  tile_rule_data_t *tile_data = &registry->tile_data[tile_index];

  // Check cache
  if (!tile_data->production_dirty &&
      tile_data->cache_generation == registry->cache_generation) {
    return tile_data->cached_production;
  }

  // Set context for this tile
  context->current_tile = tile;
  context->current_cell = tile->cell;
  context->current_tile_index = tile_index;

  float production = tile_get_effective_production(tile); // Base production

  // Apply rules in priority order
  // 1. Self-scope rules first (cheapest)
  for (uint16_t i = 0; i < registry->self_count; i++) {
    rule_t *rule = &registry->rules[registry->self_rules[i]];
    if (rule->is_active && rule->target == RULE_TARGET_PRODUCTION &&
        rule_check_condition(rule, context)) {
      production = rule_apply_effect(rule, context, production);
    }
  }

  // 2. Range-scope rules (more expensive)
  for (uint16_t i = 0; i < registry->range_count; i++) {
    rule_t *rule = &registry->rules[registry->range_rules[i]];
    if (rule->is_active && rule->target == RULE_TARGET_PRODUCTION) {
      // Check if this tile is in range of the rule source
      if (grid_distance(tile->cell, rule->source_cell) <=
            rule->affected_range &&
          rule_check_condition(rule, context)) {
        production = rule_apply_effect(rule, context, production);
      }
    }
  }

  // 3. Global rules (most expensive)
  for (uint16_t i = 0; i < registry->global_count; i++) {
    rule_t *rule = &registry->rules[registry->global_rules[i]];
    if (rule->is_active && rule->target == RULE_TARGET_PRODUCTION &&
        rule_check_condition(rule, context)) {
      production = rule_apply_effect(rule, context, production);
    }
  }

  // Cache result
  tile_data->cached_production = production;
  tile_data->production_dirty = false;
  tile_data->cache_generation = registry->cache_generation;

  registry->evaluations_total++;

  return production;
}

uint8_t rule_calculate_tile_range(rule_registry_t *registry,
                                  rule_context_t *context, const tile_t *tile) {
  if (!registry || !context || !tile) {
    return tile_get_range(tile);
  }

  uint32_t tile_index =
    cell_to_tile_index(tile->cell, 100); // TODO: Get real board width

  if (tile_index >= registry->tile_data_capacity) {
    return tile_get_range(tile); // Fallback
  }

  tile_rule_data_t *tile_data = &registry->tile_data[tile_index];

  // Check cache
  if (!tile_data->range_dirty &&
      tile_data->cache_generation == registry->cache_generation) {
    return tile_data->cached_range;
  }

  context->current_tile = tile;
  context->current_cell = tile->cell;
  context->current_tile_index = tile_index;

  uint8_t range = tile_get_range(tile); // Base range

  // Apply range modification rules (highest priority)
  for (uint16_t i = 0; i < registry->rule_count; i++) {
    rule_t *rule = &registry->rules[i];
    if (rule->is_active && rule->target == RULE_TARGET_RANGE &&
        rule->priority == RULE_PRIORITY_RANGE_MODIFY &&
        rule_check_condition(rule, context)) {

      if (rule->effect_type == RULE_EFFECT_ADD_FLAT) {
        int new_range = (int)range + (int)rule->effect_params.range_delta;
        range = (new_range < 0)                ? 0
                : (new_range > MAX_RULE_RANGE) ? MAX_RULE_RANGE
                                               : (uint8_t)new_range;
      }
    }
  }

  // Cache result
  tile_data->cached_range = range;
  tile_data->range_dirty = false;
  tile_data->cache_generation = registry->cache_generation;

  return range;
}

tile_type_t rule_calculate_perceived_type(rule_registry_t *registry,
                                          rule_context_t *context,
                                          const tile_t *tile,
                                          grid_cell_t observer_cell) {
  if (!registry || !context || !tile) {
    return tile->data.type;
  }

  uint32_t tile_index =
    cell_to_tile_index(tile->cell, 100); // TODO: Get real board width

  if (tile_index >= registry->tile_data_capacity) {
    return tile->data.type; // Fallback
  }

  tile_rule_data_t *tile_data = &registry->tile_data[tile_index];

  // Check cache (note: this doesn't account for observer position yet - TODO)
  if (!tile_data->type_dirty &&
      tile_data->cache_generation == registry->cache_generation) {
    return tile_data->cached_type;
  }

  tile_type_t perceived_type = tile->data.type;

  // Check for perception override rules
  for (uint16_t i = 0; i < registry->range_count; i++) {
    rule_t *rule = &registry->rules[registry->range_rules[i]];
    if (rule->is_active && rule->target == RULE_TARGET_TYPE_OVERRIDE) {
      // Check if observer is within range of the rule source
      if (grid_distance(observer_cell, rule->source_cell) <=
          rule->affected_range) {
        perceived_type = rule->effect_params.override_type;
        break; // First override wins
      }
    }
  }

  // Cache result
  tile_data->cached_type = perceived_type;
  tile_data->type_dirty = false;
  tile_data->cache_generation = registry->cache_generation;

  return perceived_type;
}

// --- Incremental Updates ---

void rule_registry_mark_tile_dirty(rule_registry_t *registry,
                                   uint32_t tile_index) {
  if (!registry || tile_index >= registry->tile_data_capacity) {
    return;
  }

  tile_rule_data_t *tile_data = &registry->tile_data[tile_index];
  tile_data->production_dirty = true;
  tile_data->range_dirty = true;
  tile_data->type_dirty = true;

  if (!registry->batch_mode) {
    // Add to dirty list for immediate processing
    registry->dirty_tiles[registry->dirty_tile_count++] = tile_index;
  }
}

void rule_registry_mark_area_dirty(rule_registry_t *registry, grid_cell_t cell,
                                   uint8_t radius) {
  if (!registry || radius > MAX_RULE_RANGE) {
    return;
  }

  // Get all cells within radius and mark them dirty
  grid_cell_t *cells_in_area;
  size_t cell_count;

  grid_get_cells_in_range(NULL, cell, radius, &cells_in_area, &cell_count);

  for (size_t i = 0; i < cell_count; i++) {
    uint32_t tile_index =
      cell_to_tile_index(cells_in_area[i], 100); // TODO: Get real board width
    rule_registry_mark_tile_dirty(registry, tile_index);
  }

  free(cells_in_area);
}

void rule_registry_process_dirty_tiles(rule_registry_t *registry,
                                       rule_context_t *context) {
  if (!registry || !context) {
    return;
  }

  // Process all dirty tiles in batch
  for (uint32_t i = 0; i < registry->dirty_tile_count; i++) {
    uint32_t tile_index = registry->dirty_tiles[i];

    if (tile_index < registry->tile_data_capacity) {
      tile_rule_data_t *tile_data = &registry->tile_data[tile_index];

      // Invalidate spatial caches
      spatial_cache_invalidate(&tile_data->spatial_cache, MAX_RULE_RANGE);
    }
  }

  registry->dirty_tile_count = 0;
}

void rule_registry_set_batch_mode(rule_registry_t *registry, bool enabled) {
  if (!registry) {
    return;
  }

  if (!enabled && registry->batch_mode) {
    // Exiting batch mode - process any pending dirty tiles
    rule_context_t temp_context = {0};
    rule_registry_process_dirty_tiles(registry, &temp_context);
  }

  registry->batch_mode = enabled;
}

// --- Spatial Query Optimization ---

uint32_t rule_get_tiles_in_range(rule_registry_t *registry,
                                 rule_context_t *context,
                                 grid_cell_t center_cell, tile_type_t tile_type,
                                 uint8_t range, tile_t **out_tiles,
                                 uint32_t max_tiles) {
  if (!registry || !context || !out_tiles || max_tiles == 0 ||
      range > MAX_RULE_RANGE) {
    return 0;
  }

  uint32_t found_count = 0;

  // Get cells in range
  grid_cell_t *cells;
  size_t cell_count;
  grid_get_cells_in_range(NULL, center_cell, range, &cells, &cell_count);

  // Check each cell for matching tiles
  for (size_t i = 0; i < cell_count && found_count < max_tiles; i++) {
    tile_t *tile = tile_map_get_tile(context->board->tiles, cells[i]);
    if (tile && tile->data.type == tile_type) {
      out_tiles[found_count++] = tile;
    }
  }

  free(cells);
  return found_count;
}

uint32_t rule_count_tiles_in_range(rule_registry_t *registry,
                                   rule_context_t *context,
                                   grid_cell_t center_cell,
                                   tile_type_t tile_type, uint8_t range) {
  if (!registry || !context || range > MAX_RULE_RANGE) {
    return 0;
  }

  // Try to use cached spatial data if available
  uint32_t center_index =
    cell_to_tile_index(center_cell, 100); // TODO: Get real board width

  if (center_index < registry->tile_data_capacity) {
    spatial_cache_t *cache = &registry->tile_data[center_index].spatial_cache;
    spatial_cache_build_range(cache, center_cell, range);

    uint32_t count = 0;
    for (uint16_t i = 0; i < cache->counts[range]; i++) {
      tile_t *tile =
        tile_map_get_tile(context->board->tiles, cache->cells[range][i]);
      if (tile && tile->data.type == tile_type) {
        count++;
      }
    }
    return count;
  }

  // Fallback to direct calculation
  uint32_t count = 0;
  grid_cell_t *cells;
  size_t cell_count;
  grid_get_cells_in_range(NULL, center_cell, range, &cells, &cell_count);

  for (size_t i = 0; i < cell_count; i++) {
    tile_t *tile = tile_map_get_tile(context->board->tiles, cells[i]);
    if (tile && tile->data.type == tile_type) {
      count++;
    }
  }

  free(cells);
  return count;
}

// --- Rule Factory Functions ---

rule_t rule_create_neighbor_bonus(grid_cell_t source_cell,
                                  tile_type_t neighbor_type,
                                  float bonus_per_neighbor, uint8_t range) {
  rule_t rule = {0};

  rule.priority = RULE_PRIORITY_PRODUCTION;
  rule.scope = RULE_SCOPE_SELF;
  rule.target = RULE_TARGET_PRODUCTION;

  rule.condition_type = RULE_CONDITION_ALWAYS;

  rule.effect_type = RULE_EFFECT_ADD_SCALED;
  rule.effect_params.scaled.base_value = 0.0f;
  rule.effect_params.scaled.scale_factor = bonus_per_neighbor;
  rule.effect_params.scaled.scale_source = RULE_CONDITION_NEIGHBOR_COUNT;
  rule.effect_params.scaled.scale_params.neighbor_count.neighbor_type =
    neighbor_type;
  rule.effect_params.scaled.scale_params.neighbor_count.min_count = 0;
  rule.effect_params.scaled.scale_params.neighbor_count.max_count = 255;
  rule.effect_params.scaled.scale_params.neighbor_count.range = range;

  rule.source_cell = source_cell;
  rule.affected_range = range;
  rule.cache_friendly = false; // Depends on neighbors

  return rule;
}

rule_t rule_create_range_modifier(grid_cell_t source_cell,
                                  tile_type_t target_type, int8_t range_delta) {
  rule_t rule = {0};

  rule.priority = RULE_PRIORITY_RANGE_MODIFY;
  rule.scope =
    (target_type == TILE_UNDEFINED) ? RULE_SCOPE_SELF : RULE_SCOPE_TYPE_GLOBAL;
  rule.target = RULE_TARGET_RANGE;

  rule.condition_type = (target_type == TILE_UNDEFINED)
                          ? RULE_CONDITION_ALWAYS
                          : RULE_CONDITION_SELF_TYPE;
  if (target_type != TILE_UNDEFINED) {
    rule.condition_params.tile_type = target_type;
  }

  rule.effect_type = RULE_EFFECT_ADD_FLAT;
  rule.effect_params.range_delta = range_delta;

  rule.source_cell = source_cell;
  rule.affected_range = 0; // Global effect
  rule.cache_friendly = true;

  return rule;
}

rule_t rule_create_type_override(grid_cell_t source_cell,
                                 tile_type_t override_type, uint8_t range) {
  rule_t rule = {0};

  rule.priority = RULE_PRIORITY_PERCEPTION;
  rule.scope = RULE_SCOPE_RANGE;
  rule.target = RULE_TARGET_TYPE_OVERRIDE;

  rule.condition_type = RULE_CONDITION_ALWAYS;

  rule.effect_type = RULE_EFFECT_OVERRIDE_TYPE;
  rule.effect_params.override_type = override_type;

  rule.source_cell = source_cell;
  rule.affected_range = range;
  rule.cache_friendly = true;

  return rule;
}

rule_t rule_create_pool_scaling(grid_cell_t source_cell, float base_bonus,
                                float scale_factor) {
  rule_t rule = {0};

  rule.priority = RULE_PRIORITY_PRODUCTION;
  rule.scope = RULE_SCOPE_POOL;
  rule.target = RULE_TARGET_PRODUCTION;

  rule.condition_type = RULE_CONDITION_ALWAYS;

  rule.effect_type = RULE_EFFECT_ADD_SCALED;
  rule.effect_params.scaled.base_value = base_bonus;
  rule.effect_params.scaled.scale_factor = scale_factor;
  rule.effect_params.scaled.scale_source = RULE_CONDITION_POOL_SIZE;

  rule.source_cell = source_cell;
  rule.affected_range = 0;
  rule.cache_friendly = false; // Depends on pool size

  return rule;
}

rule_t rule_create_global_modifier(grid_cell_t source_cell,
                                   tile_type_t target_type, float modifier) {
  rule_t rule = {0};

  rule.priority = RULE_PRIORITY_PRODUCTION;
  rule.scope = RULE_SCOPE_TYPE_GLOBAL;
  rule.target = RULE_TARGET_PRODUCTION;

  rule.condition_type = RULE_CONDITION_SELF_TYPE;
  rule.condition_params.tile_type = target_type;

  rule.effect_type = RULE_EFFECT_ADD_FLAT;
  rule.effect_params.value = modifier;

  rule.source_cell = source_cell;
  rule.affected_range = 0; // Global
  rule.cache_friendly = true;

  return rule;
}

// --- Cache Management ---

void rule_registry_invalidate_cache(rule_registry_t *registry) {
  if (!registry) {
    return;
  }

  registry->cache_generation++;

  // Clear all tile data caches
  for (uint32_t i = 0; i < registry->tile_data_capacity; i++) {
    tile_rule_data_t *tile_data = &registry->tile_data[i];
    tile_data->production_dirty = true;
    tile_data->range_dirty = true;
    tile_data->type_dirty = true;
    spatial_cache_invalidate(&tile_data->spatial_cache, MAX_RULE_RANGE);
  }

  // Clear rule cache
  memset(registry->rule_cache, 0, sizeof(registry->rule_cache));
}

void rule_registry_warm_cache(rule_registry_t *registry,
                              rule_context_t *context) {
  if (!registry || !context) {
    return;
  }

  // Pre-calculate common patterns by iterating through all tiles
  for (uint32_t i = 0; i < registry->tile_data_capacity; i++) {
    // Get tile for this index
    // TODO: Need reverse mapping from tile_index to actual tile
    // For now, skip warming
  }
}

void rule_registry_get_cache_stats(const rule_registry_t *registry,
                                   float *out_hit_rate,
                                   uint64_t *out_total_evaluations,
                                   uint32_t *out_cache_size) {
  if (!registry) {
    if (out_hit_rate)
      *out_hit_rate = 0.0f;
    if (out_total_evaluations)
      *out_total_evaluations = 0;
    if (out_cache_size)
      *out_cache_size = 0;
    return;
  }

  if (out_hit_rate) {
    uint64_t total_requests = registry->cache_hits + registry->cache_misses;
    *out_hit_rate = total_requests > 0
                      ? (float)registry->cache_hits / (float)total_requests
                      : 0.0f;
  }

  if (out_total_evaluations) {
    *out_total_evaluations = registry->evaluations_total;
  }

  if (out_cache_size) {
    // Count valid cache entries
    uint32_t valid_entries = 0;
    for (uint32_t i = 0; i < RULE_CACHE_SIZE; i++) {
      if (registry->rule_cache[i].valid &&
          registry->rule_cache[i].cache_generation ==
            registry->cache_generation) {
        valid_entries++;
      }
    }
    *out_cache_size = valid_entries;
  }
}

// --- Debugging and Profiling ---

void rule_registry_print_stats(const rule_registry_t *registry) {
  if (!registry) {
    return;
  }

  printf("=== Rule Registry Statistics ===\n");
  printf("Rules: %u / %u (%.1f%% full)\n", registry->rule_count,
         registry->rule_capacity,
         100.0f * registry->rule_count / registry->rule_capacity);
  printf("Next ID: %u\n", registry->next_rule_id);
  printf("Cache Generation: %u\n", registry->cache_generation);
  printf("Batch Mode: %s\n", registry->batch_mode ? "ON" : "OFF");

  printf("\nRule Scope Distribution:\n");
  printf("  Self: %u\n", registry->self_count);
  printf("  Neighbors: %u\n", registry->neighbor_count);
  printf("  Range: %u\n", registry->range_count);
  printf("  Pool: %u\n", registry->pool_count);
  printf("  Global: %u\n", registry->global_count);

  uint32_t active_count = 0;
  for (uint32_t i = 0; i < registry->rule_count; i++) {
    if (registry->rules[i].is_active) {
      active_count++;
    }
  }
  printf("Active Rules: %u / %u\n", active_count, registry->rule_count);

  printf("Dirty Tiles: %u\n", registry->dirty_tile_count);
}

void rule_registry_print_performance(const rule_registry_t *registry) {
  if (!registry) {
    return;
  }

  printf("=== Rule Performance Profile ===\n");
  printf("Total Evaluations: %llu\n",
         (unsigned long long)registry->evaluations_total);
  printf("Cache Hits: %llu\n", (unsigned long long)registry->cache_hits);
  printf("Cache Misses: %llu\n", (unsigned long long)registry->cache_misses);

  uint64_t total_requests = registry->cache_hits + registry->cache_misses;
  if (total_requests > 0) {
    printf("Cache Hit Rate: %.2f%%\n",
           100.0f * registry->cache_hits / total_requests);
  }

  uint32_t valid_cache_entries = 0;
  for (uint32_t i = 0; i < RULE_CACHE_SIZE; i++) {
    if (registry->rule_cache[i].valid &&
        registry->rule_cache[i].cache_generation ==
          registry->cache_generation) {
      valid_cache_entries++;
    }
  }
  printf("Cache Usage: %u / %u (%.1f%%)\n", valid_cache_entries,
         RULE_CACHE_SIZE, 100.0f * valid_cache_entries / RULE_CACHE_SIZE);
}

void rule_registry_print_tile_rules(const rule_registry_t *registry,
                                    uint32_t tile_index) {
  if (!registry || tile_index >= registry->tile_data_capacity) {
    return;
  }

  printf("=== Rules Affecting Tile %u ===\n", tile_index);

  tile_rule_data_t *tile_data = &registry->tile_data[tile_index];
  printf("Affecting Rules: %u\n", tile_data->rule_count);

  for (uint8_t i = 0; i < tile_data->rule_count; i++) {
    rule_t *rule = tile_data->affecting_rules[i];
    printf("  Rule %u: Priority=%u, Target=%u, Active=%s\n", rule->id,
           rule->priority, rule->target, rule->is_active ? "YES" : "NO");
  }

  printf("Cache Status:\n");
  printf("  Production: %.2f (dirty=%s)\n", tile_data->cached_production,
         tile_data->production_dirty ? "YES" : "NO");
  printf("  Range: %u (dirty=%s)\n", tile_data->cached_range,
         tile_data->range_dirty ? "YES" : "NO");
  printf("  Type: %u (dirty=%s)\n", tile_data->cached_type,
         tile_data->type_dirty ? "YES" : "NO");
  printf("  Cache Gen: %u\n", tile_data->cache_generation);
}

bool rule_registry_validate(const rule_registry_t *registry) {
  if (!registry) {
    return false;
  }

  // Check basic consistency
  if (registry->rule_count > registry->rule_capacity) {
    printf("ERROR: rule_count (%u) > rule_capacity (%u)\n",
           registry->rule_count, registry->rule_capacity);
    return false;
  }

  // Check scope counts
  uint32_t total_scope_count = registry->self_count + registry->neighbor_count +
                               registry->range_count + registry->pool_count +
                               registry->global_count;
  if (total_scope_count > registry->rule_count) {
    printf("ERROR: Total scope count (%u) > rule count (%u)\n",
           total_scope_count, registry->rule_count);
    return false;
  }

  // Check active rules
  uint32_t active_count = 0;
  for (uint32_t i = 0; i < registry->rule_count; i++) {
    if (registry->rules[i].is_active) {
      active_count++;

      // Check range validity
      if (registry->rules[i].affected_range > MAX_RULE_RANGE) {
        printf("ERROR: Rule %u has invalid range %u\n", registry->rules[i].id,
               registry->rules[i].affected_range);
        return false;
      }
    }
  }

  printf("Registry validation passed: %u active rules\n", active_count);
  return true;
}
