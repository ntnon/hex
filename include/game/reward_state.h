/**************************************************************************//**
 * @file reward_state.h
 * @brief Reward state management for reward selection UI
 *
 * This module manages the state when the player is selecting rewards.
 * It handles the transition into/out of reward selection and manages
 * the UI state during reward selection.
 *****************************************************************************/

#ifndef REWARD_STATE_H
#define REWARD_STATE_H

#include <stdint.h>
#include <stdbool.h>

// Forward declarations
typedef struct game game_t;
typedef struct game_controller game_controller_t;

#define MAX_REWARD_OPTIONS 10

/**
 * @brief Reward state transition triggers
 */
typedef enum {
    REWARD_TRIGGER_TURN_END,        // End of turn reward
    REWARD_TRIGGER_MILESTONE,       // Milestone achievement
    REWARD_TRIGGER_POOL_COMPLETE,   // Large pool completed
    REWARD_TRIGGER_SPECIAL_EVENT,   // Special game event
    REWARD_TRIGGER_MANUAL,          // Manually triggered (debug/testing)
    REWARD_TRIGGER_COUNT
} reward_trigger_t;

/**
 * @brief Reward state phases
 */
typedef enum {
    REWARD_PHASE_ENTERING,          // Transitioning into reward state
    REWARD_PHASE_GENERATING,        // Generating reward options
    REWARD_PHASE_PRESENTING,        // Showing options to player
    REWARD_PHASE_SELECTED,          // Player has selected an option
    REWARD_PHASE_CONFIRMING,        // Player is confirming selection
    REWARD_PHASE_APPLYING,          // Applying selected reward
    REWARD_PHASE_EXITING,           // Transitioning out of reward state
    REWARD_PHASE_COUNT
} reward_state_phase_t;

/**
 * @brief UI animation state for reward selection
 */
typedef struct {
    float phase_time;               // Time spent in current phase
    float transition_progress;      // 0.0 - 1.0 transition progress
    float hover_animation[MAX_REWARD_OPTIONS];  // Per-option hover animations
    float selection_animation;      // Selection confirmation animation
    bool animations_enabled;        // Whether to show animations

    // Visual effects
    float card_reveal_delays[MAX_REWARD_OPTIONS]; // Staggered card reveals
    float glow_intensity[MAX_REWARD_OPTIONS];     // Card glow effects
    bool shake_effect;              // Screen shake on selection
    float particle_time;            // Particle effect timing
} reward_ui_animation_t;

/**
 * @brief Input handling state for reward selection
 */
typedef struct {
    bool mouse_over_options[MAX_REWARD_OPTIONS];  // Mouse hover state
    bool option_clicked[MAX_REWARD_OPTIONS];      // Click state
    bool confirm_button_hovered;                  // Confirm button state
    bool confirm_button_clicked;                  // Confirm button clicked
    bool skip_button_hovered;                     // Skip button state (if allowed)
    bool skip_button_clicked;                     // Skip button clicked

    // Keyboard navigation
    int8_t keyboard_selection;                    // Keyboard-selected option (-1 = none)
    bool keyboard_navigation_active;              // Using keyboard navigation
    uint32_t last_input_time;                     // Last input timestamp

    // Touch/gamepad support
    bool touch_active;                            // Touch input detected
    float touch_positions[MAX_REWARD_OPTIONS][2]; // Touch positions for options
} reward_input_state_t;

/**
 * @brief Complete reward state management
 */
typedef struct reward_state {
    // Core state
    reward_state_phase_t current_phase;          // Current state phase
    reward_trigger_t trigger;                    // What triggered reward state
    reward_system_t reward_system;               // Reward generation and selection

    // Timing
    uint32_t state_entered_time;                 // When we entered reward state
    uint32_t phase_entered_time;                 // When we entered current phase
    float phase_timeout;                         // Max time for current phase (0 = no timeout)

    // UI and interaction
    reward_ui_animation_t animation;             // Animation state
    reward_input_state_t input;                  // Input handling state
    bool player_can_interact;                    // Player can interact with UI
    bool allow_skip;                             // Can player skip reward selection

    // Context preservation
    game_state_e previous_game_state;            // State to return to
    bool preserve_hover_state;                   // Keep tile hover info
    grid_cell_t preserved_hovered_cell;          // Preserved hover cell

    // Configuration
    bool auto_advance_phases;                    // Automatically advance through phases
    float min_presentation_time;                 // Minimum time to show options
    bool require_confirmation;                   // Require explicit confirmation
    bool show_skip_option;                       // Show skip button

    // Error handling
    bool generation_failed;                      // Reward generation failed
    const char *error_message;                   // Error message for display
    uint8_t retry_count;                         // Number of generation retries
} reward_state_t;

