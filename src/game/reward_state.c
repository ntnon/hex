/**************************************************************************/ /**
                                                                              * @file reward_state.c
                                                                              * @brief Implementation of reward state management for reward selection UI
                                                                              *****************************************************************************/

#include "game/reward_state.h"
#include "game/rule_manager.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// --- Constants ---

#define PHASE_TRANSITION_TIME 0.5f
#define MIN_HOVER_TIME 0.1f
#define SELECTION_ANIMATION_SPEED 3.0f
#define HOVER_ANIMATION_SPEED 5.0f

// --- Internal Helper Functions ---

static void reward_state_reset_animations(reward_state_t *state);
static void reward_state_update_phase_timing(reward_state_t *state,
                                             float delta_time);
static bool reward_state_is_valid_phase_transition(reward_state_phase_t from,
                                                   reward_state_phase_t to);
static void reward_state_on_phase_enter(reward_state_t *state,
                                        reward_state_phase_t phase);
static void reward_state_on_phase_exit(reward_state_t *state,
                                       reward_state_phase_t phase);

// --- Reward State Lifecycle ---

bool reward_state_init(reward_state_t *state, uint32_t seed) {
  if (!state) {
    return false;
  }

  memset(state, 0, sizeof(reward_state_t));

  // Initialize reward system
  if (!reward_system_init(&state->reward_system, seed)) {
    return false;
  }

  // Set default configuration
  state->current_phase = REWARD_PHASE_ENTERING;
  state->trigger = REWARD_TRIGGER_MANUAL;
  state->state_entered_time = 0;
  state->phase_entered_time = 0;
  state->phase_timeout = 0.0f;

  // UI and interaction defaults
  state->player_can_interact = false;
  state->allow_skip = false;
  state->auto_advance_phases = true;
  state->min_presentation_time = 2.0f;
  state->require_confirmation = true;
  state->show_skip_option = false;

  // Animation defaults
  state->animation.animations_enabled = true;
  for (int i = 0; i < MAX_REWARD_OPTIONS; i++) {
    state->animation.hover_animation[i] = 0.0f;
    state->animation.glow_intensity[i] = 0.0f;
    state->animation.card_reveal_delays[i] = i * 0.2f;
  }

  // Input state
  state->input.keyboard_selection = -1;
  state->input.keyboard_navigation_active = false;

  // Error handling
  state->generation_failed = false;
  state->error_message = NULL;
  state->retry_count = 0;

  return true;
}

void reward_state_cleanup(reward_state_t *state) {
  if (!state) {
    return;
  }

  reward_system_cleanup(&state->reward_system);
  memset(state, 0, sizeof(reward_state_t));
}

void reward_state_reset(reward_state_t *state) {
  if (!state) {
    return;
  }

  // Reset phase state
  state->current_phase = REWARD_PHASE_ENTERING;
  state->state_entered_time = 0;
  state->phase_entered_time = 0;

  // Reset selection
  reward_system_clear_selection(&state->reward_system);

  // Reset UI state
  reward_state_reset_animations(state);
  state->player_can_interact = false;

  // Reset input
  memset(&state->input, 0, sizeof(reward_input_state_t));
  state->input.keyboard_selection = -1;

  // Reset error state
  state->generation_failed = false;
  state->error_message = NULL;
  state->retry_count = 0;
}

// --- State Transitions ---

bool reward_state_enter(reward_state_t *state, game_t *game,
                        reward_trigger_t trigger) {
  if (!state || !game) {
    return false;
  }

  // Store context
  state->previous_game_state = game->state;
  state->trigger = trigger;
  state->state_entered_time = (uint32_t)time(NULL);
  state->preserve_hover_state = game->should_show_tile_info;
  if (state->preserve_hover_state) {
    state->preserved_hovered_cell = game->hovered_cell;
  }

  // Set game to reward state
  game->state = GAME_STATE_REWARD;

  // Start with entering phase
  reward_state_set_phase(state, REWARD_PHASE_ENTERING);

  return true;
}

