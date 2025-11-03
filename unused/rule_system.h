/**************************************************************************//**
 * @file rule_system.h
 * @brief High-performance rule system optimized for single-player games with large boards
 *****************************************************************************/

#ifndef RULE_SYSTEM_H
#define RULE_SYSTEM_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "grid/grid_types.h"
#include "tile/tile.h"
#include "game/board.h"

// Forward declarations
typedef struct tile_pool pool_t;

// --- Performance Constants ---
#define MAX_RULE_RANGE 9
#define MAX_RULES_PER_TILE 32
#define SPATIAL_CACHE_MAX_RANGE 9
#define RULE_BATCH_SIZE 256
#define RULE_CACHE_SIZE 4096

// --- Rule System Types ---

/**
 * @brief Rule evaluation priority for deterministic ordering
 */
typedef enum {
    RULE_PRIORITY_RANGE_MODIFY = 0,     // Range modifications (highest priority)
    RULE_PRIORITY_PERCEPTION = 10,      // Color/type overrides
    RULE_PRIORITY_PRODUCTION = 50,      // Production bonuses (most common)
    RULE_PRIORITY_EFFECTS = 100,        // Secondary effects
    RULE_PRIORITY_VISUAL = 200          // Visual-only effects (lowest priority)
} rule_priority_t;

/**
 * @brief Rule scope determines evaluation strategy
 */
typedef enum {
    RULE_SCOPE_SELF,            // Only affects source tile (cheapest)
    RULE_SCOPE_NEIGHBORS,       // Affects immediate neighbors
    RULE_SCOPE_RANGE,           // Affects tiles within range
    RULE_SCOPE_POOL,            // Affects entire pool
    RULE_SCOPE_TYPE_GLOBAL,     // Affects all tiles of specific type
    RULE_SCOPE_BOARD_GLOBAL     // Affects entire board (most expensive)
} rule_scope_t;

/**
 * @brief Rule target determines what gets modified
 */
typedef enum {
    RULE_TARGET_PRODUCTION,     // Tile production value
    RULE_TARGET_RANGE,          // Tile interaction range
    RULE_TARGET_POOL_MODIFIER,  // Pool production multiplier
    RULE_TARGET_TYPE_OVERRIDE,  // Perceived tile type
    RULE_TARGET_PLACEMENT_COST, // Tile placement cost
    RULE_TARGET_MOVEMENT_RANGE  // Tile movement range
} rule_target_t;

/**
 * @brief Optimized rule condition types
 */
typedef enum {
    RULE_CONDITION_ALWAYS,              // Always applies (fastest)
    RULE_CONDITION_SELF_TYPE,           // Check own type
    RULE_CONDITION_NEIGHBOR_COUNT,      // Count specific neighbors in range
    RULE_CONDITION_POOL_SIZE,           // Check pool size
    RULE_CONDITION_BOARD_COUNT,         // Count tiles of type on board
    RULE_CONDITION_PRODUCTION_THRESHOLD // Check production level
} rule_condition_type_t;

/**
 * @brief Optimized rule effect types
 */
typedef enum {
    RULE_EFFECT_ADD_FLAT,       // Add fixed value
    RULE_EFFECT_ADD_SCALED,     // Add value * scale_factor
    RULE_EFFECT_MULTIPLY,       // Multiply by factor
    RULE_EFFECT_SET_VALUE,      // Set to specific value
    RULE_EFFECT_OVERRIDE_TYPE,  // Override perceived type
    RULE_EFFECT_MODIFY_RANGE    // Modify range
} rule_effect_type_t;

// --- Optimized Data Structures ---

/**
 * @brief Compact rule condition parameters
 */
typedef union {
    tile_type_t tile_type;

    struct {
        tile_type_t neighbor_type;
        uint8_t min_count;
        uint8_t max_count;
        uint8_t range;
    } neighbor_count;

    struct {
        uint16_t min_size;
        uint16_t max_size;
    } pool_size;

    struct {
        tile_type_t target_type;
        uint16_t min_count;
        uint16_t max_count;
    } board_count;

    struct {
        float threshold;
        bool greater_than;
    } production_threshold;
} rule_condition_params_t;

/**
 * @brief Compact rule effect parameters
 */
