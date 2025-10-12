#include "controller/game_controller.h"
#include "controller/input_state.h"
#include "game/board.h"
#include "game/inventory.h"
#include "ui.h"
#include <MacTypes.h>
#include <stdio.h>

void game_controller_init(game_controller_t *controller, game_t *game) {
  controller->game = game;
  controller->generation = 0;
  controller->is_initialized = true;

  // Initialize input handler for camera/keyboard
  input_handler_init(&controller->input_handler, game);

  // Initialize state flags
  controller->inventory_open = true; // Inventory visible by default
  controller->placing_tile = false;
  controller->camera_locked = false;

  controller->selected_inventory_index = -1;
  controller->held_piece = NULL;

  // Initialize UI element tracking
  controller->hovered_element_id = UI_ID_NONE;
  controller->last_clicked_element_id = UI_ID_NONE;
  controller->hovered_element_data = (Clay_ElementData){0};

  printf("Game controller initialized as orchestrator\n");
}

bool inventory_controller_update(game_controller_t *controller,
                                 const input_state_t *input) {
  if (!controller->game || !controller->game->inventory) {
    return false;
  }

  // Check if Add Inventory button was clicked
  if (ui_was_clicked(UI_BUTTON_ADD_INVENTORY_ITEM)) {
    // Call inventory's add function directly
    inventory_add_random_item(controller->game->inventory);
    printf("Added random item to inventory\n");
    return true;
  }

  // Check if any inventory item was clicked
  int inventory_size = inventory_get_size(controller->game->inventory);
  for (int i = 0; i < inventory_size; i++) {
    inventory_item_t item = inventory_get_item(controller->game->inventory, i);

    if (ui_was_clicked(item.id)) {
      // Clicked on inventory item
      int currently_selected = controller->game->inventory->selected_index;

      if (i == currently_selected) {
        // Clicking selected item - deselect it
        inventory_set_index(controller->game->inventory, -1);
        game_controller_exit_placement_mode(controller);
        printf("Deselected inventory item %d\n", i);
      } else {
        // Select new item
        inventory_set_index(controller->game->inventory, i);
        controller->selected_inventory_index = i;
        game_controller_enter_placement_mode(controller);
        printf("Selected inventory item %d for placement\n", i);
      }

      return true; // Consumed the click
    }
  }

  return false; // No inventory interaction
}

void game_controller_update(game_controller_t *controller,
                            const input_state_t *input) {
  if (!controller->is_initialized) {
    printf("Warning: Controller not initialized\n");
    return;
  }

  // Get game area bounds for input processing
  Clay_BoundingBox game_bounds =
    Clay_GetElementData(UI_ID_GAME_AREA).boundingBox;

  // if the inventory is hovered, process only inventory interactions
  if (ui_get_hovered_element().id == UI_ID_INVENTORY_AREA.id) {
    // Handle inventory interactions
    if (inventory_controller_update(controller, input)) {
      return; // Input consumed by inventory
    }
  }

  if (inventory_get_index(controller->game->inventory)) {
    if (game_controller_handle_placement_input(controller, input)) {
      return; // Input consumed by placement
    }
  };

  if (input->key_r_pressed && controller->placing_tile) {
    // Rotate held piece
    if (inventory_rotate_selected(controller->game->inventory, 1)) {
      printf("Rotated held piece\n");
    }
  }

  if (input->key_m_pressed) {
    // Cycle game state for debugging
    game_state_cycle(controller->game);
  }

  // 4. Default camera/board interaction
  input_handler_update(&controller->input_handler, input, game_bounds);

  // Update the game with processed input
  update_game(controller->game, input);
}

bool game_controller_handle_placement_input(game_controller_t *controller,
                                            const input_state_t *input) {
  if (!controller->placing_tile || !controller->held_piece) {
    return false;
  }

  // Check if clicked on game area
  if (ui_was_clicked(UI_ID_GAME_AREA)) {
    grid_cell_t target_position = controller->game->hovered_cell;

    // Get the selected board from inventory
    board_t *selected_board =
      inventory_get_selected_board(controller->game->inventory);
    if (!selected_board) {
      printf("No board selected from inventory\n");
      return false;
    }

    // Calculate placement position
    grid_cell_t source_center = board_get_center_position(selected_board);

    // Attempt to place tile using board's placement logic
    if (board_place_board_at_position(controller->game->board, selected_board,
                                      target_position, source_center)) {
      printf("Successfully placed tile at (%d, %d)\n", target_position.q,
             target_position.r);

      // Clear selection and exit placement mode
      inventory_set_index(controller->game->inventory, -1);
      game_controller_exit_placement_mode(controller);

      // Update game state if needed
      controller->game->state = GAME_STATE_VIEW;

      return true; // Consumed the click
    } else {
      printf("Cannot place tile at (%d, %d) - position blocked or invalid\n",
             target_position.q, target_position.r);
    }
  }

  // ESC key cancels placement
  if (input->key_escape_pressed) {
    inventory_set_index(controller->game->inventory, -1);
    game_controller_exit_placement_mode(controller);
    printf("Cancelled placement mode\n");
    return true;
  }

  return false; // Didn't consume input
}

bool game_controller_handle_board_input(game_controller_t *controller,
                                        const input_state_t *input) {
  // Handle direct board interactions when not in placement mode
  if (controller->placing_tile) {
    return false; // Let placement handler deal with board clicks
  }

  if (ui_was_clicked(UI_ID_GAME_AREA)) {
    // Could handle tile selection, collection, etc. here
    grid_cell_t clicked_position = controller->game->hovered_cell;

    // For now, just log the click
    printf("Board clicked at (%d, %d)\n", clicked_position.q,
           clicked_position.r);

    // Future: Check if there's a tile at this position for
    // collection/interaction tile_t *tile =
    // board_get_tile_at(controller->game->board, clicked_position); if (tile) {
    // ... }

    return true;
  }

  return false;
}

bool game_controller_handle_camera_input(game_controller_t *controller,
                                         const input_state_t *input) {
  if (controller->camera_locked) {
    return false;
  }

  // Camera input is handled by input_handler in the update function
  // This function is here for completeness and future expansion
  return false;
}

void game_controller_enter_placement_mode(game_controller_t *controller) {
  controller->placing_tile = true;
  controller->held_piece =
    inventory_get_selected_board(controller->game->inventory);
  controller->game->state = GAME_STATE_PLACE;
  printf("Entered placement mode\n");
}

void game_controller_exit_placement_mode(game_controller_t *controller) {
  controller->placing_tile = false;
  controller->held_piece = NULL;
  controller->selected_inventory_index = -1;
  controller->game->state = GAME_STATE_VIEW;
  printf("Exited placement mode\n");
}

void game_controller_toggle_inventory(game_controller_t *controller) {
  controller->inventory_open = !controller->inventory_open;
  printf("Inventory %s\n", controller->inventory_open ? "opened" : "closed");
}

Clay_ElementId
game_controller_get_hovered_element_id(game_controller_t *controller) {
  return controller->hovered_element_id;
}

void game_controller_set_hovered_element_id(game_controller_t *controller,
                                            Clay_ElementId elementId) {
  controller->hovered_element_id = elementId;
}