bool reward_state_exit(reward_state_t *state, game_t *game,
                       bool apply_selection) {
  if (!state || !game) {
    return false;
  }

  // Apply selected reward if requested
  if (apply_selection) {
    if (!reward_state_apply_to_game(state, game)) {
      printf("Warning: Failed to apply selected reward to game\n");
    }
  }

  // Restore previous game state
  game->state = state->previous_game_state;

  // Restore hover state if preserved
  if (state->preserve_hover_state) {
    game->hovered_cell = state->preserved_hovered_cell;
    game->should_show_tile_info = true;
  }

  // Reset reward state for next use
  reward_state_reset(state);

  return true;
}

void reward_state_update(reward_state_t *state, game_t *game,
                         float delta_time) {
  if (!state || !game) {
    return;
  }

  // Update timing
  reward_state_update_phase_timing(state, delta_time);

  // Update animations
  reward_state_update_animations(state, delta_time);

  // Handle phase transitions
  switch (state->current_phase) {
  case REWARD_PHASE_ENTERING:
    if (state->animation.phase_time > PHASE_TRANSITION_TIME) {
      reward_state_advance_phase(state);
    }
    break;

  case REWARD_PHASE_GENERATING:
    if (!state->generation_failed) {
      // Generate reward options
      reward_generation_params_t params = reward_system_create_default_params(
        game->board, game->rule_manager, 1, 1);

      if (reward_system_generate_options(&state->reward_system, &params)) {
        reward_state_advance_phase(state);
      } else {
        state->generation_failed = true;
        state->error_message = "Failed to generate reward options";
        state->retry_count++;

        if (state->retry_count < 3) {
          // Retry generation
          state->generation_failed = false;
        }
      }
    }
    break;

  case REWARD_PHASE_PRESENTING:
    state->player_can_interact = true;
    // Wait for player interaction or timeout
    if (reward_state_phase_timeout(state)) {
      if (state->allow_skip) {
        reward_state_skip_selection(state);
      }
    }
    break;

  case REWARD_PHASE_SELECTED:
    // Auto-advance if confirmation not required
    if (!state->require_confirmation && state->auto_advance_phases) {
      if (state->animation.phase_time > 0.5f) {
        reward_state_advance_phase(state);
      }
    }
    break;

  case REWARD_PHASE_CONFIRMING:
    // Wait for confirmation
    break;

  case REWARD_PHASE_APPLYING:
    if (state->animation.phase_time > 1.0f) {
      reward_state_advance_phase(state);
    }
    break;

  case REWARD_PHASE_EXITING:
    if (state->animation.phase_time > PHASE_TRANSITION_TIME) {
      reward_state_exit(state, game, true);
    }
    break;

  default:
    break;
  }
}

// --- Phase Management ---

bool reward_state_advance_phase(reward_state_t *state) {
  if (!state) {
    return false;
  }

  reward_state_phase_t next_phase;

  switch (state->current_phase) {
  case REWARD_PHASE_ENTERING:
    next_phase = REWARD_PHASE_GENERATING;
    break;
  case REWARD_PHASE_GENERATING:
    next_phase =
      state->generation_failed ? REWARD_PHASE_EXITING : REWARD_PHASE_PRESENTING;
    break;
  case REWARD_PHASE_PRESENTING:
    if (reward_system_get_selected_option(&state->reward_system)) {
      next_phase = REWARD_PHASE_SELECTED;
    } else {
      next_phase = REWARD_PHASE_EXITING; // No selection made
    }
    break;
  case REWARD_PHASE_SELECTED:
    next_phase = state->require_confirmation ? REWARD_PHASE_CONFIRMING
                                             : REWARD_PHASE_APPLYING;
    break;
  case REWARD_PHASE_CONFIRMING:
    next_phase = REWARD_PHASE_APPLYING;
    break;
  case REWARD_PHASE_APPLYING:
    next_phase = REWARD_PHASE_EXITING;
    break;
  case REWARD_PHASE_EXITING:
    return false; // Already at final phase
  default:
    return false;
  }

  return reward_state_set_phase(state, next_phase);
}

