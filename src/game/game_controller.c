#include "game/game_controller.h"
#include "game/camera.h"
#include "game/input_state.h"
#include "stdio.h"
#include "ui.h"
#include "utility/geometry.h"

void controller_init(game_controller_t *controller, game_t *game) {
  controller->game = game;
  controller->hovered_element_id = CLAY_ID("none");

  // Initialize input state
  input_state_init(&controller->input);
}

void controller_update(game_controller_t *controller, input_state_t *input) {
  // Update controller's input state with current input
  controller->input = *input;
  // Update camera with current input

  // printf("bounding box: %f, %f, %f, %f\n", controller->game_bounds.x,
  //        controller->game_bounds.y, controller->game_bounds.width,
  //        controller->game_bounds.height);

  if (point_in_bounds(input->mouse, controller->game_bounds)) {
    update_camera(&controller->camera, input);
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
      printf("click\n");
      break;
    default:
      break;
    }
  }
}
