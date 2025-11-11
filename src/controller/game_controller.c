#include "controller/game_controller.h"
#include "controller/input_state.h"
#include "game/board.h"
#include "game/inventory.h"
#include "grid/grid_geometry.h"
#include "raylib.h"
#include "ui.h"
#include <stdio.h>

void game_controller_init(game_controller_t *controller, game_t *game) {
    controller->game = game;
    controller->generation = 0;
    controller->is_initialized = true;

    // Initialize input handler for camera/keyboard
    input_handler_init(&controller->input_handler, game);

    // Initialize state management
    controller->state = GAME_STATE_VIEW;

    // Initialize state flags
    controller->inventory_open = true; // Inventory visible by default
    controller->placing_tile = false;
    controller->camera_locked = false;

    controller->selected_inventory_index = -1;
    controller->held_piece = NULL;

    // Initialize UI element tracking
    controller->hovered_element_id = ID_NONE;
    controller->last_clicked_element_id = ID_NONE;
    controller->hovered_element_data = (Clay_ElementData){0};

    printf("Game controller initialized as orchestrator\n");
}

void game_controller_update(game_controller_t *controller,
                            const input_state_t *input) {
    if (!controller->is_initialized || !input) {
        return;
    }

    // Update hover state for UI tooltip display
    game_controller_update_hover_state(controller, input);

    // Process inputs by priority
    if (controller->inventory_open &&
        game_controller_handle_inventory_input(controller, input)) {
        return;
    }

    if (controller->placing_tile &&
        game_controller_handle_placement_input(controller, input)) {
        return;
    }

    // Keyboard shortcuts
    if (input->key_r_pressed && controller->placing_tile) {
        inventory_rotate_selected(controller->game->inventory, 1);
    }

    if (input->key_m_pressed) {
        game_controller_cycle_state(controller);
    }

    // Camera and game updates
    Clay_BoundingBox game_bounds =
      Clay_GetElementData(ID_GAME_AREA).boundingBox;
    input_handler_update(&controller->input_handler, input, game_bounds);
    update_game(controller->game, input);

    // Update preview if placing
    if (controller->placing_tile) {
        game_update_preview_at_position(controller->game,
                                        controller->hovered_cell);
    }
}

void game_controller_update_hover_state(game_controller_t *controller,
                                        const input_state_t *input) {
    if (!controller->game || !controller->game->board) {
        controller->hovered_tile = NULL;
        controller->should_show_tile_info = false;
        return;
    }

    // Get world mouse position from camera
    Vector2 world_mouse =
      GetScreenToWorld2D((Vector2){input->mouse.x, input->mouse.y},
                         controller->game->board->camera);

    // Update hovered cell
    controller->hovered_cell = grid_geometry_pixel_to_cell(
      controller->game->board->geometry_type, &controller->game->board->layout,
      (point_t){world_mouse.x, world_mouse.y});

    // Get tile at position
    controller->hovered_tile =
      get_tile_at_cell(controller->game->board, controller->hovered_cell);

    // Show tooltip when hovering tile and not placing
    controller->should_show_tile_info =
      controller->hovered_tile && !controller->placing_tile &&
      (controller->state == GAME_STATE_VIEW ||
       controller->state == GAME_STATE_INSPECT);
}

bool game_controller_handle_inventory_input(game_controller_t *controller,
                                            const input_state_t *input) {
    if (!controller->game || !controller->game->inventory) {
        return false;
    }

    // Check if Add Inventory button was clicked
    if (ui_was_clicked(UI_BUTTON_ADD_INVENTORY_ITEM)) {
        // Delegate to game's business logic
        game_add_random_inventory_item(controller->game);
        return true;
    }

    // Check if any inventory item was clicked
    int inventory_size = inventory_get_size(controller->game->inventory);
    for (int i = 0; i < inventory_size; i++) {
        inventory_item_t item =
          inventory_get_item(controller->game->inventory, i);

        if (ui_was_clicked(item.id)) {
            // Delegate selection logic to game
            if (game_try_select_inventory_item(controller->game, i)) {
                // Update controller's placement state based on selection
                if (controller->game->inventory->selected_index >= 0) {
                    game_controller_enter_placement_mode(controller);
                } else {
                    game_controller_exit_placement_mode(controller);
                }
            }
            return true; // Consumed the click
        }
    }

    return false; // No inventory interaction
}

bool game_controller_handle_placement_input(game_controller_t *controller,
                                            const input_state_t *input) {
    if (!controller->placing_tile) {
        return false;
    }

    // Check if clicked on game area
    if (ui_was_clicked(ID_GAME_AREA)) {
        // Use controller's hovered cell instead of game's
        grid_cell_t target_position = controller->hovered_cell;

        // Delegate placement logic to game
        if (game_try_place_tile(controller->game, target_position)) {
            // Placement successful - update controller state
            game_controller_exit_placement_mode(controller);
            return true;
        }
        // Placement failed but we still consumed the click
        return true;
    }

    // ESC key cancels placement
    if (input->key_escape_pressed) {
        game_exit_placement_mode(controller->game);
        game_controller_exit_placement_mode(controller);
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

    if (ui_was_clicked(ID_GAME_AREA)) {
        // Could handle tile selection, collection, etc. here
        // Use controller's hovered cell instead of game's
        grid_cell_t clicked_position = controller->hovered_cell;

        // For now, just log the click
        printf("Board clicked at (%d, %d)\n", clicked_position.coord.hex.q,
               clicked_position.coord.hex.r);

        // Future: Check if there's a tile at this position for
        // collection/interaction
        // tile_t *tile = board_get_tile_at(controller->game->board,
        // clicked_position); if (tile) {
        //   ...
        // }

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
    controller->state = GAME_STATE_PLACE;
    printf("Controller: Entered placement mode\n");
}

void game_controller_exit_placement_mode(game_controller_t *controller) {
    controller->placing_tile = false;
    controller->held_piece = NULL;
    controller->selected_inventory_index = -1;
    controller->state = GAME_STATE_VIEW;
    printf("Controller: Exited placement mode\n");
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

void game_controller_set_state(game_controller_t *controller,
                               game_state_e new_state) {
    controller->state = new_state;
    printf("Game state changed to: %s\n",
           game_controller_state_to_string(new_state));
}

game_state_e game_controller_get_state(game_controller_t *controller) {
    return controller->state;
}

void game_controller_cycle_state(game_controller_t *controller) {
    controller->state = (controller->state + 1) % GAME_STATE_COUNT;
    printf("State cycled to: %s\n",
           game_controller_state_to_string(controller->state));
}

const char *game_controller_state_to_string(game_state_e state) {
    switch (state) {
    case GAME_STATE_VIEW:
        return "View";
    case GAME_STATE_PLACE:
        return "Place";
    case GAME_STATE_INSPECT:
        return "Inspect";
    case GAME_STATE_REWARD:
        return "Reward";
    case GAME_STATE_GAME_OVER:
        return "Game Over";
    case GAME_STATE_COUNT:
        return "Count";
    default:
        return "Unknown";
    }
}