bool reward_state_set_phase(reward_state_t *state, reward_state_phase_t phase) {
  if (!state || phase >= REWARD_PHASE_COUNT) {
    return false;
  }

  if (!reward_state_is_valid_phase_transition(state->current_phase, phase)) {
    return false;
  }

  reward_state_on_phase_exit(state, state->current_phase);

  state->current_phase = phase;
  state->phase_entered_time = (uint32_t)time(NULL);
  state->animation.phase_time = 0.0f;

  reward_state_on_phase_enter(state, phase);

  return true;
}

bool reward_state_phase_timeout(const reward_state_t *state) {
  if (!state || state->phase_timeout <= 0.0f) {
    return false;
  }

  return state->animation.phase_time >= state->phase_timeout;
}

float reward_state_get_phase_time_remaining(const reward_state_t *state) {
  if (!state || state->phase_timeout <= 0.0f) {
    return -1.0f; // No timeout
  }

  return state->phase_timeout - state->animation.phase_time;
}

// --- Input Handling ---

bool reward_state_handle_mouse_input(reward_state_t *state, float mouse_x,
                                     float mouse_y, bool mouse_clicked) {
  if (!state || !reward_state_can_interact(state)) {
    return false;
  }

  reward_state_update_hover(state, mouse_x, mouse_y);

  if (mouse_clicked) {
    // Check for option clicks
    for (uint8_t i = 0; i < MAX_REWARD_OPTIONS; i++) {
      if (state->input.mouse_over_options[i]) {
        reward_state_select_option(state, i);
        return true;
      }
    }

    // Check for confirm button click
    if (state->input.confirm_button_hovered) {
      reward_state_confirm_selection(state);
      return true;
    }

    // Check for skip button click
    if (state->input.skip_button_hovered && state->allow_skip) {
      reward_state_skip_selection(state);
      return true;
    }
  }

  return false;
}

bool reward_state_handle_keyboard_input(reward_state_t *state, int key,
                                        bool is_pressed) {
  if (!state || !reward_state_can_interact(state) || !is_pressed) {
    return false;
  }

  // Enable keyboard navigation
  state->input.keyboard_navigation_active = true;

  // Handle numeric keys for selection
  if (key >= '1' && key <= '3') {
    uint8_t option_index = key - '1';
    return reward_state_select_option(state, option_index);
  }

  // Handle arrow keys for navigation
  if (key == 262 || key == 263) { // Right/Left arrow keys
    int8_t current = state->input.keyboard_selection;
    if (key == 262) { // Right arrow
      current = (current + 1) % MAX_REWARD_OPTIONS;
    } else { // Left arrow
      current = (current - 1 + MAX_REWARD_OPTIONS) % MAX_REWARD_OPTIONS;
    }
    state->input.keyboard_selection = current;
    return true;
  }

  // Handle Enter for confirmation
  if (key == 257) { // Enter key
    if (state->input.keyboard_selection >= 0) {
      reward_state_select_option(state, state->input.keyboard_selection);
    }
    return reward_state_confirm_selection(state);
  }

  // Handle Escape for skip/cancel
  if (key == 256) { // Escape key
    if (state->allow_skip) {
      return reward_state_skip_selection(state);
    } else {
      reward_state_cancel_selection(state);
      return true;
    }
  }

  return false;
}

void reward_state_update_hover(reward_state_t *state, float mouse_x,
                               float mouse_y) {
  if (!state) {
    return;
  }

  // Note: In a real implementation, this would check mouse position against
  // actual UI element bounds. For now, we'll simulate hover detection.
  // This would need to be integrated with the actual UI layout system.

  // Clear all hover states
  for (uint8_t i = 0; i < MAX_REWARD_OPTIONS; i++) {
    state->input.mouse_over_options[i] = false;
  }
  state->input.confirm_button_hovered = false;
  state->input.skip_button_hovered = false;

  // Update last input time
  state->input.last_input_time = (uint32_t)time(NULL);
}

