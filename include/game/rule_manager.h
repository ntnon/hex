/**************************************************************************//**
 * @file rule_manager.h
 * @brief High-level rule management integration for hexhex game
 *****************************************************************************/

#ifndef RULE_MANAGER_H
#define RULE_MANAGER_H

#include "rule_system.h"
#include "game/board.h"
#include "tile/tile.h"

// Forward declarations
typedef struct game game_t;

/**
 * @brief High-level rule manager that integrates with game systems
 */
typedef struct rule_manager {
    rule_registry_t registry;      // Core rule registry
    rule_context_t context;        // Evaluation context
    
    // Integration state
    bool initialized;
    bool dirty;                    // True if rules need re-evaluation
    
    // Performance tracking
    uint32_t evaluations_this_frame;
    uint32_t total_evaluations;
} rule_manager_t;

// --- Rule Manager Lifecycle ---

/**
 * @brief Initialize rule manager for a game
 * @param manager Rule manager to initialize
 * @param game Game instance to manage rules for
 * @return true on success, false on failure
 */
bool rule_manager_init(rule_manager_t *manager, const game_t *game);

/**
 * @brief Cleanup rule manager and free resources
 * @param manager Rule manager to cleanup
 */
void rule_manager_cleanup(rule_manager_t *manager);

/**
 * @brief Mark rule manager as needing re-evaluation
 * @param manager Rule manager to mark dirty
 */
void rule_manager_mark_dirty(rule_manager_t *manager);

// --- Tile-Based Rule Creation ---

/**
 * @brief Add rules when a tile is placed on the board
 * @param manager Rule manager
 * @param tile Tile that was placed
 * @param board Current board state
 */
void rule_manager_on_tile_placed(rule_manager_t *manager, 
                                const tile_t *tile, 
                                const board_t *board);

/**
 * @brief Remove rules when a tile is removed from the board
 * @param manager Rule manager
 * @param tile Tile that was removed
 */
void rule_manager_on_tile_removed(rule_manager_t *manager, const tile_t *tile);

/**
 * @brief Add rules when a pool is formed or modified
 * @param manager Rule manager
 * @param pool Pool that was created/modified
 * @param board Current board state
 */
void rule_manager_on_pool_changed(rule_manager_t *manager, 
                                 const pool_t *pool, 
                                 const board_t *board);

/**
 * @brief Remove rules when a pool is destroyed
 * @param manager Rule manager
 * @param pool Pool that was destroyed
 */
void rule_manager_on_pool_destroyed(rule_manager_t *manager, const pool_t *pool);

// --- Production Calculation Integration ---

/**
 * @brief Calculate effective production for a tile with all rule effects
 * @param manager Rule manager
 * @param tile Tile to calculate production for
 * @param board Current board state
 * @return Effective production value after applying all rules
 */
float rule_manager_calculate_tile_production(rule_manager_t *manager,
                                           const tile_t *tile,
                                           const board_t *board);

/**
 * @brief Calculate effective modifier for a pool with all rule effects
 * @param manager Rule manager
 * @param pool Pool to calculate modifier for
 * @param board Current board state
 * @return Effective pool modifier after applying all rules
 */
float rule_manager_calculate_pool_modifier(rule_manager_t *manager,
                                         const pool_t *pool,
                                         const board_t *board);

/**
 * @brief Update all production values based on current rules
 * @param manager Rule manager
 * @param board Board to update production for
 */
void rule_manager_update_production(rule_manager_t *manager, board_t *board);

// --- Specific Rule Factories ---

/**
 * @brief Create standard tile-based rules based on tile type
 * @param manager Rule manager
 * @param tile Tile to create rules for
 * @param board Current board state
 */
void rule_manager_create_tile_rules(rule_manager_t *manager,
                                   const tile_t *tile,
                                   const board_t *board);

/**
 * @brief Create standard pool-based rules
 * @param manager Rule manager
 * @param pool Pool to create rules for
 * @param board Current board state
 */
void rule_manager_create_pool_rules(rule_manager_t *manager,
                                   const pool_t *pool,
                                   const board_t *board);

// --- Example Rule Definitions ---

/**
 * @brief Add a "neighbor bonus" rule for a tile
 * Example: "+1 production per same-color neighbor"
 * @param manager Rule manager
 * @param source_tile Tile creating the rule
 * @param bonus_per_neighbor Bonus amount per matching neighbor
 */
void rule_manager_add_neighbor_bonus_rule(rule_manager_t *manager,
                                         const tile_t *source_tile,
                                         float bonus_per_neighbor);

/**
 * @brief Add a "perception override" rule for a tile
 * Example: "All neighbors appear as red tiles"
 * @param manager Rule manager
 * @param source_tile Tile creating the rule
 * @param override_type Type to make neighbors appear as
 */
void rule_manager_add_perception_rule(rule_manager_t *manager,
                                     const tile_t *source_tile,
                                     tile_type_t override_type);

/**
 * @brief Add a "pool size bonus" rule
 * Example: "+20% production for pools of size 5+"
 * @param manager Rule manager
 * @param source_pool Pool creating the rule
 * @param min_size Minimum pool size for bonus
 * @param bonus_percentage Percentage bonus
 */
void rule_manager_add_pool_size_rule(rule_manager_t *manager,
                                    const pool_t *source_pool,
                                    int min_size,
                                    float bonus_percentage);

/**
 * @brief Add an instant modifier rule
 * Example: "Permanently increase production by 2"
 * @param manager Rule manager
 * @param target_cell Cell to apply modifier to
 * @param target What to modify
 * @param amount Amount to add
 */
void rule_manager_add_instant_modifier(rule_manager_t *manager,
                                      grid_cell_t target_cell,
                                      rule_target_t target,
                                      float amount);

// --- Query Functions ---

/**
 * @brief Get all active rules affecting a specific cell
 * @param manager Rule manager
 * @param cell Cell to query
 * @param out_rules Array of rule pointers (caller should not free)
 * @param out_count Number of rules found
 */
void rule_manager_get_rules_for_cell(const rule_manager_t *manager,
                                    grid_cell_t cell,
                                    rule_t ***out_rules,
                                    size_t *out_count);

/**
 * @brief Count total number of active rules
 * @param manager Rule manager
 * @return Number of active rules
 */
size_t rule_manager_get_rule_count(const rule_manager_t *manager);

/**
 * @brief Check if any rules are pending evaluation
 * @param manager Rule manager
 * @return true if rules need evaluation
 */
bool rule_manager_needs_evaluation(const rule_manager_t *manager);

// --- Debug Functions ---

/**
 * @brief Print all active rules for debugging
 * @param manager Rule manager
 */
void rule_manager_print_all_rules(const rule_manager_t *manager);

/**
 * @brief Print rules affecting a specific cell
 * @param manager Rule manager
 * @param cell Cell to query
 */
void rule_manager_print_rules_for_cell(const rule_manager_t *manager, 
                                      grid_cell_t cell);

/**
 * @brief Print performance statistics
 * @param manager Rule manager
 */
void rule_manager_print_stats(const rule_manager_t *manager);

/**
 * @brief Reset performance counters
 * @param manager Rule manager
 */
void rule_manager_reset_stats(rule_manager_t *manager);

#endif // RULE_MANAGER_H