/**************************************************************************//**
 * @file rule_manager.h
 * @brief High-level rule management integration for single-player hexhex game
 *****************************************************************************/

#ifndef RULE_MANAGER_H
#define RULE_MANAGER_H

#include "rule_system.h"
#include "game/board.h"
#include "tile/tile.h"

// Forward declarations
typedef struct game game_t;

/**
 * @brief Rule evaluation timing for game phases
 */
typedef enum {
    RULE_TIMING_TILE_PLACED,          // After a tile is placed
    RULE_TIMING_TILE_REMOVED,         // After a tile is removed  
    RULE_TIMING_POOL_FORMED,          // After a pool is created/merged
    RULE_TIMING_POOL_DESTROYED,       // After a pool is destroyed
    RULE_TIMING_TURN_START,           // At the start of a turn
    RULE_TIMING_TURN_END,             // At the end of a turn
    RULE_TIMING_PRODUCTION_CALC,      // During production calculation
    RULE_TIMING_MANUAL                // Manually triggered evaluation
} rule_timing_t;

/**
 * @brief High-level rule manager for single-player games
 */
typedef struct rule_manager {
    rule_registry_t registry;          // Core rule registry
    rule_context_t context;            // Evaluation context
    
    // Game integration
    const board_t *board;              // Board this manager operates on
    bool initialized;                  // Manager initialization state
    bool rules_dirty;                  // True if rules need re-evaluation
    
    // Performance tracking
    uint32_t evaluations_this_turn;
    uint32_t total_evaluations;
    uint32_t rules_applied_this_turn;
} rule_manager_t;

// --- Rule Manager Lifecycle ---

/**
 * @brief Initialize rule manager for a single-player game
 * @param manager Rule manager to initialize
 * @param board Game board to manage rules for
 * @param max_tiles Expected maximum number of tiles
 * @return true on success, false on failure
 */
bool rule_manager_init(rule_manager_t *manager, const board_t *board, uint32_t max_tiles);

/**
 * @brief Cleanup rule manager and free resources
 * @param manager Rule manager to cleanup
 */
void rule_manager_cleanup(rule_manager_t *manager);

// --- Rule Management ---

/**
 * @brief Add a rule to the manager
 * @param manager Rule manager
 * @param rule Rule to add
 * @return Rule ID, or 0 on failure
 */
uint32_t rule_manager_add_rule(rule_manager_t *manager, const rule_t *rule);

/**
 * @brief Remove a rule from the manager
 * @param manager Rule manager
 * @param rule_id Rule to remove
 * @return true if removed successfully
 */
bool rule_manager_remove_rule(rule_manager_t *manager, uint32_t rule_id);

/**
 * @brief Remove all rules created by a specific tile
 * @param manager Rule manager
 * @param source_cell Cell that created the rules
 */
void rule_manager_remove_rules_by_source(rule_manager_t *manager, grid_cell_t source_cell);

// --- Game Event Integration ---

/**
 * @brief Notify rule manager that a tile was placed
 * @param manager Rule manager
 * @param tile Tile that was placed
 */
void rule_manager_on_tile_placed(rule_manager_t *manager, const tile_t *tile);

/**
 * @brief Notify rule manager that a tile was removed
 * @param manager Rule manager
 * @param cell Cell where tile was removed
 */
void rule_manager_on_tile_removed(rule_manager_t *manager, grid_cell_t cell);

/**
 * @brief Notify rule manager that a pool was formed or changed
 * @param manager Rule manager
 * @param pool Pool that was formed/changed
 */
void rule_manager_on_pool_changed(rule_manager_t *manager, const pool_t *pool);

/**
 * @brief Notify rule manager of turn start
 * @param manager Rule manager
 */
void rule_manager_on_turn_start(rule_manager_t *manager);

/**
 * @brief Notify rule manager of turn end
 * @param manager Rule manager
 */
void rule_manager_on_turn_end(rule_manager_t *manager);

// --- Rule Evaluation ---

/**
 * @brief Evaluate all active rules at specific timing
 * @param manager Rule manager
 * @param timing When this evaluation is happening
 */
void rule_manager_evaluate_rules(rule_manager_t *manager, rule_timing_t timing);

/**
 * @brief Calculate effective production for a tile considering all active rules
 * @param manager Rule manager
 * @param tile Tile to calculate production for
 * @return Effective production value
 */
