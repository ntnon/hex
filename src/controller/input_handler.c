#include "controller/input_handler.h"
#include "game/camera.h"
#include "game/game.h"
#include "game/inventory.h"
#include "game/reward_state.h"
#include "ui_types.h"
#include "utility/geometry.h"
#include <stdio.h>

void input_handler_init(input_handler_t *handler, game_t *game) {
  handler->game = game;
  handler->game_bounds = (Clay_BoundingBox){0};
}

void input_handler_update(input_handler_t *handler, input_state_t *input,
                          Clay_BoundingBox game_bounds) {
  handler->game_bounds = game_bounds;

  input_handler_process_keyboard(handler, input);
  input_handler_process_camera(handler, input);
}

void input_handler_process_keyboard(input_handler_t *handler,
                                    input_state_t *input) {
  // If in reward state, handle reward input
  if (game_is_in_reward_state(handler->game)) {
    game_handle_reward_input(handler->game, input);
    return; // Don't process other keyboard input while in reward state
  }

  // Handle rotation of currently selected/held inventory item
  if (input->key_r_pressed) {
    if (inventory_rotate_selected(handler->game->inventory, 1)) {
      printf("Rotated inventory item clockwise\n");
    }
  }

  // Handle state cycling
  if (input->key_m_pressed) {
    printf("Cycling game state\n");
    game_state_cycle(handler->game);
  }

  // Handle reward trigger (for testing/debugging)
  if (input->key_space_pressed) {
    if (game_trigger_reward_selection(handler->game, REWARD_TRIGGER_MANUAL)) {
      printf("Triggered reward selection\n");
    } else {
      printf("Failed to trigger reward selection\n");
    }
  }
}

void input_handler_process_camera(input_handler_t *handler,
                                  input_state_t *input) {
  // Only update camera if mouse is within game bounds and hovering game area
  if (point_in_bounds(input->mouse, handler->game_bounds) &&
      input->hovered_element_id.id == UI_ID_GAME.id) {
    update_camera(&handler->game->board->camera, input);
  }
}