// --- Selection Management ---

bool reward_state_select_option(reward_state_t *state, uint8_t option_index) {
  if (!state || !reward_state_can_interact(state)) {
    return false;
  }

  if (!reward_system_select_option(&state->reward_system, option_index)) {
    return false;
  }

  // Trigger selection effects
  reward_state_trigger_selection_effect(state, option_index);

  // Auto-advance if not requiring confirmation
  if (state->auto_advance_phases &&
      state->current_phase == REWARD_PHASE_PRESENTING) {
    reward_state_advance_phase(state);
  }

  return true;
}

bool reward_state_confirm_selection(reward_state_t *state) {
  if (!state || !reward_state_can_interact(state)) {
    return false;
  }

  if (!reward_system_get_selected_option(&state->reward_system)) {
    return false; // No selection to confirm
  }

  // Confirm selection in reward system
  rule_t selected_rule;
  if (!reward_system_confirm_selection(&state->reward_system, &selected_rule)) {
    return false;
  }

  // Advance phase if in confirming phase
  if (state->current_phase == REWARD_PHASE_CONFIRMING) {
    reward_state_advance_phase(state);
  }

  return true;
}

void reward_state_cancel_selection(reward_state_t *state) {
  if (!state) {
    return;
  }

  reward_system_clear_selection(&state->reward_system);

  // Return to presenting phase if possible
  if (state->current_phase == REWARD_PHASE_SELECTED ||
      state->current_phase == REWARD_PHASE_CONFIRMING) {
    reward_state_set_phase(state, REWARD_PHASE_PRESENTING);
  }
}

bool reward_state_skip_selection(reward_state_t *state) {
  if (!state || !state->allow_skip) {
    return false;
  }

  // Clear any selection
  reward_system_clear_selection(&state->reward_system);

  // Advance to exit phase
  reward_state_set_phase(state, REWARD_PHASE_EXITING);

  return true;
}

// --- Information Access ---

bool reward_state_is_active(const reward_state_t *state) {
  if (!state) {
    return false;
  }

  return state->current_phase != REWARD_PHASE_EXITING;
}

bool reward_state_can_interact(const reward_state_t *state) {
  if (!state) {
    return false;
  }

  return state->player_can_interact &&
         (state->current_phase == REWARD_PHASE_PRESENTING ||
          state->current_phase == REWARD_PHASE_SELECTED ||
          state->current_phase == REWARD_PHASE_CONFIRMING);
}

reward_state_phase_t
reward_state_get_current_phase(const reward_state_t *state) {
  if (!state) {
    return REWARD_PHASE_ENTERING;
  }

  return state->current_phase;
}

float reward_state_get_phase_progress(const reward_state_t *state) {
  if (!state || state->phase_timeout <= 0.0f) {
    return 0.0f;
  }

  return fminf(state->animation.phase_time / state->phase_timeout, 1.0f);
}

bool reward_state_get_options(const reward_state_t *state,
                              const reward_option_t **out_options,
                              uint8_t *out_count) {
  if (!state || !out_options || !out_count) {
    return false;
  }

  reward_option_t *options;
  return reward_system_get_options(&state->reward_system, &options,
                                   out_count) &&
         (*out_options = (const reward_option_t *)options, true);
}

int8_t reward_state_get_selected_index(const reward_state_t *state) {
  if (!state) {
    return -1;
  }

  const reward_option_t *selected =
    reward_system_get_selected_option(&state->reward_system);
  if (!selected) {
    return -1;
  }

  // Find index of selected option
  reward_option_t *options;
  uint8_t count;
  if (reward_system_get_options(&state->reward_system, &options, &count)) {
    for (uint8_t i = 0; i < count; i++) {
      if (&options[i] == selected) {
        return i;
      }
    }
  }

  return -1;
}

