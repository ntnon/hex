// #include "game/game_controller.h"
#include "stdio.h"
#include "ui.h"
#include <stdio.h>

// ui_event.c
static ui_event_t event_queue[MAX_UI_EVENTS];
static int event_count = 0;

void ui_push_event(ui_event_t evt) {
  if (event_count < MAX_UI_EVENTS) {
    event_queue[event_count++] = evt;
  }
}

ui_event_t ui_poll_event(void) {
  if (event_count == 0)
    return (ui_event_t){.type = UI_EVENT_NONE};
  return event_queue[--event_count]; // LIFO or FIFO depending on style
}

void ui_clear_events(void) { event_count = 0; }

void handle_hover(Clay_ElementId elementId, Clay_PointerData pointerData,
                  intptr_t userData) {
  game_controller_t *controller = (game_controller_t *)userData;

  // Always update hovered element in controller (optional, immediate mode)
  // Push hover end if leaving previous element
  if (elementId.id != controller->hovered_element_id.id &&
      controller->hovered_element_id.id != 0) {
    ui_push_event(
      (ui_event_t){UI_EVENT_HOVER_END, controller->hovered_element_id});
  }

  ui_push_event((ui_event_t){UI_EVENT_HOVER_START, elementId});

  // Update controller immediately
  controller->hovered_element_id = elementId;
  // Optional: handle click or drag events
  if (pointerData.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
    ui_push_event((ui_event_t){
      .type = UI_EVENT_CLICK,
      .element_id = elementId,
    });
    controller->input.mouse_dragging = false;
  }
}
