#include "controller/game_controller.h"
#include "controller/input_state.h"
#include <stdio.h>

void game_controller_init(game_controller_t *controller, game_t *game) {
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

void game_controller_update(game_controller_t *controller,
                            input_state_t *input) {
  printf("Game Controller Update\n");
  if (!controller->is_initialized) {
    printf("Warning: Controller not initialized\n");
    return;
  }

  // Store input state
  controller->input = *input;

  // Sync hovered element ID from event router to input state for camera
  // controls
  controller->input.hovered_element_id =
    controller->event_router.hovered_element_id;
  printf("controller.input.hovered_element_id.id %s",
         controller->input.hovered_element_id.stringId.chars);
  // Get current game bounds
  Clay_BoundingBox game_bounds =
    Clay_GetElementData(UI_ID_GAME_AREA).boundingBox;

  // Set drag bounds to game area if hovering over game area
  if (controller->input.hovered_element_id.id == UI_ID_GAME_AREA.id) {
    controller->input.drag_bounds = game_bounds;
  }

  // Process input through input handler first (including camera updates)
  input_handler_update(&controller->input_handler, &controller->input,
                       game_bounds);

  // Then update the game with the new camera position
  update_game(controller->game, &controller->input);
}

void game_controller_process_events(game_controller_t *controller) {
  if (!controller->is_initialized) {
    printf("Warning: Controller not initialized\n");
    return;
  }

  // Delegate all event processing to the event router
  event_router_process_events(&controller->event_router);
}

void game_controller_hover(game_controller_t *controller,
                           Clay_ElementId elementId) {
  controller->event_router.hovered_element_id = elementId;
}

void game_controller_set_hover(game_controller_t *controller, ui_event_t evt) {
  controller->input.hovered_element_id = evt.element_id;
  controller->input.drag_bounds = evt.element_data.boundingBox;
  controller->hovered_element_data = evt.element_data;
}

void game_controller_clear_hover(game_controller_t *controller,
                                 ui_event_t evt) {
  if (controller->event_router.hovered_element_id.id == evt.element_id.id) {
    controller->event_router.hovered_element_id = UI_ID_NONE;
  }
  controller->hovered_element_data = (Clay_ElementData){0};
}

/* Legacy accessors for backward compatibility */
Clay_ElementId
game_controller_get_last_clicked_element(game_controller_t *controller) {
  return controller->event_router.last_clicked_element_id;
}

Clay_ElementId
game_controller_get_hovered_element(game_controller_t *controller) {
  return controller->event_router.hovered_element_id;
}

void game_controller_add_game_bounds(game_controller_t *controller,
                                     Clay_BoundingBox bounds) {
  controller->input_handler.game_bounds = bounds;
}

/* Direct hovered element access for UI events */
Clay_ElementId
game_controller_get_hovered_element_id(game_controller_t *controller) {
  return controller->event_router.hovered_element_id;
}

void game_controller_set_hovered_element_id(game_controller_t *controller,
                                            Clay_ElementId elementId) {
  controller->event_router.hovered_element_id = elementId;
}
