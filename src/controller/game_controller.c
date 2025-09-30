#include "controller/game_controller.h"
#include "controller/input_state.h"
#include "game/board.h"
#include "game/camera.h"
#include "game/game.h"
#include "game/inventory.h"
#include "grid/grid_geometry.h"
#include "stdio.h"
#include "ui.h"
#include "utility/geometry.h"

void controller_init(game_controller_t *controller, game_t *game) {
  controller->game = game;
  controller->last_clicked_ui_element_id = UI_ID_NONE;
  controller->hovered_element_id = UI_ID_NONE;
  controller->game_bounds = Clay_GetElementData(UI_ID_GAME).boundingBox;

  // Initialize input state
  input_state_init(&controller->input);
}

void controller_update(game_controller_t *controller, input_state_t *input) {
  Clay_BoundingBox bounds = Clay_GetElementData(UI_ID_GAME).boundingBox;
  controller->input = *input;
  controller->game_bounds = bounds;
  update_game(controller->game, input);

  // Handle rotation of currently selected/held inventory item
  if (input->key_r_pressed) {
    if (inventory_rotate_selected(controller->game->inventory, 1)) {
      printf("Rotated inventory item clockwise\n");
    }
  }
  if (input->key_m_pressed) {
    printf("HI");
    game_state_cycle(controller->game);
  }
  // Update game camera with current input
  if (point_in_bounds(input->mouse, controller->game_bounds) &&
      controller->input.hovered_element_id.id == UI_ID_GAME.id) {
    update_camera(&controller->game->board->camera, input);
  }
}

void controller_hover(game_controller_t *controller, Clay_ElementId elementId) {
  controller->hovered_element_id = elementId;
}

static void handle_add_inventory_button_click(game_controller_t *ctrl);
static void handle_game_area_click(game_controller_t *ctrl);
static void handle_tile_placement(game_controller_t *ctrl);

void controller_set_hover(game_controller_t *ctrl, ui_event_t evt) {
  ctrl->input.hovered_element_id = evt.element_id;
  ctrl->input.drag_bounds = evt.element_data.boundingBox;
}

void controller_clear_hover(game_controller_t *ctrl, ui_event_t evt) {
  if (ctrl->hovered_element_id.id == evt.element_id.id) {
    ctrl->hovered_element_id = (Clay_ElementId){0};
  }
}

void controller_process_events(game_controller_t *ctrl) {
  ui_event_t evt;
  while ((evt = ui_poll_event()).type != UI_EVENT_NONE) {
    switch (evt.type) {
    case UI_EVENT_HOVER_START:
      controller_set_hover(ctrl, evt);
      break;
    case UI_EVENT_HOVER_END:
      controller_clear_hover(ctrl, evt);
      break;
    case UI_EVENT_CLICK:
      ctrl->last_clicked_ui_element_id = evt.element_id;
      printf("Clicked on element: %s\n", evt.element_id.stringId.chars);

      if (ctrl->last_clicked_ui_element_id.id ==
          UI_BUTTON_ADD_INVENTORY_ITEM.id) {
        handle_add_inventory_button_click(ctrl);
      } else if (ctrl->last_clicked_ui_element_id.id == UI_ID_GAME.id) {
        handle_game_area_click(ctrl);
      }

      break;
    case UI_EVENT_NONE:
      // No special handling needed - rotation handled in controller_update
      break;
    case UI_EVENT_INVENTORY_ITEM_CLICK:
      printf("Inventory item clicked: %s\n", evt.element_id.stringId.chars);

      // Find which inventory item was clicked by matching the element ID
      int inventory_size = inventory_get_size(ctrl->game->inventory);
      for (int i = 0; i < inventory_size; i++) {
        inventory_item_t item = inventory_get_item(ctrl->game->inventory, i);
        if (item.id.id == evt.element_id.id) {
          inventory_set_index(ctrl->game->inventory, i);
          printf("Selected inventory item at index %d\n", i);
          break;
        }
      }
      break;
    default:
      break;
    }
  }
}

static void handle_add_inventory_button_click(game_controller_t *ctrl) {
  inventory_add_random_item(ctrl->game->inventory);
  printf("Added random item to inventory\n");
}

static void handle_game_area_click(game_controller_t *ctrl) {
  if (ctrl->input.mouse_left_was_dragging) {
    printf("Game area dragged - camera moved, no tile placement\n");
  } else {
    printf("Game area clicked!\n");
    handle_tile_placement(ctrl);
  }
}

static void enter_collect_state(game_controller_t *ctrl) {
  printf("Entering collect state\n");
  ctrl->game->state = GAME_STATE_COLLECT;
  ctrl->game->inventory->selected_index = -1;
}

static void handle_tile_placement(game_controller_t *ctrl) {
  board_t *selected_board = inventory_get_selected_board(ctrl->game->inventory);

  if (!selected_board) {
    printf("No inventory item selected for merging\n");
    return;
  }

  if (!ctrl->game->board->hovered_grid_cell) {
    printf("No valid hover position on game board\n");
    return;
  }

  grid_cell_t target_position = *(ctrl->game->board->hovered_grid_cell);
  printf("Attempting to merge at position (%d, %d)\n",
         target_position.coord.hex.q, target_position.coord.hex.r);

  grid_cell_t source_center =
    grid_get_center_cell(selected_board->geometry_type);

  if (merge_boards(ctrl->game->board, selected_board, target_position,
                   source_center)) {
    printf("Successfully merged inventory item into main board\n");
    enter_collect_state(ctrl);
  } else {
    printf("Failed to merge inventory item - invalid placement\n");
  }
}
