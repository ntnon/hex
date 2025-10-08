/**************************************************************************//**
 * @file rule_system.h
 * @brief Player-driven rule system with flexible rule management and priority-based evaluation
 *****************************************************************************/

#ifndef RULE_SYSTEM_H
#define RULE_SYSTEM_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "grid/grid_types.h"
#include "tile/tile.h"

// Forward declarations
typedef struct tile_pool pool_t;
typedef struct board board_t;
typedef struct player player_t;

// --- Rule System Constants ---
#define MAX_RULE_NAME_LENGTH 64
#define MAX_RULE_DESCRIPTION_LENGTH 256
#define MAX_RULE_HINT_LENGTH 128
#define MAX_RULE_CONDITIONS 8
#define MAX_RULE_EFFECTS 4
#define MAX_RULE_CONFLICTS 16
#define MAX_RULE_SYNERGIES 8
#define MAX_RULE_PREREQUISITES 4

// --- Rule System Enums ---

/**
 * @brief Defines rule lifecycle behavior
 */
typedef enum {
    RULE_LIFECYCLE_INSTANT,     // Execute once, modify base values, then self-destruct
    RULE_LIFECYCLE_PERSISTENT   // Stay active, evaluated during each calculation cycle
} rule_lifecycle_t;

/**
 * @brief Rule evaluation priority (lower numbers = earlier evaluation)
 */
typedef enum {
    RULE_PRIORITY_CRITICAL = 0,     // Range modifications, perception overrides
    RULE_PRIORITY_HIGH = 10,        // Base value modifications
    RULE_PRIORITY_NORMAL = 50,      // Most production bonuses
    RULE_PRIORITY_LOW = 100,        // Secondary effects
    RULE_PRIORITY_COSMETIC = 200    // Visual/UI effects only
} rule_priority_t;

/**
 * @brief Defines what the rule affects
 */
typedef enum {
    RULE_SCOPE_SELF,           // Rule affects only the source tile/pool
    RULE_SCOPE_NEIGHBORS,      // Rule affects neighboring tiles/pools
    RULE_SCOPE_POOL,           // Rule affects entire pool containing source
    RULE_SCOPE_TYPE_GLOBAL,    // Rule affects all tiles/pools of specific type globally
    RULE_SCOPE_GLOBAL          // Rule affects entire board
} rule_scope_t;

/**
 * @brief Defines what aspect of the target is modified
 */
typedef enum {
    RULE_TARGET_PRODUCTION,     // Tile production value
    RULE_TARGET_RANGE,          // Tile interaction range
    RULE_TARGET_POOL_MODIFIER,  // Pool production multiplier
    RULE_TARGET_PERCEPTION,     // How tiles appear to other rules (color override)
    RULE_TARGET_PLACEMENT,      // Tile placement rules/costs
    RULE_TARGET_MOVEMENT        // Tile movement/repositioning
} rule_target_t;

/**
 * @brief Defines different types of rule conditions (can be combined with AND/OR)
 */
typedef enum {
    RULE_CONDITION_ALWAYS,              // Rule always applies
    RULE_CONDITION_SELF_TYPE,           // Based on source tile/pool type
    RULE_CONDITION_NEIGHBOR_COUNT,      // Based on number of specific neighbors
    RULE_CONDITION_NEIGHBOR_TYPE,       // Based on neighbor tile types
    RULE_CONDITION_POOL_SIZE,           // Based on pool size
    RULE_CONDITION_TOTAL_TILE_COUNT,    // Based on total tiles of type on board
    RULE_CONDITION_GEOMETRIC,           // Based on geometric properties
    RULE_CONDITION_VALUE_THRESHOLD,     // Based on value thresholds
    RULE_CONDITION_RULE_ACTIVE,         // Based on other active rules
    RULE_CONDITION_AND,                 // Logical AND of sub-conditions
    RULE_CONDITION_OR,                  // Logical OR of sub-conditions
    RULE_CONDITION_NOT                  // Logical NOT of sub-condition
} rule_condition_type_t;

/**
 * @brief Defines different types of rule effects
 */
