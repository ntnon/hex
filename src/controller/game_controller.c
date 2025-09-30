#include "controller/game_controller.h"
#include "controller/input_state.h"
#include "ui.h"
#include <stdio.h>

void controller_init(game_controller_t *controller, game_t *game) {
  controller->game = game;
  controller->generation = 0;
  controller->is_initialized = true;

  // Initialize input state
  input_state_init(&controller->input);

  // Initialize composed components
  input_handler_init(&controller->input_handler, game);
  event_router_init(&controller->event_router, game);
  game_actions_init(&controller->game_actions, game);

  // Initialize hovered element data
  controller->hovered_element_data = (Clay_ElementData){0};
}

void controller_update(game_controller_t *controller, input_state_t *input) {
  if (!controller->is_initialized) {
    printf("Warning: Controller not initialized\n");
    return;
  }

  // Store input state
  controller->input = *input;

  // Get current game bounds
  Clay_BoundingBox game_bounds = Clay_GetElementData(UI_ID_GAME).boundingBox;

  // Update the game itself
  update_game(controller->game, input);

  // Process input through input handler
  input_handler_update(&controller->input_handler, input, game_bounds);
}

void controller_process_events(game_controller_t *controller) {
  if (!controller->is_initialized) {
    printf("Warning: Controller not initialized\n");
    return;
  }

  // Delegate all event processing to the event router
  event_router_process_events(&controller->event_router);
}

void controller_hover(game_controller_t *controller, Clay_ElementId elementId) {
  controller->event_router.hovered_element_id = elementId;
}

void controller_set_hover(game_controller_t *controller, ui_event_t evt) {
  controller->input.hovered_element_id = evt.element_id;
  controller->input.drag_bounds = evt.element_data.boundingBox;
  controller->hovered_element_data = evt.element_data;
}

void controller_clear_hover(game_controller_t *controller, ui_event_t evt) {
  if (controller->event_router.hovered_element_id.id == evt.element_id.id) {
    controller->event_router.hovered_element_id = UI_ID_NONE;
  }
  controller->hovered_element_data = (Clay_ElementData){0};
}

/* Legacy accessors for backward compatibility */
Clay_ElementId
controller_get_last_clicked_element(game_controller_t *controller) {
  return controller->event_router.last_clicked_element_id;
}

Clay_ElementId controller_get_hovered_element(game_controller_t *controller) {
  return controller->event_router.hovered_element_id;
}

void controller_add_game_bounds(game_controller_t *controller,
                                Clay_BoundingBox bounds) {
  controller->input_handler.game_bounds = bounds;
}

/* Direct hovered element access for UI events */
Clay_ElementId
controller_get_hovered_element_id(game_controller_t *controller) {
  return controller->event_router.hovered_element_id;
}

void controller_set_hovered_element_id(game_controller_t *controller,
                                       Clay_ElementId elementId) {
  controller->event_router.hovered_element_id = elementId;
}