// --- Reward State Lifecycle ---

/**
 * @brief Initialize reward state system
 * @param state Reward state to initialize
 * @param seed Random seed for reward generation
 * @return true on success, false on failure
 */
bool reward_state_init(reward_state_t *state, uint32_t seed);

/**
 * @brief Cleanup reward state resources
 * @param state Reward state to cleanup
 */
void reward_state_cleanup(reward_state_t *state);

/**
 * @brief Reset reward state for new selection
 * @param state Reward state to reset
 */
void reward_state_reset(reward_state_t *state);

// --- State Transitions ---

/**
 * @brief Enter reward selection state
 * @param state Reward state
 * @param game Current game
 * @param trigger What triggered the reward state
 * @return true if transition was successful
 */
bool reward_state_enter(reward_state_t *state, game_t *game, reward_trigger_t trigger);

/**
 * @brief Exit reward selection state
 * @param state Reward state
 * @param game Game to return to
 * @param apply_selection Whether to apply the selected reward
 * @return true if transition was successful
 */
bool reward_state_exit(reward_state_t *state, game_t *game, bool apply_selection);

/**
 * @brief Update reward state (call each frame)
 * @param state Reward state
 * @param game Current game
 * @param delta_time Time since last update
 */
void reward_state_update(reward_state_t *state, game_t *game, float delta_time);

// --- Phase Management ---

/**
 * @brief Advance to next phase
 * @param state Reward state
 * @return true if phase transition was successful
 */
bool reward_state_advance_phase(reward_state_t *state);

/**
 * @brief Set specific phase
 * @param state Reward state
 * @param phase Phase to transition to
 * @return true if phase transition was valid
 */
bool reward_state_set_phase(reward_state_t *state, reward_state_phase_t phase);

/**
 * @brief Check if current phase has timed out
 * @param state Reward state
 * @return true if phase should timeout
 */
bool reward_state_phase_timeout(const reward_state_t *state);

/**
 * @brief Get time remaining in current phase
 * @param state Reward state
 * @return Time remaining in seconds (negative if no timeout)
 */
float reward_state_get_phase_time_remaining(const reward_state_t *state);

// --- Input Handling ---

/**
 * @brief Handle mouse input for reward selection
 * @param state Reward state
 * @param mouse_x Mouse X position
 * @param mouse_y Mouse Y position
 * @param mouse_clicked Whether mouse was clicked this frame
 * @return true if input was handled
 */
bool reward_state_handle_mouse_input(reward_state_t *state,
                                    float mouse_x, float mouse_y,
                                    bool mouse_clicked);

/**
 * @brief Handle keyboard input for reward selection
 * @param state Reward state
 * @param key Key that was pressed
 * @param is_pressed Whether key was pressed (vs released)
 * @return true if input was handled
 */
bool reward_state_handle_keyboard_input(reward_state_t *state,
                                       int key, bool is_pressed);

/**
 * @brief Update hover states based on mouse position
 * @param state Reward state
 * @param mouse_x Mouse X position
 * @param mouse_y Mouse Y position
 */
void reward_state_update_hover(reward_state_t *state, float mouse_x, float mouse_y);

// --- Selection Management ---

/**
 * @brief Select reward option by index
 * @param state Reward state
 * @param option_index Index of option to select
 * @return true if selection was valid
 */
bool reward_state_select_option(reward_state_t *state, uint8_t option_index);

/**
 * @brief Confirm current selection
 * @param state Reward state
 * @return true if confirmation was successful
 */
bool reward_state_confirm_selection(reward_state_t *state);

/**
 * @brief Cancel current selection
 * @param state Reward state
 */
void reward_state_cancel_selection(reward_state_t *state);

/**
 * @brief Skip reward selection (if allowed)
 * @param state Reward state
 * @return true if skip was allowed and processed
 */
