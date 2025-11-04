#include <stdint.h>
#include <stdlib.h>
#include "game/board.h"
#include "stdbool.h"
#include "tile/tile.h"

typedef enum {
    RULE_INSTANT,     // Execute once, modify base values, removed
    RULE_PERSISTENT   // Stay active, evaluated during calculations
} rule_lifecycle_t;

typedef enum {
    PHASE_ON_PLACEMENT,   // When tile is placed (instant rules)
    PHASE_ON_REMOVAL,     // When tile is removed (cleanup)
    PHASE_CALCULATION,    // During production/value calculations
    PHASE_START_TURN,     // Beginning of turn
    PHASE_END_TURN        // End of turn
} rule_phase_t;

typedef enum {
    SCOPE_SELF,       // Affects only the source tile
    SCOPE_NEIGHBORS,  // Affects adjacent tiles
    SCOPE_POOL,       // Affects tiles in same pool
    SCOPE_GLOBAL      // Affects all tiles/game state
} rule_scope_t;

typedef enum {
    TARGET_PRODUCTION,      // Modify tile production
    TARGET_POOL_MULTIPLIER, // Modify pool multiplier
    TARGET_COST,           // Modify placement/action costs
    TARGET_CUSTOM          // For special game mechanics
} rule_target_t;

// Simple condition structure
typedef struct {
    enum {
        COND_ALWAYS,
        COND_NEIGHBOR_COUNT,    // Has X neighbors of type Y
        COND_SAME_COLOR_NEIGHBORS,
        COND_POOL_SIZE,         // Pool has min/max tiles
        COND_TILE_TYPE,         // Tile is specific type
        COND_CUSTOM             // For complex conditions
    } type;

    union {
        struct {
            tile_type_t type;
            int min, max;  // -1 for no limit
        } neighbors;

        struct {
            int min, max;
        } pool_size;

        tile_type_t tile_type;

        void *custom_data;  // For extensions
    } params;
} rule_condition_t;

// Simple effect structure
typedef struct {
    enum {
        EFFECT_ADD,      // Add value
        EFFECT_MULTIPLY, // Multiply by value
        EFFECT_SET,      // Set to value
        EFFECT_ADD_PER,  // Add X per Y (e.g., +1 per neighbor)
    } type;

    float value;
    float per_value;  // For ADD_PER effects
} rule_effect_t;

typedef struct {
    uint32_t id;
    grid_cell_t source;        // Tile that created this rule

    rule_lifecycle_t lifecycle;
    rule_phase_t phase;
    rule_scope_t scope;
    rule_target_t target;

    rule_condition_t condition;
    rule_effect_t effect;

    bool active;  // Can be toggled without removal
} rule_t;

typedef struct {
    rule_t *rules;
    size_t count;
    size_t capacity;
    uint32_t next_id;
} rule_registry_t;

// Core API - minimal and focused
rule_registry_t* rule_registry_create(void);
void rule_registry_destroy(rule_registry_t *registry);

// Add/remove rules
uint32_t rule_add(rule_registry_t *registry, const rule_t *rule);
void rule_remove(rule_registry_t *registry, uint32_t id);
void rule_remove_by_source(rule_registry_t *registry, grid_cell_t source);

// Apply instant rules (modifies base values directly)
void rule_apply_instant(rule_registry_t *registry, rule_t *rule,
                       board_t *board, tile_t *tile);

// Calculate values with persistent rules (no caching yet)
float rule_calc_production(rule_registry_t *registry, board_t *board,
                          tile_t *tile, grid_cell_t cell);
float rule_calc_pool_multiplier(rule_registry_t *registry, board_t *board,
                               pool_t *pool);

// Execute phase-based rules
void rule_execute_phase(rule_registry_t *registry, board_t *board,
                       rule_phase_t phase);

// Helper to create common rules
rule_t rule_make_neighbor_bonus(grid_cell_t source, tile_type_t neighbor_type,
                               float bonus_per);
rule_t rule_make_pool_multiplier(grid_cell_t source, float multiplier);
rule_t rule_make_production_boost(grid_cell_t source, float boost);
