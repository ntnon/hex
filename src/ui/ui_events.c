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

void handle_inventory_click(Clay_ElementId elementId,
                            Clay_PointerData pointerData, intptr_t userData) {
  game_controller_t *controller = (game_controller_t *)userData;
  Clay_ElementData element_data = Clay_GetElementData(elementId);
  ui_push_event((ui_event_t){.type = UI_EVENT_INVENTORY_CLICK,
                             .element_id = elementId,
                             .element_data = element_data});
}

void handle_inventory_item_click(Clay_ElementId elementId,
                                 Clay_PointerData pointerData,
                                 intptr_t userData) {
  game_controller_t *controller = (game_controller_t *)userData;
  Clay_ElementData element_data = Clay_GetElementData(elementId);

  // Handle hover events
  if (elementId.id != controller->hovered_element_id.id &&
      controller->hovered_element_id.id != 0) {
    ui_push_event((ui_event_t){.type = UI_EVENT_HOVER_END,
                               .element_id = controller->hovered_element_id,
                               .element_data = element_data});
  }

  ui_push_event((ui_event_t){.type = UI_EVENT_HOVER_START,
                             .element_id = elementId,
                             .element_data = element_data});

  // Update controller immediately
  controller->hovered_element_id = elementId;

  // Handle click events
  if (pointerData.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
    ui_push_event((ui_event_t){.type = UI_EVENT_INVENTORY_ITEM_CLICK,
                               .element_id = elementId,
                               .element_data = element_data});
  }
}

void handle_hover(Clay_ElementId elementId, Clay_PointerData pointerData,
                  intptr_t userData) {

  game_controller_t *controller = (game_controller_t *)userData;
  Clay_ElementData element_data = Clay_GetElementData(elementId);
  if (elementId.id != controller->hovered_element_id.id &&
      controller->hovered_element_id.id != 0) {
    ui_push_event((ui_event_t){.type = UI_EVENT_HOVER_END,
                               .element_id = controller->hovered_element_id,
                               .element_data = element_data});
  }

  ui_push_event((ui_event_t){.type = UI_EVENT_HOVER_START,
                             .element_id = elementId,
                             .element_data = element_data});

  // Update controller immediately
  controller->hovered_element_id = elementId;

  // Handle click events on release, but only if no dragging occurred
  if (pointerData.state == CLAY_POINTER_DATA_RELEASED_THIS_FRAME) {
    // Only generate click event if we didn't drag during the press-release
    // cycle
    if (!controller->input.mouse_left_was_dragging) {
      ui_push_event((ui_event_t){.type = UI_EVENT_CLICK,
                                 .element_id = elementId,
                                 .element_data = element_data});
    }
  }
}