bool reward_state_skip_selection(reward_state_t *state);

// --- Information Access ---

/**
 * @brief Check if reward state is active
 * @param state Reward state
 * @return true if in reward selection state
 */
bool reward_state_is_active(const reward_state_t *state);

/**
 * @brief Check if player can currently interact
 * @param state Reward state
 * @return true if player input should be accepted
 */
bool reward_state_can_interact(const reward_state_t *state);

/**
 * @brief Get current phase
 * @param state Reward state
 * @return Current state phase
 */
reward_state_phase_t reward_state_get_current_phase(const reward_state_t *state);

/**
 * @brief Get phase progress (0.0 - 1.0)
 * @param state Reward state
 * @return Progress through current phase
 */
float reward_state_get_phase_progress(const reward_state_t *state);

/**
 * @brief Get current reward options
 * @param state Reward state
 * @param out_options Pointer to store options array
 * @param out_count Pointer to store option count
 * @return true if options are available
 */
bool reward_state_get_options(const reward_state_t *state,
                             const reward_option_t **out_options,
                             uint8_t *out_count);

/**
 * @brief Get currently selected option index
 * @param state Reward state
 * @return Selected option index (-1 if none selected)
 */
int8_t reward_state_get_selected_index(const reward_state_t *state);

// --- Animation and Effects ---

/**
 * @brief Update animations for reward state
 * @param state Reward state
 * @param delta_time Time since last update
 */
void reward_state_update_animations(reward_state_t *state, float delta_time);

/**
 * @brief Trigger selection effect
 * @param state Reward state
 * @param option_index Index of selected option
 */
void reward_state_trigger_selection_effect(reward_state_t *state, uint8_t option_index);

/**
 * @brief Get hover animation value for option
 * @param state Reward state
 * @param option_index Option index
 * @return Animation value (0.0 - 1.0)
 */
float reward_state_get_hover_animation(const reward_state_t *state, uint8_t option_index);

/**
 * @brief Get selection animation value
 * @param state Reward state
 * @return Animation value (0.0 - 1.0)
 */
float reward_state_get_selection_animation(const reward_state_t *state);

// --- Configuration ---

/**
 * @brief Set reward state configuration
 * @param state Reward state
 * @param auto_advance Whether to auto-advance phases
 * @param min_presentation_time Minimum time to show options
 * @param require_confirmation Whether to require confirmation
 * @param allow_skip Whether to allow skipping
 */
void reward_state_configure(reward_state_t *state,
                           bool auto_advance,
                           float min_presentation_time,
                           bool require_confirmation,
                           bool allow_skip);

/**
 * @brief Enable/disable animations
 * @param state Reward state
 * @param enabled Whether animations are enabled
 */
void reward_state_set_animations_enabled(reward_state_t *state, bool enabled);

// --- Integration with Game ---

/**
 * @brief Apply selected reward to game
 * @param state Reward state
 * @param game Game to apply reward to
 * @return true if reward was applied successfully
 */
bool reward_state_apply_to_game(reward_state_t *state, game_t *game);

/**
 * @brief Check if reward should be triggered
 * @param game Current game state
 * @param trigger_type Type of trigger to check
 * @return true if reward should be offered
 */
bool reward_state_should_trigger_reward(const game_t *game, reward_trigger_t trigger_type);

/**
 * @brief Get trigger display name
 * @param trigger Trigger type
 * @return Human-readable trigger name
 */
const char *reward_state_get_trigger_name(reward_trigger_t trigger);

/**
 * @brief Get phase display name
 * @param phase State phase
 * @return Human-readable phase name
 */
const char *reward_state_get_phase_name(reward_state_phase_t phase);

// --- Debugging ---

/**
 * @brief Print reward state for debugging
 * @param state Reward state
 */
void reward_state_debug_print(const reward_state_t *state);

/**
 * @brief Validate reward state
 * @param state Reward state
 * @return true if state is valid
 */
bool reward_state_validate(const reward_state_t *state);

/**
 * @brief Force specific phase for testing
 * @param state Reward state
 * @param phase Phase to force
 * @param skip_validation Skip validation checks
 */
void reward_state_debug_force_phase(reward_state_t *state,
                                   reward_state_phase_t phase,
                                   bool skip_validation);

#endif // REWARD_STATE_H
