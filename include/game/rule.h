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
