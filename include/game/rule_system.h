/**************************************************************************//**
 * @file rule_system.h
 * @brief Comprehensive rule system for hexhex game with centralized registry
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

// --- Rule System Enums ---

/**
 * @brief Defines rule lifecycle behavior
 */
typedef enum {
    RULE_LIFECYCLE_INSTANT,     // Execute once, modify base values, then self-destruct
    RULE_LIFECYCLE_PERSISTENT   // Stay active, evaluated during each calculation cycle
} rule_lifecycle_t;

/**
 * @brief Defines when persistent rules are evaluated
 */
typedef enum {
    RULE_PHASE_PERCEPTION,      // Modify how game state is "seen" (color overrides, etc.)
    RULE_PHASE_CALCULATION      // Apply bonuses/penalties using effective state
} rule_phase_t;

/**
 * @brief Defines what the rule affects
 */
typedef enum {
    RULE_SCOPE_TILE,           // Rule affects a single tile
    RULE_SCOPE_POOL,           // Rule affects a pool
    RULE_SCOPE_NEIGHBORS,      // Rule affects neighboring tiles/pools
    RULE_SCOPE_GLOBAL          // Rule affects entire board
} rule_scope_t;

/**
 * @brief Defines what aspect of the target is modified
 */
typedef enum {
    RULE_TARGET_BASE_VALUE,     // tile->data.value or pool base properties
    RULE_TARGET_MODIFIER,       // tile->modifier or pool->modifier
    RULE_TARGET_PERCEPTION,     // How tile/pool appears to other rules
    RULE_TARGET_RANGE          // tile->range
} rule_target_t;

/**
 * @brief Defines different types of rule conditions
 */
typedef enum {
    RULE_CONDITION_ALWAYS,              // Rule always applies
    RULE_CONDITION_NEIGHBOR_COUNT,      // Based on number of specific neighbors
    RULE_CONDITION_NEIGHBOR_TYPE,       // Based on neighbor tile types
    RULE_CONDITION_POOL_SIZE,           // Based on pool size
    RULE_CONDITION_GEOMETRIC,           // Based on geometric properties
    RULE_CONDITION_VALUE_THRESHOLD      // Based on value thresholds
} rule_condition_type_t;

/**
 * @brief Defines different types of rule effects
 */
typedef enum {
    RULE_EFFECT_ADD_FLAT,       // Add fixed amount
    RULE_EFFECT_ADD_PERCENT,    // Add percentage
    RULE_EFFECT_MULTIPLY,       // Multiply by factor
    RULE_EFFECT_SET_VALUE,      // Set to specific value
    RULE_EFFECT_OVERRIDE_TYPE,  // Change perceived type
    RULE_EFFECT_EXTEND_RANGE    // Modify tile range
} rule_effect_type_t;

// --- Rule System Structures ---

/**
 * @brief Union for different condition parameters
 */
typedef union {
    struct {
        int min_count;
        int max_count;
        tile_type_t neighbor_type;
    } neighbor_count;
    
    struct {
        tile_type_t required_type;
    } neighbor_type;
    
    struct {
        int min_size;
        int max_size;
    } pool_size;
    
    struct {
        float min_compactness;
        int min_diameter;
    } geometric;
    
    struct {
        float threshold;
        bool greater_than;
    } value_threshold;
} rule_condition_params_t;

/**
 * @brief Condition structure for rules
 */
typedef struct {
    rule_condition_type_t type;
    rule_condition_params_t params;
} rule_condition_t;

/**
 * @brief Union for different effect parameters
 */
typedef union {
    float flat_value;
    float percentage;
    float multiplier;
    float set_value;
    tile_type_t override_type;
    int range_delta;
} rule_effect_params_t;

/**
 * @brief Effect structure for rules
 */
typedef struct {
    rule_effect_type_t type;
    rule_effect_params_t params;
} rule_effect_t;

/**
 * @brief Core rule structure
 */
typedef struct rule {
    uint32_t id;                    // Unique rule identifier
    rule_lifecycle_t lifecycle;    // Instant or persistent
    rule_phase_t phase;            // When to evaluate (for persistent rules)
    rule_scope_t scope;            // What the rule affects
    rule_target_t target;          // What aspect is modified
    
    rule_condition_t condition;    // When rule applies
    rule_effect_t effect;          // What the rule does
    
    // Source information
    grid_cell_t source_cell;       // Cell that created this rule
    bool is_tile_source;           // True if source is tile, false if pool
    
    // Spatial data for efficient queries
    grid_cell_t *affected_cells;   // Cells this rule affects (for spatial indexing)
    size_t affected_count;         // Number of affected cells
    
    // Execution state
    bool executed;                 // For instant rules
    int priority;                  // Execution order (lower = earlier)
    
    struct rule *next;             // For linked list in hash buckets
} rule_t;

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
 * @brief Context for rule evaluation
 */
typedef struct {
    const board_t *board;
    const tile_t *current_tile;
    const pool_t *current_pool;
    grid_cell_t current_cell;
    
    // Perception state (modified during PERCEPTION phase)
    tile_type_t *perceived_types;   // Override tile types
    float *perceived_values;        // Override tile values
    size_t perception_capacity;     // Size of perception arrays
} rule_context_t;