// --- Animation and Effects ---

void reward_state_update_animations(reward_state_t *state, float delta_time) {
  if (!state || !state->animation.animations_enabled) {
    return;
  }

  // Update hover animations
  for (uint8_t i = 0; i < MAX_REWARD_OPTIONS; i++) {
    float target = (state->input.mouse_over_options[i] ||
                    state->input.keyboard_selection == i)
                     ? 1.0f
                     : 0.0f;

    float speed = HOVER_ANIMATION_SPEED * delta_time;
    if (state->animation.hover_animation[i] < target) {
      state->animation.hover_animation[i] =
        fminf(state->animation.hover_animation[i] + speed, target);
    } else if (state->animation.hover_animation[i] > target) {
      state->animation.hover_animation[i] =
        fmaxf(state->animation.hover_animation[i] - speed, target);
    }

    // Update glow based on selection
    const reward_option_t *selected =
      reward_system_get_selected_option(&state->reward_system);
    const reward_option_t *option =
      reward_system_get_option(&state->reward_system, i);

    float glow_target = (option && option == selected) ? 1.0f : 0.0f;
    float glow_speed = SELECTION_ANIMATION_SPEED * delta_time;

    if (state->animation.glow_intensity[i] < glow_target) {
      state->animation.glow_intensity[i] =
        fminf(state->animation.glow_intensity[i] + glow_speed, glow_target);
    } else if (state->animation.glow_intensity[i] > glow_target) {
      state->animation.glow_intensity[i] =
        fmaxf(state->animation.glow_intensity[i] - glow_speed, glow_target);
    }
  }

  // Update selection animation
  float selection_target =
    reward_system_get_selected_option(&state->reward_system) ? 1.0f : 0.0f;
  float selection_speed = SELECTION_ANIMATION_SPEED * delta_time;

  if (state->animation.selection_animation < selection_target) {
    state->animation.selection_animation = fminf(
      state->animation.selection_animation + selection_speed, selection_target);
  } else if (state->animation.selection_animation > selection_target) {
    state->animation.selection_animation = fmaxf(
      state->animation.selection_animation - selection_speed, selection_target);
  }

  // Update particle effects
  state->animation.particle_time += delta_time;
}

void reward_state_trigger_selection_effect(reward_state_t *state,
                                           uint8_t option_index) {
  if (!state || option_index >= MAX_REWARD_OPTIONS) {
    return;
  }

  // Trigger visual effects for selection
  state->animation.glow_intensity[option_index] = 1.0f;
  state->animation.shake_effect = true;
  state->animation.particle_time = 0.0f;
}

float reward_state_get_hover_animation(const reward_state_t *state,
                                       uint8_t option_index) {
  if (!state || option_index >= MAX_REWARD_OPTIONS) {
    return 0.0f;
  }

  return state->animation.hover_animation[option_index];
}

float reward_state_get_selection_animation(const reward_state_t *state) {
  if (!state) {
    return 0.0f;
  }

  return state->animation.selection_animation;
}

// --- Configuration ---

void reward_state_configure(reward_state_t *state, bool auto_advance,
                            float min_presentation_time,
                            bool require_confirmation, bool allow_skip) {
  if (!state) {
    return;
  }

  state->auto_advance_phases = auto_advance;
  state->min_presentation_time = min_presentation_time;
  state->require_confirmation = require_confirmation;
  state->allow_skip = allow_skip;
  state->show_skip_option = allow_skip;
}

void reward_state_set_animations_enabled(reward_state_t *state, bool enabled) {
  if (!state) {
    return;
  }

  state->animation.animations_enabled = enabled;

  if (!enabled) {
    reward_state_reset_animations(state);
  }
}

// --- Integration with Game ---

