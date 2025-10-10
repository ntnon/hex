#include "controller/game_controller.h"
#include "controller/input_state.h"
#include "ui.h"
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
  if (!controller->is_initialized) {
    printf("Warning: Controller not initialized\n");
    return;
  }

  // Store input state
  controller->input = *input;

  // Check for inventory item clicks
  if (controller->game && controller->game->inventory) {
    int inventory_size = inventory_get_size(controller->game->inventory);
    for (int i = 0; i < inventory_size; i++) {
      inventory_item_t item =
        inventory_get_item(controller->game->inventory, i);
      if (ui_was_clicked(item.id)) {
        controller->game->inventory->selected_index = i;
        printf("Selected inventory item at index %d\n", i);
        break;
      }
    }
  }

  // Get current game bounds
  Clay_BoundingBox game_bounds =
    Clay_GetElementData(UI_ID_GAME_AREA).boundingBox;

  // Set drag bounds to game area if hovering over game area
  if (controller->input.hovered_element_id.id == UI_ID_GAME_AREA.id) {
    controller->input.drag_bounds = game_bounds;
    printf("YESSIER");
  }

  printf("hovered_element_id: %s\n",
         controller->input.hovered_element_id.stringId.chars);
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
  return controller->input.hovered_element_id;
}

void game_controller_set_hovered_element_id(game_controller_t *controller,
                                            Clay_ElementId elementId) {
  controller->event_router.hovered_element_id = elementId;
}
