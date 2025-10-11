#include "controller/game_controller.h"
#include "controller/input_state.h"
#include "game/game_actions.h"
#include "ui.h"
#include <stdio.h>

void game_controller_init(game_controller_t *controller, game_t *game) {
  controller->game = game;
  controller->generation = 0;
  controller->is_initialized = true;

  // Initialize composed components
  input_handler_init(&controller->input_handler, game);
  event_router_init(&controller->event_router, game);
  game_actions_init(&controller->game_actions, game);

  // Initialize hovered element data
  controller->hovered_element_data = (Clay_ElementData){0};
}

void game_controller_update(game_controller_t *controller,
                            const input_state_t *input) {
  if (!controller->is_initialized) {
    printf("Warning: Controller not initialized\n");
    return;
  }

  // Check for inventory item clicks
  if (controller->game && controller->game->inventory) {
    int inventory_size = inventory_get_size(controller->game->inventory);
    for (int i = 0; i < inventory_size; i++) {
      inventory_item_t item =
        inventory_get_item(controller->game->inventory, i);
      if (ui_was_clicked(item.id)) {
        // if the clicked item is already selected, deselect it.
        if (i == controller->game->inventory->selected_index) {
          controller->game->inventory->selected_index = -1;
        } else {
          controller->game->inventory->selected_index = i;
        }
        break;
      }
    }
  }

  // Check for game area clicks when in PLACE state
  if (controller->game && controller->game->state == GAME_STATE_PLACE) {
    if (ui_was_clicked(UI_ID_GAME_AREA)) {
      // Place the selected inventory item at the hovered position
      game_actions_t actions;
      game_actions_init(&actions, controller->game);
      if (game_actions_place_tile(&actions, controller->game->hovered_cell)) {
        // Successfully placed - deselect the inventory item
        controller->game->inventory->selected_index = -1;
        controller->game->state = GAME_STATE_VIEW;
      }
    }
  }

  // Check for game area clicks when in PLACE state
  if (controller->game && controller->game->state == GAME_STATE_PLACE) {
    if (ui_was_clicked(UI_ID_GAME_AREA)) {
      // Place the selected inventory item at the hovered position
      game_actions_t actions;
      game_actions_init(&actions, controller->game);
      if (game_actions_place_tile(&actions, controller->game->hovered_cell)) {
        // Successfully placed - deselect the inventory item
        controller->game->inventory->selected_index = -1;
        controller->game->state = GAME_STATE_VIEW;
      }
    }
  }

  // Get current game bounds
  Clay_BoundingBox game_bounds =
    Clay_GetElementData(UI_ID_GAME_AREA).boundingBox;

  // Note: drag bounds should be set in main loop or app controller
  // before passing input to game controller

  // printf("hovered_element_id: %s\n",
  //        controller->input.hovered_element_id.stringId.chars);
  //  Process input through input handler first (including camera updates)
  input_handler_update(&controller->input_handler, input, game_bounds);

  // Then update the game with the new camera position
  update_game(controller->game, input);
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

/* Direct hovered element access for UI events */
Clay_ElementId
game_controller_get_hovered_element_id(game_controller_t *controller) {
  // This should be retrieved from event router or passed input
  return controller->event_router.hovered_element_id;
}

void game_controller_set_hovered_element_id(game_controller_t *controller,
                                            Clay_ElementId elementId) {
  controller->event_router.hovered_element_id = elementId;
}