typedef union {
    float value;                // For ADD_FLAT, MULTIPLY, SET_VALUE
    tile_type_t override_type;  // For OVERRIDE_TYPE
    int8_t range_delta;         // For MODIFY_RANGE

    struct {
        float base_value;
        float scale_factor;
        rule_condition_type_t scale_source;
        rule_condition_params_t scale_params;
    } scaled;
} rule_effect_params_t;

/**
 * @brief High-performance rule structure
 */
typedef struct rule {
    uint32_t id;                        // Unique identifier
    uint16_t priority;                  // Evaluation priority
    uint8_t scope;                      // rule_scope_t
    uint8_t target;                     // rule_target_t

    // Condition
    uint8_t condition_type;             // rule_condition_type_t
    rule_condition_params_t condition_params;

    // Effect
    uint8_t effect_type;                // rule_effect_type_t
    rule_effect_params_t effect_params;

    // Source and spatial data
    grid_cell_t source_cell;            // Cell that created this rule
    uint8_t affected_range;             // Maximum range this rule affects

    // Performance optimization flags
    bool is_active;                     // Can be temporarily disabled
    bool needs_recalc;                  // Marked for recalculation
    bool cache_friendly;                // Result can be cached

} rule_t;

/**
 * @brief Spatial cache for range-based lookups
 */
typedef struct {
    grid_cell_t *cells[MAX_RULE_RANGE + 1];    // cells[r] = all cells at range r
    uint16_t counts[MAX_RULE_RANGE + 1];       // counts[r] = number of cells at range r
    bool dirty[MAX_RULE_RANGE + 1];            // dirty[r] = needs recalculation
} spatial_cache_t;

/**
 * @brief Cached rule evaluation result
 */
typedef struct {
    uint32_t rule_id;
    grid_cell_t cell;
    float result;
    uint32_t cache_generation;          // For cache invalidation
    bool valid;
} rule_cache_entry_t;

/**
 * @brief Per-tile rule tracking for fast updates
 */
typedef struct {
    rule_t *affecting_rules[MAX_RULES_PER_TILE]; // Rules that affect this tile
    uint8_t rule_count;                          // Number of affecting rules

    // Cached calculations
    float cached_production;            // Last calculated production
    uint8_t cached_range;              // Last calculated range
    tile_type_t cached_type;           // Last calculated perceived type

    // Cache validity
    uint32_t cache_generation;
    bool production_dirty;
    bool range_dirty;
    bool type_dirty;

    // Spatial cache for this tile
    spatial_cache_t spatial_cache;

} tile_rule_data_t;

/**
 * @brief High-performance rule registry
 */
typedef struct {
    // Rule storage
    rule_t *rules;                      // Flat array of all rules
    uint32_t rule_count;
    uint32_t rule_capacity;
    uint32_t next_rule_id;

    // Spatial indexing for O(1) tile->rules lookup
    tile_rule_data_t *tile_data;        // tile_data[tile_index]
    uint32_t tile_data_capacity;

    // Fast lookups by scope
    uint32_t *self_rules;               // Rules with SCOPE_SELF
    uint32_t *neighbor_rules;           // Rules with SCOPE_NEIGHBORS
    uint32_t *range_rules;              // Rules with SCOPE_RANGE
    uint32_t *pool_rules;               // Rules with SCOPE_POOL
    uint32_t *global_rules;             // Rules with global scope
    uint16_t self_count, neighbor_count, range_count, pool_count, global_count;

    // Rule result cache
    rule_cache_entry_t rule_cache[RULE_CACHE_SIZE];
    uint32_t cache_generation;          // Incremented when cache invalidated

    // Batch processing for performance
    uint32_t *dirty_tiles;              // Tiles needing rule recalculation
    uint32_t dirty_tile_count;
    bool batch_mode;                    // Defer updates until batch_process()

    // Performance tracking
    uint64_t evaluations_total;
    uint64_t cache_hits;
    uint64_t cache_misses;

} rule_registry_t;

/**
 * @brief Optimized rule evaluation context
 */
typedef struct {
    const board_t *board;
    const rule_registry_t *registry;

    // Current evaluation state
    const tile_t *current_tile;
    grid_cell_t current_cell;
    uint32_t current_tile_index;

    // Batch processing arrays (reused to avoid allocations)
    grid_cell_t *temp_cells;            // For range calculations
    tile_t **temp_tiles;                // For neighbor lookups
    float *temp_values;                 // For calculations
    uint32_t temp_capacity;

    // Performance optimization
    bool skip_cache;                    // Force recalculation
    uint32_t evaluation_id;             // Unique ID for this evaluation cycle

} rule_context_t;

