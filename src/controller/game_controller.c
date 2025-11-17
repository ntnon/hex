#include "controller/game_controller.h"
#include "controller/input_state.h"
#include "game/board.h"
#include "game/camera.h"
#include "game/game.h"
#include "game/inventory.h"
#include "grid/grid_geometry.h"
#include "raylib.h"
#include "ui.h"
#include "ui_types.h"
#include "utility/geometry.h"
#include <stdio.h>

void game_controller_init(game_controller_t *controller, game_t *game) {
    controller->game = game;
    controller->is_initialized = true;

    // Initialize state management
    controller->state = GAME_STATE_VIEW;

    // Initialize state flags
    controller->inventory_open = true; // Inventory visible by default
    controller->placing_tile = false;
    controller->camera_locked = false;

    controller->hovered_cell =
      grid_geometry_get_origin(controller->game->board->geometry_type);
    controller->hovered_tile = NULL;
    controller->game_board_hovered = false;
}

void game_controller_update(game_controller_t *controller,
                            const input_state_t *input) {
    if (!controller->is_initialized || !input) {
        return;
    }

    // Update the game bounds, in case something changed.
    bounds_t game_bounds = ui_get_element_bounds(ID_GAME_AREA);

    // update camera
    update_camera(&controller->game->board->camera, input);

    // Update hover state (calculates hovered_cell and hovered_tile)
    game_controller_update_hover_state(controller, input);

    // Handle state-agnostic interactions
    if (input->key_m_pressed) {
        game_controller_cycle_state(controller);
    }

    // Handle state-specific interactions
    switch (controller->state) {

    case GAME_STATE_VIEW:
        break;
    case GAME_STATE_INVENTORY:
        // Waiting for the user to select an inventory item
        if (game_controller_handle_inventory_input(controller, input)) {
            game_controller_set_state(controller, GAME_STATE_HOLDING_ITEM);
            return;
        }
        break;
    case GAME_STATE_HOLDING_ITEM:
        if (ui_is_hovered(ID_GAME_AREA)) {
            game_update_preview_at_position(controller->game,
                                            controller->hovered_cell);
        }

        if (!ui_is_hovered(ID_GAME_AREA)) {
            game_clear_preview(controller->game);
        }

        if (input->key_r_pressed) {
            inventory_rotate_selected(controller->game->inventory, 1);
        }

        // Only place if the input wasn't already consumed by inventory
        // selection
        if (input->should_place_tile && ui_is_hovered(ID_GAME_AREA)) {
            game_controller_set_state(controller, GAME_STATE_PLACE);
            printf("ui is hovered, %d, should place: %d",
                   ui_is_hovered(ID_INVENTORY_AREA), input->should_place_tile);
        };

        break;
    case GAME_STATE_PLACE:

        game_try_place_tile(controller->game, controller->hovered_cell);
        game_controller_set_state(controller, GAME_STATE_VIEW);
        inventory_clear_selected_index(controller->game->inventory);
        game_clear_preview(controller->game);

        break;

    case GAME_STATE_COLLECTING:

    default:
        break;
    }
}

void game_controller_update_hover_state(game_controller_t *controller,
                                        const input_state_t *input) {
    if (!controller->game || !controller->game->board) {
        controller->hovered_tile = NULL;

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
      board_tile_at_cell(controller->game->board, controller->hovered_cell);
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
            return inventory_set_selected(controller->game->inventory, i);
        };
    }

    return false;
}

void game_controller_set_state(game_controller_t *controller,
                               game_state_e new_state) {

    controller->previous_state = controller->state;
    controller->state = new_state;
    printf("Game state changed to: %s\n",
           game_controller_state_to_string(new_state));
}

game_state_e game_controller_get_state(game_controller_t *controller) {
    return controller->state;
}

game_state_e game_controller_get_previous_state(game_controller_t *controller) {
    return controller->previous_state;
}

void game_controller_cycle_state(game_controller_t *controller) {
    controller->state = (controller->state + 1) % GAME_STATE_COUNT;
}

void game_controller_revert_state(game_controller_t *controller) {
    controller->state = controller->previous_state;
}

const char *game_controller_state_to_string(game_state_e state) {
    switch (state) {
    case GAME_STATE_VIEW:
        return "View";
    case GAME_STATE_INSPECT:
        return "Inspect";
    case GAME_STATE_INVENTORY:
        return "Inventory";
    case GAME_STATE_HOLDING_ITEM:
        return "Holding Item";
    case GAME_STATE_PLACE:
        return "Place";
    case GAME_STATE_COLLECTING:
        return "Collection";
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
