#include "controller/game_controller.h"
#include "controller/input_state.h"
#include "game/camera.h"
#include "stdio.h"
#include "ui.h"
#include "utility/geometry.h"

void controller_init(game_controller_t *controller, game_t *game) {
  controller->game = game;
  controller->last_clicked_ui_element_id = UI_ID_NONE;
  controller->hovered_element_id = UI_ID_NONE;
  controller->game_bounds = Clay_GetElementData(UI_ID_GAME).boundingBox;

  camera_init(&controller->game_camera);

  // Initialize input state
  input_state_init(&controller->input);
}

void controller_update(game_controller_t *controller, input_state_t *input) {
  Clay_BoundingBox bounds = Clay_GetElementData(UI_ID_GAME).boundingBox;
  controller->input = *input;
  controller->game_bounds = bounds;

  // Update game camera with current input
  if (point_in_bounds(input->mouse, controller->game_bounds) &&
      controller->last_clicked_ui_element_id.id == UI_ID_GAME.id) {
    update_camera(&controller->game_camera, input);
  }
}

void controller_hover(game_controller_t *controller, Clay_ElementId elementId) {
  controller->hovered_element_id = elementId;
}

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
        // inventory_add_random_item(ctrl->game->inventory);
        // ctrl->game->board->grid->layout.scale += 0.1f;

        board_t *new_board = board_create(ctrl->game->board->grid->type, 0);
        board_randomize(new_board);
        printf("tiles in new board: %d\n", new_board->tiles->num_tiles);
        merge_boards(ctrl->game->board, new_board,
                     grid_get_center_cell(ctrl->game->board->grid),
                     grid_get_center_cell(new_board->grid));
        free_board(new_board);
        print_board_debug_info(ctrl->game->board);
      }

      break;
    default:
      break;
    }
  }
}
