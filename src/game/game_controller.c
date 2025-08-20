#include "game/game_controller.h"
#include "game/camera.h"
#include "stdio.h"
#include "ui.h"

void controller_init(game_controller_t *controller, game_t *game) {
  controller->game = game;
  controller->hovered_element_id = CLAY_ID("none");

  // Initialize input state
  input_state_init(&controller->input);

  // Initialize camera
  controller->camera = (Camera2D){0};
  controller->camera.target = (Vector2){0.0f, 0.0f};
  controller->camera.offset =
    (Vector2){GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};
  controller->camera.zoom = 1.0f;
}

void controller_update(game_controller_t *controller, input_state_t *input) {
  // Update controller's input state with current input
  controller->input = *input;

  // Update camera with current input
  update_camera(&controller->camera, input);
}

void controller_hover(game_controller_t *controller, Clay_ElementId elementId) {
  controller->hovered_element_id = elementId;
  printf("HI %s\n", elementId.stringId.chars);
}

void controller_handle_events(game_controller_t *ctrl) {
  ui_event_t evt;

  while ((evt = ui_poll_event()).type != UI_EVENT_NONE) {
    switch (evt.type) {
    case UI_EVENT_HOVER_START:
      ctrl->hovered_element_id = evt.element_id;
      break;
    case UI_EVENT_HOVER_END:
      if (ctrl->hovered_element_id.id == evt.element_id.id)
        ctrl->hovered_element_id = (Clay_ElementId){0};
      break;
    case UI_EVENT_CLICK:
      // handle click
      break;
    default:
      break;
    }
  }
}