float rule_manager_calculate_tile_production(rule_manager_t *manager, const tile_t *tile);

/**
 * @brief Calculate effective range for a tile considering all active rules
 * @param manager Rule manager
 * @param tile Tile to calculate range for
 * @return Effective range value
 */
uint8_t rule_manager_calculate_tile_range(rule_manager_t *manager, const tile_t *tile);

/**
 * @brief Calculate effective pool multiplier considering all active rules
 * @param manager Rule manager
 * @param pool Pool to calculate multiplier for
 * @return Effective multiplier value
 */
float rule_manager_calculate_pool_multiplier(rule_manager_t *manager, const pool_t *pool);

/**
 * @brief Get perceived tile type (may be overridden by rules)
 * @param manager Rule manager
 * @param tile Tile to get perceived type for
 * @param observer_cell Cell from which tile is being observed
 * @return Perceived tile type
 */
tile_type_t rule_manager_get_perceived_type(rule_manager_t *manager,
                                           const tile_t *tile,
                                           grid_cell_t observer_cell);

// --- Rule Creation Helpers ---

/**
 * @brief Add a neighbor bonus rule when tile is placed
 * @param manager Rule manager
 * @param source_tile Tile creating the rule
 * @param neighbor_type Type of neighbors to count
 * @param bonus_per_neighbor Bonus per matching neighbor
 * @param range Range to search for neighbors
 */
void rule_manager_add_neighbor_bonus(rule_manager_t *manager,
                                    const tile_t *source_tile,
                                    tile_type_t neighbor_type,
                                    float bonus_per_neighbor,
                                    uint8_t range);

/**
 * @brief Add a range modification rule when tile is placed
 * @param manager Rule manager
 * @param source_tile Tile creating the rule
 * @param target_type Type of tiles to affect
 * @param range_delta Change in range
 */
void rule_manager_add_range_modifier(rule_manager_t *manager,
                                    const tile_t *source_tile,
                                    tile_type_t target_type,
                                    int8_t range_delta);

/**
 * @brief Add a type override rule when tile is placed
 * @param manager Rule manager
 * @param source_tile Tile creating the rule
 * @param override_type Type to make neighbors appear as
 * @param range Range of override effect
 */
void rule_manager_add_type_override(rule_manager_t *manager,
                                   const tile_t *source_tile,
                                   tile_type_t override_type,
                                   uint8_t range);

// --- Performance and Debugging ---

/**
 * @brief Get rule evaluation performance stats
 * @param manager Rule manager
 * @param out_active_rules Number of currently active rules
 * @param out_evaluations_this_turn Evaluations performed this turn
 * @param out_total_evaluations Total evaluations since init
 */
void rule_manager_get_performance_stats(const rule_manager_t *manager,
                                       uint32_t *out_active_rules,
                                       uint32_t *out_evaluations_this_turn,
                                       uint32_t *out_total_evaluations);

/**
 * @brief Print rule manager state for debugging
 * @param manager Rule manager
 */
void rule_manager_debug_print(const rule_manager_t *manager);

/**
 * @brief Print all rules affecting a specific tile
 * @param manager Rule manager
 * @param tile Tile to analyze
 */
void rule_manager_debug_print_tile_rules(const rule_manager_t *manager, const tile_t *tile);

// --- Utility Functions ---

/**
 * @brief Check if rule manager needs re-evaluation
 * @param manager Rule manager
 * @return true if rules are dirty and need evaluation
 */
bool rule_manager_needs_evaluation(const rule_manager_t *manager);

/**
 * @brief Mark rule manager as needing re-evaluation
 * @param manager Rule manager
 */
void rule_manager_mark_dirty(rule_manager_t *manager);

/**
 * @brief Clear dirty flag after evaluation
 * @param manager Rule manager
 */
void rule_manager_mark_clean(rule_manager_t *manager);

/**
 * @brief Get total number of active rules
 * @param manager Rule manager
 * @return Number of active rules
 */
uint32_t rule_manager_get_rule_count(const rule_manager_t *manager);

/**
 * @brief Enable/disable batch processing mode
 * @param manager Rule manager
 * @param enabled true to enable batch mode
 */
void rule_manager_set_batch_mode(rule_manager_t *manager, bool enabled);

/**
 * @brief Process all pending rule updates in batch
 * @param manager Rule manager
 */
void rule_manager_process_batch_updates(rule_manager_t *manager);

#endif // RULE_MANAGER_H