typedef enum {
    RULE_EFFECT_ADD_FLAT,       // Add fixed amount
    RULE_EFFECT_ADD_PERCENT,    // Add percentage of base value
    RULE_EFFECT_MULTIPLY,       // Multiply by factor
    RULE_EFFECT_SET_VALUE,      // Set to specific value
    RULE_EFFECT_SCALE_BY_COUNT, // Scale effect by count of something
    RULE_EFFECT_OVERRIDE_TYPE,  // Change perceived type
    RULE_EFFECT_MODIFY_RANGE,   // Modify tile range
    RULE_EFFECT_CONDITIONAL     // Apply different effects based on conditions
} rule_effect_type_t;

/**
 * @brief Rule availability and player choice status
 */
typedef enum {
    RULE_STATUS_LOCKED,         // Not yet available to player
    RULE_STATUS_AVAILABLE,      // Can be acquired by player
    RULE_STATUS_ACTIVE,         // Currently active and affecting gameplay
    RULE_STATUS_DISABLED,       // Temporarily disabled (e.g., due to conflicts)
    RULE_STATUS_EXHAUSTED       // One-time rule that has been used
} rule_status_t;

// --- Rule System Structures ---

/**
 * @brief Forward declaration for recursive condition structures
 */
typedef struct rule_condition rule_condition_t;

/**
 * @brief Union for different condition parameters
 */
typedef union {
    struct {
        tile_type_t tile_type;
    } self_type;
    
    struct {
        int min_count;
        int max_count;
        tile_type_t neighbor_type;
        int range;              // Range to search for neighbors
    } neighbor_count;
    
    struct {
        tile_type_t required_type;
        int range;              // Range to search
    } neighbor_type;
    
    struct {
        int min_size;
        int max_size;
    } pool_size;
    
    struct {
        tile_type_t tile_type;
        int min_count;
        int max_count;
    } total_tile_count;
    
    struct {
        float min_compactness;
        int min_diameter;
        int max_diameter;
    } geometric;
    
    struct {
        float threshold;
        bool greater_than;
        rule_target_t value_source;  // What value to check
    } value_threshold;
    
    struct {
        uint32_t required_rule_id;
    } rule_active;
    
    struct {
        rule_condition_t *conditions;
        size_t condition_count;
    } logical;
} rule_condition_params_t;

/**
 * @brief Condition structure for rules
 */
struct rule_condition {
    rule_condition_type_t type;
    rule_condition_params_t params;
};

/**
 * @brief Union for different effect parameters
 */
typedef union {
    float flat_value;
    float percentage;
    float multiplier;
    float set_value;
    
    struct {
        float base_value;
        float scale_factor;
        rule_condition_type_t count_source;  // What to count for scaling
        rule_condition_params_t count_params;
    } scale_by_count;
    
    tile_type_t override_type;
    
    struct {
        int range_delta;
        int min_range;
        int max_range;
    } range_modification;
    
    struct {
        rule_condition_t condition;
        struct rule_effect *true_effect;
        struct rule_effect *false_effect;
    } conditional;
} rule_effect_params_t;

/**
 * @brief Effect structure for rules
 */
typedef struct rule_effect {
    rule_effect_type_t type;
    rule_target_t target;
    rule_effect_params_t params;
    float multiplier;           // Global multiplier for this effect (for synergies)
} rule_effect_t;

/**
 * @brief Core rule structure (internal game logic)
 */
typedef struct rule {
    uint32_t id;                    // Unique rule identifier
    rule_lifecycle_t lifecycle;    // Instant or persistent
    int priority;                   // Evaluation priority (lower = earlier)
    rule_scope_t scope;            // What the rule affects
    
    rule_condition_t condition;    // When rule applies
    rule_effect_t *effects;        // What the rule does (array)
    size_t effect_count;          // Number of effects
    
    // Source information
    grid_cell_t source_cell;       // Cell that created this rule
    bool is_tile_source;           // True if source is tile, false if pool
    uint32_t player_id;            // Player who owns this rule
    
    // Spatial data for efficient queries
    grid_cell_t *affected_cells;   // Cells this rule affects (for spatial indexing)
    size_t affected_count;         // Number of affected cells
    
    // Execution state
    bool executed;                 // For instant rules
    bool is_active;               // Can be temporarily disabled
    
    struct rule *next;             // For linked list in hash buckets
} rule_t;

/**
 * @brief Player-facing rule definition with metadata and costs
 */
