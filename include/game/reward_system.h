/**************************************************************************//**
 * @file reward_system.h
 * @brief Reward system for generating and managing rule-based rewards
 * 
 * This system generates rewards (which are rules presented to the user)
 * and manages the reward selection process. From the system perspective,
 * these are rules. From the user perspective, these are rewards.
 *****************************************************************************/

#ifndef REWARD_SYSTEM_H
#define REWARD_SYSTEM_H

#include <stdint.h>
#include <stdbool.h>
#include "rule_system.h"
#include "game/board.h"
#include "tile/tile.h"

// Forward declarations
typedef struct game game_t;
typedef struct rule_manager rule_manager_t;

/**
 * @brief Maximum number of reward options to present
 */
#define MAX_REWARD_OPTIONS 3
#define MAX_REWARD_POOLS 10

/**
 * @brief Reward rarity/power levels
 */
typedef enum {
    REWARD_RARITY_COMMON,      // Simple, low-impact rules
    REWARD_RARITY_UNCOMMON,    // Moderate impact rules
    REWARD_RARITY_RARE,        // High impact rules
    REWARD_RARITY_LEGENDARY,   // Game-changing rules
    REWARD_RARITY_COUNT
} reward_rarity_t;

/**
 * @brief Reward categories for balanced generation
 */
typedef enum {
    REWARD_CATEGORY_PRODUCTION,  // Production bonuses
    REWARD_CATEGORY_RANGE,       // Range modifications
    REWARD_CATEGORY_SYNERGY,     // Tile type synergies
    REWARD_CATEGORY_GLOBAL,      // Global effects
    REWARD_CATEGORY_SPECIAL,     // Unique mechanics
    REWARD_CATEGORY_COUNT
} reward_category_t;

/**
 * @brief A reward option presented to the player
 */
typedef struct {
    rule_t rule;                    // The underlying rule
    reward_rarity_t rarity;         // Reward rarity level
    reward_category_t category;     // Reward category
    
    // Display information
    const char *title;              // Player-friendly title
    const char *description;        // Player-friendly description
    const char *flavor_text;        // Optional flavor text
    uint32_t display_priority;      // For sorting rewards
    
    // Selection state
    bool is_selected;               // Currently selected by UI
    bool is_available;              // Can be selected
    
    // Generation metadata
    uint32_t generation_seed;       // Seed used to generate this reward
    float power_score;              // Calculated power level
} reward_option_t;

/**
 * @brief Reward generation parameters
 */
typedef struct {
    reward_rarity_t min_rarity;     // Minimum rarity to generate
    reward_rarity_t max_rarity;     // Maximum rarity to generate
    reward_category_t preferred_categories[REWARD_CATEGORY_COUNT];
    uint8_t num_preferred_categories;
    
    // Context-based generation
    const board_t *board;           // Current board state
    const rule_manager_t *rule_manager; // Active rules for context
    uint32_t player_level;          // Player progress level
    uint32_t turn_number;           // Current turn
    
    // Randomization
    uint32_t seed;                  // Random seed
    bool ensure_variety;            // Avoid duplicate categories
    bool balance_power;             // Balance power levels
} reward_generation_params_t;

/**
 * @brief Reward selection state
 */
typedef struct {
    reward_option_t options[MAX_REWARD_OPTIONS];  // Available rewards
    uint8_t option_count;                         // Number of options
    int8_t selected_index;                        // Currently selected (-1 = none)
    bool selection_confirmed;                     // Player has confirmed selection
    
    // Generation state
    reward_generation_params_t generation_params;
    bool rewards_generated;                       // Options have been generated
    uint32_t generation_time;                     // When rewards were generated
    
    // Display state
    bool show_details;                            // Show detailed descriptions
    float selection_animation_time;               // For UI animations
} reward_selection_state_t;

/**
 * @brief Reward template for procedural generation
 */
typedef struct {
    reward_category_t category;
    reward_rarity_t rarity;
    const char *title_template;
    const char *description_template;
    const char *flavor_template;
    
    // Rule generation function pointer
    rule_t (*generate_rule)(const reward_generation_params_t *params, uint32_t seed);
    
    // Requirements
    uint32_t min_player_level;
    bool requires_specific_tiles;
    tile_type_t required_tile_types[TILE_TYPE_COUNT];
    
    // Balancing
    float base_power;
    float power_variance;
} reward_template_t;

/**
 * @brief Main reward system
 */
typedef struct reward_system {
    // Reward templates for generation
    const reward_template_t *templates;
    uint32_t template_count;
    
    // Current selection state
    reward_selection_state_t selection_state;
    
    // History and statistics
    uint32_t rewards_offered;
    uint32_t rewards_selected;
    reward_rarity_t last_rarities[10];  // Track recent rarities
    reward_category_t last_categories[10];  // Track recent categories
    
    // Configuration
    bool initialized;
    uint32_t random_seed;
} reward_system_t;

// --- Reward System Lifecycle ---

/**
 * @brief Initialize the reward system
 * @param system Reward system to initialize
 * @param seed Random seed for generation
 * @return true on success, false on failure
 */
bool reward_system_init(reward_system_t *system, uint32_t seed);

/**
 * @brief Cleanup reward system resources
 * @param system Reward system to cleanup
 */
void reward_system_cleanup(reward_system_t *system);

// --- Reward Generation ---

/**
 * @brief Generate reward options for the player to choose from
 * @param system Reward system
 * @param params Generation parameters
 * @return true if rewards were generated successfully
 */
