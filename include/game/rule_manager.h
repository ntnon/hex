/**************************************************************************//**
 * @file rule_manager.h
 * @brief High-level rule management integration for hexhex game with player-driven rules
 *****************************************************************************/

#ifndef RULE_MANAGER_H
#define RULE_MANAGER_H

#include "rule_system.h"
#include "player_rule_manager.h"
#include "game/board.h"
#include "tile/tile.h"

// Forward declarations
typedef struct game game_t;

/**
 * @brief High-level rule manager that integrates player choices with game systems
 */
typedef struct rule_manager {
    rule_registry_t active_rules;      // Currently active rules affecting gameplay
    rule_catalog_t rule_catalog;       // All possible rules and player ownership
    rule_context_t evaluation_context; // Context for rule evaluation
    
    // Player rule management
    player_rule_context_t *player_contexts; // Per-player rule management contexts
    size_t player_count;                     // Number of players
    
    // Game integration
    const board_t *board;              // Board this manager operates on
    bool initialized;                  // Manager initialization state
    bool rules_dirty;                  // True if rules need re-evaluation
    
    // Performance tracking
    uint32_t evaluations_this_turn;
    uint32_t total_evaluations;
    uint32_t rules_applied_this_turn;
} rule_manager_t;

/**
 * @brief Rule evaluation timing for game phases
 */
typedef enum {
    RULE_TIMING_TILE_PLACED,          // After a tile is placed
    RULE_TIMING_TILE_REMOVED,         // After a tile is removed  
    RULE_TIMING_POOL_FORMED,          // After a pool is created/merged
    RULE_TIMING_POOL_DESTROYED,       // After a pool is destroyed
    RULE_TIMING_TURN_START,           // At the start of a player's turn
    RULE_TIMING_TURN_END,             // At the end of a player's turn
    RULE_TIMING_PRODUCTION_CALC,      // During production calculation
    RULE_TIMING_MANUAL                // Manually triggered evaluation
} rule_timing_t;

// --- Rule Manager Lifecycle ---

/**
 * @brief Initialize rule manager for a game
 * @param manager Rule manager to initialize
 * @param board Game board to manage rules for
 * @param player_count Number of players in the game
 * @return true on success, false on failure
 */
bool rule_manager_init(rule_manager_t *manager, const board_t *board, size_t player_count);

/**
 * @brief Cleanup rule manager and free resources
 * @param manager Rule manager to cleanup
 */
void rule_manager_cleanup(rule_manager_t *manager);

/**
 * @brief Load default rule catalog for the game
 * @param manager Rule manager
 * @return Number of rules loaded into catalog
 */
size_t rule_manager_load_default_catalog(rule_manager_t *manager);

/**
 * @brief Reset all player rules (for new game)
 * @param manager Rule manager
 */
void rule_manager_reset_player_rules(rule_manager_t *manager);

// --- Player Rule Interface ---

/**
 * @brief Get available rules for a player
 * @param manager Rule manager
 * @param player_id Player to query
 * @param out_rules Array to store available rules
 * @param max_rules Maximum rules to return
 * @return Number of rules returned
 */
size_t rule_manager_get_available_rules(const rule_manager_t *manager,
                                       uint32_t player_id,
                                       const player_rule_t **out_rules,
                                       size_t max_rules);

/**
 * @brief Get active rules for a player
 * @param manager Rule manager
 * @param player_id Player to query
 * @param out_rules Array to store active rules
 * @param max_rules Maximum rules to return
 * @return Number of rules returned
 */
size_t rule_manager_get_active_rules(const rule_manager_t *manager,
                                    uint32_t player_id,
                                    const player_rule_t **out_rules,
                                    size_t max_rules);

/**
 * @brief Attempt to acquire a rule for a player
 * @param manager Rule manager
 * @param player_id Player acquiring the rule
 * @param rule_id Rule to acquire
 * @return true if acquisition successful
 */
bool rule_manager_acquire_rule(rule_manager_t *manager, uint32_t player_id, uint32_t rule_id);

/**
 * @brief Attempt to remove a rule from a player
 * @param manager Rule manager
 * @param player_id Player removing the rule
 * @param rule_id Rule to remove
 * @return true if removal successful
 */
bool rule_manager_remove_rule(rule_manager_t *manager, uint32_t player_id, uint32_t rule_id);

/**
 * @brief Get rule recommendations for a player
 * @param manager Rule manager
 * @param player_id Player to get recommendations for
 * @param out_recommendations Array to store recommendations
 * @param max_recommendations Maximum recommendations to return
 * @return Number of recommendations returned
 */
size_t rule_manager_get_recommendations(const rule_manager_t *manager,
                                       uint32_t player_id,
                                       rule_recommendation_t *out_recommendations,
                                       size_t max_recommendations);