typedef struct player_rule {
    // Core rule logic
    rule_t rule_template;          // Template for creating active rules
    
    // Player choice metadata
    char name[MAX_RULE_NAME_LENGTH];
    char description[MAX_RULE_DESCRIPTION_LENGTH];
    char strategic_hint[MAX_RULE_HINT_LENGTH];
    
    // Rule management
    rule_status_t status;          // Current availability status
    int cost_to_acquire;          // Resource cost to add this rule
    int cost_to_remove;           // Resource cost to remove (-1 if permanent)
    int max_instances;            // How many copies can player have (0 = unlimited)
    int current_instances;        // How many copies player currently has
    
    // Rule relationships
    uint32_t conflicts_with[MAX_RULE_CONFLICTS];     // Rule IDs that can't coexist
    size_t conflict_count;
    
    uint32_t synergizes_with[MAX_RULE_SYNERGIES];    // Rules that enhance this one
    float synergy_multipliers[MAX_RULE_SYNERGIES];   // Multiplier for each synergy
    size_t synergy_count;
    
    uint32_t prerequisites[MAX_RULE_PREREQUISITES];   // Rules required before this one
    size_t prerequisite_count;
    
    // Unlock conditions
    rule_condition_t unlock_condition;  // Condition to make this rule available
    
    // Visual/UI
    uint32_t icon_id;             // Icon for UI display
    uint32_t category_id;         // Category for organization
} player_rule_t;

/**
 * @brief Spatial hash map for efficient rule lookup
 */
#define RULE_HASH_SIZE 1024

typedef struct {
    rule_t *buckets[RULE_HASH_SIZE];  // Hash buckets for rules by cell
    rule_t **all_rules;               // Array of all rules for iteration
    size_t rule_count;                // Number of active rules
    size_t rule_capacity;             // Capacity of all_rules array
    uint32_t next_rule_id;            // Next available rule ID
} rule_registry_t;

/**
 * @brief Player rule catalog and management
 */
typedef struct {
    player_rule_t *rule_catalog;      // All possible rules in the game
    size_t catalog_size;              // Number of rules in catalog
    
    player_rule_t **player_available; // Rules available per player
    player_rule_t **player_active;    // Rules active per player  
    size_t *player_available_count;   // Count per player
    size_t *player_active_count;      // Count per player
    
    int *player_rule_points;          // Currency for buying rules per player
    size_t player_count;              // Number of players
} rule_catalog_t;

/**
 * @brief Context for rule evaluation
 */
typedef struct {
    const board_t *board;
    const tile_t *current_tile;
    const pool_t *current_pool;
    grid_cell_t current_cell;
    uint32_t current_player_id;
    
    // Perception state (modified during high-priority rules)
    tile_type_t *perceived_types;   // Override tile types
    float *perceived_values;        // Override tile values
    int *perceived_ranges;          // Override tile ranges
    size_t perception_capacity;     // Size of perception arrays
    
    // Evaluation state
    bool *rules_processed;          // Track which rules have been processed this cycle
    size_t rules_processed_capacity;
} rule_context_t;

// --- Rule Registry Functions ---

/**
 * @brief Initialize a rule registry
 */
bool rule_registry_init(rule_registry_t *registry);

/**
 * @brief Cleanup and free rule registry
 */
void rule_registry_cleanup(rule_registry_t *registry);

/**
 * @brief Add a rule to the registry
 */
uint32_t rule_registry_add(rule_registry_t *registry, const rule_t *rule);

/**
 * @brief Remove a rule from the registry
 */
bool rule_registry_remove(rule_registry_t *registry, uint32_t rule_id);

/**
 * @brief Get all rules affecting a specific cell
 */
void rule_registry_get_rules_for_cell(const rule_registry_t *registry, 
                                     grid_cell_t cell,
                                     rule_t ***out_rules, 
                                     size_t *out_count);

/**
 * @brief Remove all rules created by a specific source
 */
void rule_registry_remove_by_source(rule_registry_t *registry, 
                                   grid_cell_t source_cell, 
                                   bool is_tile_source);

// --- Rule Catalog Functions ---

/**
 * @brief Initialize rule catalog for all players
 */
bool rule_catalog_init(rule_catalog_t *catalog, size_t player_count);

/**
 * @brief Cleanup rule catalog
 */
void rule_catalog_cleanup(rule_catalog_t *catalog);