bool reward_system_generate_options(reward_system_t *system, 
                                   const reward_generation_params_t *params);

/**
 * @brief Create default generation parameters
 * @param board Current board state
 * @param rule_manager Current rule manager
 * @param player_level Player progress level
 * @param turn_number Current turn
 * @return Default generation parameters
 */
reward_generation_params_t reward_system_create_default_params(const board_t *board,
                                                             const rule_manager_t *rule_manager,
                                                             uint32_t player_level,
                                                             uint32_t turn_number);

/**
 * @brief Force regenerate current reward options
 * @param system Reward system
 * @return true if rewards were regenerated successfully
 */
bool reward_system_regenerate_options(reward_system_t *system);

// --- Reward Selection ---

/**
 * @brief Select a reward option by index
 * @param system Reward system
 * @param option_index Index of reward to select (0-2)
 * @return true if selection was valid
 */
bool reward_system_select_option(reward_system_t *system, uint8_t option_index);

/**
 * @brief Confirm the current selection
 * @param system Reward system
 * @param out_selected_rule Pointer to store the selected rule
 * @return true if a rule was selected and confirmed
 */
bool reward_system_confirm_selection(reward_system_t *system, rule_t *out_selected_rule);

/**
 * @brief Clear current selection (deselect all)
 * @param system Reward system
 */
void reward_system_clear_selection(reward_system_t *system);

/**
 * @brief Check if a selection has been made and confirmed
 * @param system Reward system
 * @return true if player has confirmed a selection
 */
bool reward_system_has_confirmed_selection(const reward_system_t *system);

// --- Reward Information ---

/**
 * @brief Get current reward options
 * @param system Reward system
 * @param out_options Array to store reward options
 * @param out_count Pointer to store number of options
 * @return true if options are available
 */
bool reward_system_get_options(const reward_system_t *system, 
                              reward_option_t **out_options,
                              uint8_t *out_count);

/**
 * @brief Get currently selected option
 * @param system Reward system
 * @return Pointer to selected option, or NULL if none selected
 */
const reward_option_t *reward_system_get_selected_option(const reward_system_t *system);

/**
 * @brief Get reward option by index
 * @param system Reward system
 * @param option_index Index of reward option
 * @return Pointer to reward option, or NULL if invalid index
 */
const reward_option_t *reward_system_get_option(const reward_system_t *system, 
                                                uint8_t option_index);

// --- Reward Templates and Balance ---

/**
 * @brief Calculate power score for a reward
 * @param reward Reward option to evaluate
 * @param board Current board state for context
 * @return Power score (higher = more impactful)
 */
float reward_system_calculate_power_score(const reward_option_t *reward, 
                                         const board_t *board);

/**
 * @brief Get rarity display information
 * @param rarity Reward rarity
 * @param out_name Pointer to store rarity name
 * @param out_color Pointer to store rarity color
 */
void reward_system_get_rarity_info(reward_rarity_t rarity, 
                                  const char **out_name,
                                  uint32_t *out_color);

/**
 * @brief Get category display information
 * @param category Reward category
 * @param out_name Pointer to store category name
 * @param out_icon Pointer to store category icon
 */
void reward_system_get_category_info(reward_category_t category,
                                    const char **out_name,
                                    const char **out_icon);

// --- Reward History and Statistics ---

/**
 * @brief Record that a reward was selected
 * @param system Reward system
 * @param selected_reward The reward that was chosen
 */
void reward_system_record_selection(reward_system_t *system, 
                                   const reward_option_t *selected_reward);

/**
 * @brief Get reward selection statistics
 * @param system Reward system
 * @param out_total_offered Total rewards offered
 * @param out_total_selected Total rewards selected
 * @param out_selection_rate Selection rate (0.0 - 1.0)
 */
void reward_system_get_statistics(const reward_system_t *system,
                                 uint32_t *out_total_offered,
                                 uint32_t *out_total_selected,
                                 float *out_selection_rate);

/**
 * @brief Check if a category was recently offered
 * @param system Reward system
 * @param category Category to check
 * @param lookback_count Number of recent rewards to check
 * @return true if category was offered recently
 */
bool reward_system_was_category_recent(const reward_system_t *system,
                                      reward_category_t category,
                                      uint8_t lookback_count);

// --- Debugging and Development ---

/**
 * @brief Print reward system state for debugging
 * @param system Reward system
 */
void reward_system_debug_print(const reward_system_t *system);

/**
 * @brief Print detailed information about current reward options
 * @param system Reward system
 */
void reward_system_debug_print_options(const reward_system_t *system);

/**
 * @brief Validate reward system state
 * @param system Reward system
 * @return true if system state is valid
 */
bool reward_system_validate(const reward_system_t *system);

// --- Utility Functions ---

/**
 * @brief Convert a rule to a reward option with display information
 * @param rule Source rule
 * @param rarity Reward rarity
 * @param category Reward category
 * @return Reward option with generated display information
 */
reward_option_t reward_system_rule_to_reward(const rule_t *rule,
                                            reward_rarity_t rarity,
                                            reward_category_t category);

/**
 * @brief Generate a random reward of specified rarity and category
 * @param rarity Desired rarity
 * @param category Desired category
 * @param params Generation parameters
 * @param seed Random seed
 * @return Generated reward option
 */
reward_option_t reward_system_generate_specific_reward(reward_rarity_t rarity,
                                                      reward_category_t category,
                                                      const reward_generation_params_t *params,
                                                      uint32_t seed);

#endif // REWARD_SYSTEM_H