bool reward_state_apply_to_game(reward_state_t *state, game_t *game) {
  if (!state || !game || !game->rule_manager) {
    return false;
  }

  const reward_option_t *selected =
    reward_system_get_selected_option(&state->reward_system);
  if (!selected) {
    return false; // No selection to apply
  }

  // Add the rule to the game's rule manager
  uint32_t rule_id = rule_manager_add_rule(game->rule_manager, &selected->rule);
  if (rule_id == 0) {
    return false; // Failed to add rule
  }

  printf("Applied reward: %s (Rule ID: %u)\n", selected->title, rule_id);
  return true;
}

bool reward_state_should_trigger_reward(const game_t *game,
                                        reward_trigger_t trigger_type) {
  if (!game) {
    return false;
  }

  // Simple trigger logic - can be expanded based on game conditions
  switch (trigger_type) {
  case REWARD_TRIGGER_TURN_END:
    return true; // Always offer rewards at turn end for now
  case REWARD_TRIGGER_MILESTONE:
    // Check for milestone conditions
    return false; // Not implemented yet
  case REWARD_TRIGGER_POOL_COMPLETE:
    // Check for completed pools
    return false; // Not implemented yet
  case REWARD_TRIGGER_SPECIAL_EVENT:
    return false; // Not implemented yet
  case REWARD_TRIGGER_MANUAL:
    return true; // Always allow manual triggers
  default:
    return false;
  }
}

const char *reward_state_get_trigger_name(reward_trigger_t trigger) {
  static const char *trigger_names[] = {
    "Turn End", "Milestone", "Pool Complete", "Special Event", "Manual"};

  if (trigger >= REWARD_TRIGGER_COUNT) {
    return "Unknown";
  }

  return trigger_names[trigger];
}

const char *reward_state_get_phase_name(reward_state_phase_t phase) {
  static const char *phase_names[] = {"Entering", "Generating", "Presenting",
                                      "Selected", "Confirming", "Applying",
                                      "Exiting"};

  if (phase >= REWARD_PHASE_COUNT) {
    return "Unknown";
  }

  return phase_names[phase];
}

// --- Debugging ---

void reward_state_debug_print(const reward_state_t *state) {
  if (!state) {
    printf("Reward state: NULL\n");
    return;
  }

  printf("=== Reward State Debug ===\n");
  printf("Current phase: %s\n",
         reward_state_get_phase_name(state->current_phase));
  printf("Trigger: %s\n", reward_state_get_trigger_name(state->trigger));
  printf("Phase time: %.2f\n", state->animation.phase_time);
  printf("Can interact: %s\n", reward_state_can_interact(state) ? "Yes" : "No");
  printf("Allow skip: %s\n", state->allow_skip ? "Yes" : "No");
  printf("Auto advance: %s\n", state->auto_advance_phases ? "Yes" : "No");
  printf("Require confirmation: %s\n",
         state->require_confirmation ? "Yes" : "No");

  if (state->generation_failed) {
    printf("Generation failed: %s\n",
           state->error_message ? state->error_message : "Unknown error");
    printf("Retry count: %u\n", state->retry_count);
  }

  printf("\nInput State:\n");
  printf("  Keyboard selection: %d\n", state->input.keyboard_selection);
  printf("  Keyboard navigation: %s\n",
         state->input.keyboard_navigation_active ? "Active" : "Inactive");

  printf("\nAnimation State:\n");
  printf("  Animations enabled: %s\n",
         state->animation.animations_enabled ? "Yes" : "No");
  printf("  Selection animation: %.2f\n", state->animation.selection_animation);
  for (uint8_t i = 0; i < MAX_REWARD_OPTIONS; i++) {
    printf("  Hover[%u]: %.2f, Glow[%u]: %.2f\n", i,
           state->animation.hover_animation[i], i,
           state->animation.glow_intensity[i]);
  }

  reward_system_debug_print(&state->reward_system);
}

