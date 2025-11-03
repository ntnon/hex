#include "controller/input_handler.h"
#include "game/camera.h"
#include "game/game.h"
#include "game/inventory.h"
#include "ui_types.h"
#include "utility/geometry.h"
#include <stdio.h>

void input_handler_init(input_handler_t *handler, game_t *game) {
  handler->game = game;
  handler->game_bounds = (Clay_BoundingBox){0};
}

void input_handler_update(input_handler_t *handler, const input_state_t *input,
                          Clay_BoundingBox game_bounds) {
  handler->game_bounds = game_bounds;

  input_handler_process_keyboard(handler, input);
  input_handler_process_camera(handler, input);
}

void input_handler_process_keyboard(input_handler_t *handler,
                                    const input_state_t *input) {

  // State cycling is now handled by game_controller
}

void input_handler_process_camera(input_handler_t *handler,
                                  const input_state_t *input) {
  // Only update camera if mouse is within game bounds and hovering game area
  if (point_in_bounds(input->mouse, handler->game_bounds) &&
      input->hovered_element_id.id == UI_ID_GAME_AREA.id) {
    update_camera(&handler->game->board->camera, input);
  }
}