/**
 * @brief Check if player can afford and acquire a specific rule
 * @param manager Rule manager
 * @param player_id Player to check
 * @param rule_id Rule to check
 * @param out_reason String explaining why rule can't be acquired (if false)
 * @return true if rule can be acquired
 */
bool rule_manager_can_acquire_rule(const rule_manager_t *manager,
                                  uint32_t player_id,
                                  uint32_t rule_id,
                                  char *out_reason);

// --- Game Event Integration ---

/**
 * @brief Notify rule manager that a tile was placed
 * @param manager Rule manager
 * @param tile Tile that was placed
 * @param player_id Player who placed the tile
 */
void rule_manager_on_tile_placed(rule_manager_t *manager, const tile_t *tile, uint32_t player_id);

/**
 * @brief Notify rule manager that a tile was removed
 * @param manager Rule manager
 * @param cell Cell where tile was removed
 * @param player_id Player who removed the tile
 */
void rule_manager_on_tile_removed(rule_manager_t *manager, grid_cell_t cell, uint32_t player_id);

/**
 * @brief Notify rule manager that a pool was formed or changed
 * @param manager Rule manager
 * @param pool Pool that was formed/changed
 * @param player_id Player who owns the pool
 */
void rule_manager_on_pool_changed(rule_manager_t *manager, const pool_t *pool, uint32_t player_id);

/**
 * @brief Notify rule manager of turn start
 * @param manager Rule manager
 * @param player_id Player whose turn is starting
 */
void rule_manager_on_turn_start(rule_manager_t *manager, uint32_t player_id);

/**
 * @brief Notify rule manager of turn end
 * @param manager Rule manager
 * @param player_id Player whose turn is ending
 */
void rule_manager_on_turn_end(rule_manager_t *manager, uint32_t player_id);

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
float rule_manager_calculate_tile_production(const rule_manager_t *manager, const tile_t *tile);

/**
 * @brief Calculate effective range for a tile considering all active rules
 * @param manager Rule manager
 * @param tile Tile to calculate range for
 * @return Effective range value
 */
int rule_manager_calculate_tile_range(const rule_manager_t *manager, const tile_t *tile);

/**
 * @brief Calculate effective pool multiplier considering all active rules
 * @param manager Rule manager
 * @param pool Pool to calculate multiplier for
 * @return Effective multiplier value
 */
float rule_manager_calculate_pool_multiplier(const rule_manager_t *manager, const pool_t *pool);

/**
 * @brief Get perceived tile type (may be overridden by rules)
 * @param manager Rule manager
 * @param tile Tile to get perceived type for
 * @param observer_cell Cell from which tile is being observed
 * @return Perceived tile type
 */
tile_type_t rule_manager_get_perceived_type(const rule_manager_t *manager,
                                           const tile_t *tile,
                                           grid_cell_t observer_cell);

// --- Rule Creation and Management ---

/**
 * @brief Create and add a custom rule to the catalog
 * @param manager Rule manager
 * @param rule Rule to add to catalog
 * @return true if rule was added successfully
 */
bool rule_manager_add_catalog_rule(rule_manager_t *manager, const player_rule_t *rule);

/**
 * @brief Remove a rule from the catalog entirely
 * @param manager Rule manager
 * @param rule_id Rule to remove from catalog
 * @return true if rule was removed
 */
bool rule_manager_remove_catalog_rule(rule_manager_t *manager, uint32_t rule_id);

/**
 * @brief Update rule availability for all players based on current game state
 * @param manager Rule manager
 */
void rule_manager_update_rule_availability(rule_manager_t *manager);

// --- Performance and Debugging ---

/**
 * @brief Get rule evaluation performance stats
 * @param manager Rule manager
 * @param out_active_rules Number of currently active rules
 * @param out_evaluations_this_turn Evaluations performed this turn
 * @param out_total_evaluations Total evaluations since init
 */
void rule_manager_get_performance_stats(const rule_manager_t *manager,
                                       size_t *out_active_rules,
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

/**
 * @brief Print player rule portfolio
 * @param manager Rule manager  
 * @param player_id Player to analyze
 */
void rule_manager_debug_print_player_rules(const rule_manager_t *manager, uint32_t player_id);

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
 * @brief Get total number of rules in catalog
 * @param manager Rule manager
 * @return Number of rules in catalog
 */
size_t rule_manager_get_catalog_size(const rule_manager_t *manager);

/**
 * @brief Get total rule points for a player
 * @param manager Rule manager
 * @param player_id Player to query
 * @return Current rule points for player
 */
int rule_manager_get_player_rule_points(const rule_manager_t *manager, uint32_t player_id);

/**
 * @brief Award rule points to a player
 * @param manager Rule manager
 * @param player_id Player to award points to
 * @param points Points to award
 */
void rule_manager_award_rule_points(rule_manager_t *manager, uint32_t player_id, int points);

#endif // RULE_MANAGER_H