// --- Rule Registry Functions ---

/**
 * @brief Initialize a rule registry
 * @param registry Pointer to registry to initialize
 * @return true on success, false on failure
 */
bool rule_registry_init(rule_registry_t *registry);

/**
 * @brief Cleanup and free rule registry
 * @param registry Pointer to registry to cleanup
 */
void rule_registry_cleanup(rule_registry_t *registry);

/**
 * @brief Add a rule to the registry
 * @param registry Pointer to rule registry
 * @param rule Pointer to rule to add (will be copied)
 * @return Assigned rule ID, or 0 on failure
 */
uint32_t rule_registry_add(rule_registry_t *registry, const rule_t *rule);

/**
 * @brief Remove a rule from the registry
 * @param registry Pointer to rule registry
 * @param rule_id ID of rule to remove
 * @return true if removed, false if not found
 */
bool rule_registry_remove(rule_registry_t *registry, uint32_t rule_id);

/**
 * @brief Get all rules affecting a specific cell
 * @param registry Pointer to rule registry
 * @param cell Cell to query
 * @param out_rules Pointer to store array of rule pointers
 * @param out_count Pointer to store number of rules found
 * @note Caller should NOT free the returned array - it's managed by registry
 */
void rule_registry_get_rules_for_cell(const rule_registry_t *registry, 
                                     grid_cell_t cell,
                                     rule_t ***out_rules, 
                                     size_t *out_count);

/**
 * @brief Remove all rules created by a specific source
 * @param registry Pointer to rule registry
 * @param source_cell Source cell that created the rules
 * @param is_tile_source True if source is a tile, false if pool
 */
void rule_registry_remove_by_source(rule_registry_t *registry, 
                                   grid_cell_t source_cell, 
                                   bool is_tile_source);

// --- Rule Factory Functions ---

/**
 * @brief Create a simple neighbor bonus rule
 * @param source_cell Cell creating the rule
 * @param neighbor_type Type of neighbors to count
 * @param bonus_per_neighbor Bonus amount per matching neighbor
 * @return Created rule (caller owns)
 */
rule_t rule_factory_neighbor_bonus(grid_cell_t source_cell,
                                  tile_type_t neighbor_type,
                                  float bonus_per_neighbor);

/**
 * @brief Create a perception override rule (makes neighbors appear different)
 * @param source_cell Cell creating the rule  
 * @param override_type Type to make neighbors appear as
 * @param range Range of effect
 * @return Created rule (caller owns)
 */
rule_t rule_factory_perception_override(grid_cell_t source_cell,
                                       tile_type_t override_type,
                                       int range);

/**
 * @brief Create a pool size bonus rule
 * @param source_cell Cell creating the rule
 * @param min_size Minimum pool size for bonus
 * @param bonus_percentage Percentage bonus for large pools
 * @return Created rule (caller owns)
 */
rule_t rule_factory_pool_size_bonus(grid_cell_t source_cell,
                                   int min_size,
                                   float bonus_percentage);

/**
 * @brief Create an instant modifier rule
 * @param source_cell Cell creating the rule
 * @param target What to modify
 * @param flat_bonus Amount to add
 * @return Created rule (caller owns)
 */
rule_t rule_factory_instant_modifier(grid_cell_t source_cell,
                                    rule_target_t target,
                                    float flat_bonus);

// --- Rule Evaluation Functions ---

/**
 * @brief Initialize rule evaluation context
 * @param context Pointer to context to initialize
 * @param board Board to evaluate on
 * @return true on success, false on failure
 */
bool rule_context_init(rule_context_t *context, const board_t *board);

/**
 * @brief Cleanup rule evaluation context
 * @param context Pointer to context to cleanup
 */
void rule_context_cleanup(rule_context_t *context);

/**
 * @brief Evaluate all persistent rules for a specific phase
 * @param registry Rule registry
 * @param context Evaluation context
 * @param phase Phase to evaluate
 */
void rule_evaluate_phase(const rule_registry_t *registry,
                        rule_context_t *context,
                        rule_phase_t phase);

/**
 * @brief Check if a rule condition is met
 * @param rule Rule to check
 * @param context Evaluation context
 * @return true if condition is met
 */
bool rule_check_condition(const rule_t *rule, const rule_context_t *context);

/**
 * @brief Apply a rule effect
 * @param rule Rule to apply
 * @param context Evaluation context (may be modified)
 */
void rule_apply_effect(const rule_t *rule, rule_context_t *context);

/**
 * @brief Execute all instant rules and remove them
 * @param registry Rule registry
 * @param context Evaluation context
 */
void rule_execute_instant_rules(rule_registry_t *registry, rule_context_t *context);

// --- Utility Functions ---

/**
 * @brief Calculate hash for spatial indexing
 * @param cell Grid cell to hash
 * @return Hash value for indexing
 */
uint32_t rule_hash_cell(grid_cell_t cell);

/**
 * @brief Print rule information for debugging
 * @param rule Rule to print
 */
void rule_print(const rule_t *rule);

/**
 * @brief Print registry statistics for debugging
 * @param registry Rule registry
 */
void rule_registry_print_stats(const rule_registry_t *registry);

#endif // RULE_SYSTEM_H