// --- Core API ---

/**
 * @brief Initialize high-performance rule registry
 * @param registry Registry to initialize
 * @param max_tiles Expected maximum number of tiles on board
 * @return true on success
 */
bool rule_registry_init(rule_registry_t *registry, uint32_t max_tiles);

/**
 * @brief Cleanup rule registry and free all memory
 * @param registry Registry to cleanup
 */
void rule_registry_cleanup(rule_registry_t *registry);

/**
 * @brief Add rule to registry with automatic optimization
 * @param registry Rule registry
 * @param rule Rule to add (will be copied and optimized)
 * @return Rule ID, or 0 on failure
 */
uint32_t rule_registry_add_rule(rule_registry_t *registry, const rule_t *rule);

/**
 * @brief Remove rule from registry
 * @param registry Rule registry
 * @param rule_id Rule to remove
 * @return true if removed successfully
 */
bool rule_registry_remove_rule(rule_registry_t *registry, uint32_t rule_id);

/**
 * @brief Remove all rules created by specific tile
 * @param registry Rule registry
 * @param source_cell Cell that created the rules
 */
void rule_registry_remove_by_source(rule_registry_t *registry, grid_cell_t source_cell);

// --- High-Performance Evaluation ---

/**
 * @brief Initialize rule evaluation context
 * @param context Context to initialize
 * @param board Game board
 * @param registry Rule registry
 * @param temp_buffer_size Size of temporary buffers for batch operations
 * @return true on success
 */
bool rule_context_init(rule_context_t *context, const board_t *board,
                      const rule_registry_t *registry, uint32_t temp_buffer_size);

/**
 * @brief Cleanup rule evaluation context
 * @param context Context to cleanup
 */
void rule_context_cleanup(rule_context_t *context);

/**
 * @brief Calculate effective production for a tile (with caching)
 * @param registry Rule registry
 * @param context Evaluation context
 * @param tile Tile to calculate production for
 * @return Effective production value
 */
float rule_calculate_tile_production(rule_registry_t *registry, rule_context_t *context,
                                    const tile_t *tile);

/**
 * @brief Calculate effective range for a tile (with caching)
 * @param registry Rule registry
 * @param context Evaluation context
 * @param tile Tile to calculate range for
 * @return Effective range value
 */
uint8_t rule_calculate_tile_range(rule_registry_t *registry, rule_context_t *context,
                                 const tile_t *tile);

/**
 * @brief Get perceived tile type (with caching)
 * @param registry Rule registry
 * @param context Evaluation context
 * @param tile Tile to get perceived type for
 * @param observer_cell Cell observing the tile
 * @return Perceived tile type
 */
tile_type_t rule_calculate_perceived_type(rule_registry_t *registry, rule_context_t *context,
                                         const tile_t *tile, grid_cell_t observer_cell);

// --- Incremental Updates ---

/**
 * @brief Mark tile as needing rule recalculation
 * @param registry Rule registry
 * @param tile_index Index of tile that changed
 */
void rule_registry_mark_tile_dirty(rule_registry_t *registry, uint32_t tile_index);

/**
 * @brief Mark area around cell as needing recalculation
 * @param registry Rule registry
 * @param cell Center of area that changed
 * @param radius Radius of area to mark dirty
 */
void rule_registry_mark_area_dirty(rule_registry_t *registry, grid_cell_t cell, uint8_t radius);

/**
 * @brief Process all dirty tiles in batch for maximum performance
 * @param registry Rule registry
 * @param context Evaluation context
 */
void rule_registry_process_dirty_tiles(rule_registry_t *registry, rule_context_t *context);

/**
 * @brief Enable/disable batch mode for bulk operations
 * @param registry Rule registry
 * @param enabled true to defer updates until batch_process
 */
void rule_registry_set_batch_mode(rule_registry_t *registry, bool enabled);

// --- Spatial Query Optimization ---

/**
 * @brief Get all tiles of specific type within range (cached)
 * @param registry Rule registry
 * @param context Evaluation context
 * @param center_cell Center of search
 * @param tile_type Type of tiles to find
 * @param range Search radius
 * @param out_tiles Array to store found tiles
 * @param max_tiles Maximum tiles to return
 * @return Number of tiles found
 */