/**
 * @brief Add a rule definition to the catalog
 */
bool rule_catalog_add_rule(rule_catalog_t *catalog, const player_rule_t *rule);

/**
 * @brief Check if player can acquire a rule
 */
bool rule_catalog_can_acquire(const rule_catalog_t *catalog, uint32_t player_id, 
                             uint32_t rule_id);

/**
 * @brief Attempt to acquire a rule for a player
 */
bool rule_catalog_acquire_rule(rule_catalog_t *catalog, uint32_t player_id, 
                              uint32_t rule_id);

/**
 * @brief Remove a rule from a player
 */
bool rule_catalog_remove_rule(rule_catalog_t *catalog, uint32_t player_id, 
                             uint32_t rule_id);

/**
 * @brief Get all available rules for a player
 */
void rule_catalog_get_available(const rule_catalog_t *catalog, uint32_t player_id,
                               player_rule_t ***out_rules, size_t *out_count);

/**
 * @brief Get all active rules for a player
 */
void rule_catalog_get_active(const rule_catalog_t *catalog, uint32_t player_id,
                            player_rule_t ***out_rules, size_t *out_count);

/**
 * @brief Update rule availability based on unlock conditions
 */
void rule_catalog_update_availability(rule_catalog_t *catalog, uint32_t player_id,
                                     const rule_context_t *context);

/**
 * @brief Calculate synergy multipliers for a player's active rules
 */
void rule_catalog_calculate_synergies(rule_catalog_t *catalog, uint32_t player_id);

// --- Rule Factory Functions ---

/**
 * @brief Create a neighbor-based production bonus rule
 */
player_rule_t rule_factory_neighbor_bonus(const char *name, 
                                         tile_type_t neighbor_type,
                                         float bonus_per_neighbor,
                                         int range);

/**
 * @brief Create a perception override rule
 */
player_rule_t rule_factory_perception_override(const char *name,
                                              tile_type_t override_type,
                                              int range);

/**
 * @brief Create a pool size scaling rule
 */
player_rule_t rule_factory_pool_scaling(const char *name,
                                       int min_size,
                                       float bonus_percentage);

/**
 * @brief Create a type-based range modifier rule
 */
player_rule_t rule_factory_type_range_modifier(const char *name,
                                              tile_type_t target_type,
                                              int range_delta);

/**
 * @brief Create a conditional scaling rule
 */
player_rule_t rule_factory_conditional_scaling(const char *name,
                                              rule_condition_t condition,
                                              float scale_factor,
                                              rule_condition_type_t count_source);

// --- Rule Evaluation Functions ---

/**
 * @brief Initialize rule evaluation context
 */
bool rule_context_init(rule_context_t *context, const board_t *board, size_t max_rules);

/**
 * @brief Cleanup rule evaluation context
 */
void rule_context_cleanup(rule_context_t *context);

/**
 * @brief Evaluate all rules in priority order
 */
void rule_evaluate_all(const rule_registry_t *registry, rule_context_t *context);

/**
 * @brief Check if a rule condition is met
 */
bool rule_check_condition(const rule_condition_t *condition, const rule_context_t *context);

/**
 * @brief Apply a rule effect
 */
void rule_apply_effect(const rule_effect_t *effect, rule_context_t *context);

/**
 * @brief Execute all instant rules and remove them
 */
void rule_execute_instant_rules(rule_registry_t *registry, rule_context_t *context);

// --- Utility Functions ---

/**
 * @brief Calculate hash for spatial indexing
 */
uint32_t rule_hash_cell(grid_cell_t cell);

/**
 * @brief Print rule information for debugging
 */
void rule_print(const rule_t *rule);

/**
 * @brief Print player rule information for debugging
 */
void player_rule_print(const player_rule_t *rule);

/**
 * @brief Print registry statistics for debugging
 */
void rule_registry_print_stats(const rule_registry_t *registry);

/**
 * @brief Print catalog statistics for debugging
 */
void rule_catalog_print_stats(const rule_catalog_t *catalog);

/**
 * @brief Validate rule for logical consistency
 */
bool rule_validate(const player_rule_t *rule);

/**
 * @brief Check for conflicts between two rules
 */
bool rule_check_conflict(const player_rule_t *rule1, const player_rule_t *rule2);

#endif // RULE_SYSTEM_H