bool reward_state_validate(const reward_state_t *state) {
  if (!state) {
    return false;
  }

  // Validate phase
  if (state->current_phase >= REWARD_PHASE_COUNT) {
    return false;
  }

  // Validate trigger
  if (state->trigger >= REWARD_TRIGGER_COUNT) {
    return false;
  }

  // Validate reward system
  if (!reward_system_validate(&state->reward_system)) {
    return false;
  }

  return true;
}

void reward_state_debug_force_phase(reward_state_t *state,
                                    reward_state_phase_t phase,
                                    bool skip_validation) {
  if (!state) {
    return;
  }

  if (!skip_validation && phase >= REWARD_PHASE_COUNT) {
    return;
  }

  state->current_phase = phase;
  state->phase_entered_time = (uint32_t)time(NULL);
  state->animation.phase_time = 0.0f;

  reward_state_on_phase_enter(state, phase);
}

// --- Internal Helper Functions ---

static void reward_state_reset_animations(reward_state_t *state) {
  if (!state) {
    return;
  }

  for (uint8_t i = 0; i < MAX_REWARD_OPTIONS; i++) {
    state->animation.hover_animation[i] = 0.0f;
    state->animation.glow_intensity[i] = 0.0f;
  }

  state->animation.selection_animation = 0.0f;
  state->animation.shake_effect = false;
  state->animation.particle_time = 0.0f;
  state->animation.phase_time = 0.0f;
  state->animation.transition_progress = 0.0f;
}

static void reward_state_update_phase_timing(reward_state_t *state,
                                             float delta_time) {
  if (!state) {
    return;
  }

  state->animation.phase_time += delta_time;
}

static bool reward_state_is_valid_phase_transition(reward_state_phase_t from,
                                                   reward_state_phase_t to) {
  // Define valid phase transitions
  switch (from) {
  case REWARD_PHASE_ENTERING:
    return to == REWARD_PHASE_GENERATING;
  case REWARD_PHASE_GENERATING:
    return to == REWARD_PHASE_PRESENTING || to == REWARD_PHASE_EXITING;
  case REWARD_PHASE_PRESENTING:
    return to == REWARD_PHASE_SELECTED || to == REWARD_PHASE_EXITING;
  case REWARD_PHASE_SELECTED:
    return to == REWARD_PHASE_CONFIRMING || to == REWARD_PHASE_APPLYING ||
           to == REWARD_PHASE_PRESENTING;
  case REWARD_PHASE_CONFIRMING:
    return to == REWARD_PHASE_APPLYING || to == REWARD_PHASE_PRESENTING;
  case REWARD_PHASE_APPLYING:
    return to == REWARD_PHASE_EXITING;
  case REWARD_PHASE_EXITING:
    return false; // Cannot transition from exiting
  default:
    return false;
  }
}

static void reward_state_on_phase_enter(reward_state_t *state,
                                        reward_state_phase_t phase) {
  if (!state) {
    return;
  }

  switch (phase) {
  case REWARD_PHASE_ENTERING:
    state->player_can_interact = false;
    break;
  case REWARD_PHASE_GENERATING:
    state->player_can_interact = false;
    break;
  case REWARD_PHASE_PRESENTING:
    state->player_can_interact = true;
    state->phase_timeout = state->min_presentation_time;
    break;
  case REWARD_PHASE_SELECTED:
    state->player_can_interact = state->require_confirmation;
    break;
  case REWARD_PHASE_CONFIRMING:
    state->player_can_interact = true;
    break;
  case REWARD_PHASE_APPLYING:
    state->player_can_interact = false;
    break;
  case REWARD_PHASE_EXITING:
    state->player_can_interact = false;
    break;
  default:
    break;
  }
}

static void reward_state_on_phase_exit(reward_state_t *state,
                                       reward_state_phase_t phase) {
  if (!state) {
    return;
  }

  // Cleanup any phase-specific state
  switch (phase) {
  case REWARD_PHASE_PRESENTING:
    state->phase_timeout = 0.0f;
    break;
  default:
    break;
  }
}