uint32_t rule_get_tiles_in_range(rule_registry_t *registry, rule_context_t *context,
                                grid_cell_t center_cell, tile_type_t tile_type,
                                uint8_t range, tile_t **out_tiles, uint32_t max_tiles);

/**
 * @brief Count tiles of specific type within range (cached)
 * @param registry Rule registry
 * @param context Evaluation context
 * @param center_cell Center of search
 * @param tile_type Type of tiles to count
 * @param range Search radius
 * @return Number of tiles found
 */
uint32_t rule_count_tiles_in_range(rule_registry_t *registry, rule_context_t *context,
                                  grid_cell_t center_cell, tile_type_t tile_type, uint8_t range);

// --- Rule Factory Functions ---

/**
 * @brief Create neighbor-based production bonus rule
 * @param source_cell Cell creating the rule
 * @param neighbor_type Type of neighbors to count
 * @param bonus_per_neighbor Bonus per matching neighbor
 * @param range Range to search for neighbors
 * @return Created rule
 */
rule_t rule_create_neighbor_bonus(grid_cell_t source_cell, tile_type_t neighbor_type,
                                 float bonus_per_neighbor, uint8_t range);

/**
 * @brief Create range modification rule
 * @param source_cell Cell creating the rule
 * @param target_type Type of tiles to affect (TILE_UNDEFINED for self)
 * @param range_delta Change in range (+/-)
 * @return Created rule
 */
rule_t rule_create_range_modifier(grid_cell_t source_cell, tile_type_t target_type,
                                 int8_t range_delta);

/**
 * @brief Create type perception override rule
 * @param source_cell Cell creating the rule
 * @param override_type Type to make neighbors appear as
 * @param range Range of override effect
 * @return Created rule
 */
rule_t rule_create_type_override(grid_cell_t source_cell, tile_type_t override_type,
                                uint8_t range);

/**
 * @brief Create pool size scaling rule
 * @param source_cell Cell creating the rule
 * @param base_bonus Base production bonus
 * @param scale_factor Bonus multiplier per tile in pool
 * @return Created rule
 */
rule_t rule_create_pool_scaling(grid_cell_t source_cell, float base_bonus, float scale_factor);

/**
 * @brief Create board-wide type modifier rule
 * @param source_cell Cell creating the rule
 * @param target_type Type of tiles to affect
 * @param modifier Flat modifier to apply
 * @return Created rule
 */
rule_t rule_create_global_modifier(grid_cell_t source_cell, tile_type_t target_type,
                                  float modifier);

// --- Cache Management ---

/**
 * @brief Invalidate all cached rule results
 * @param registry Rule registry
 */
void rule_registry_invalidate_cache(rule_registry_t *registry);

/**
 * @brief Warm up caches by pre-calculating common patterns
 * @param registry Rule registry
 * @param context Evaluation context
 */
void rule_registry_warm_cache(rule_registry_t *registry, rule_context_t *context);

/**
 * @brief Get cache performance statistics
 * @param registry Rule registry
 * @param out_hit_rate Cache hit rate (0.0 to 1.0)
 * @param out_total_evaluations Total rule evaluations
 * @param out_cache_size Current cache usage
 */
void rule_registry_get_cache_stats(const rule_registry_t *registry, float *out_hit_rate,
                                  uint64_t *out_total_evaluations, uint32_t *out_cache_size);

// --- Debugging and Profiling ---

/**
 * @brief Print detailed rule registry statistics
 * @param registry Rule registry
 */
void rule_registry_print_stats(const rule_registry_t *registry);

/**
 * @brief Print performance profile for rule evaluation
 * @param registry Rule registry
 */
void rule_registry_print_performance(const rule_registry_t *registry);

/**
 * @brief Print all rules affecting a specific tile
 * @param registry Rule registry
 * @param tile_index Index of tile to analyze
 */
void rule_registry_print_tile_rules(const rule_registry_t *registry, uint32_t tile_index);

/**
 * @brief Validate rule registry internal consistency
 * @param registry Rule registry
 * @return true if registry is valid
 */
bool rule_registry_validate(const rule_registry_t *registry);

#endif // RULE_SYSTEM_H
