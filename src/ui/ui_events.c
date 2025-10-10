#include "controller/app_controller.h"
#include "controller/game_controller.h"
#include "stdio.h"
#include "ui.h"
#include "ui_types.h"
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
  Clay_ElementId current_hovered =
    game_controller_get_hovered_element_id(controller);
  if (elementId.id != current_hovered.id && current_hovered.id != 0) {
    ui_push_event((ui_event_t){.type = UI_EVENT_HOVER_END,
                               .element_id = current_hovered,
                               .element_data = element_data});
  }

  ui_push_event((ui_event_t){.type = UI_EVENT_HOVER_START,
                             .element_id = elementId,
                             .element_data = element_data});

  // Update controller immediately
  game_controller_set_hovered_element_id(controller, elementId);

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
  Clay_ElementId current_hovered =
    game_controller_get_hovered_element_id(controller);
  if (elementId.id != current_hovered.id && current_hovered.id != 0) {
    ui_push_event((ui_event_t){.type = UI_EVENT_HOVER_END,
                               .element_id = current_hovered,
                               .element_data = element_data});
  }

  ui_push_event((ui_event_t){.type = UI_EVENT_HOVER_START,
                             .element_id = elementId,
                             .element_data = element_data});

  // Update controller immediately
  game_controller_set_hovered_element_id(controller, elementId);

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

void handle_menu_button_hover(Clay_ElementId elementId,
                              Clay_PointerData pointerData, intptr_t userData) {
  app_controller_t *app_controller = (app_controller_t *)userData;

  // Update hover state for visual feedback on main menu buttons
  if (elementId.id == UI_ID_MENU_ITEM_NEW_GAME.id) {
    app_controller->selected_menu_item = 0;
  } else if (elementId.id == UI_ID_MENU_ITEM_SETTINGS.id) {
    app_controller->selected_menu_item = 1;
  } else if (elementId.id == UI_ID_MENU_ITEM_QUIT.id) {
    app_controller->selected_menu_item = 2;
  }

  // Handle click events
  if (pointerData.state == CLAY_POINTER_DATA_RELEASED_THIS_FRAME) {
    // Main menu button clicks
    if (elementId.id == UI_ID_MENU_ITEM_NEW_GAME.id) {
      printf("New Game button clicked via mouse\n");
      app_controller_start_game(app_controller);
    } else if (elementId.id == UI_ID_MENU_ITEM_SETTINGS.id) {
      printf("Settings button clicked via mouse\n");
      app_controller_open_settings(app_controller);
    } else if (elementId.id == UI_ID_MENU_ITEM_QUIT.id) {
      printf("Quit button clicked via mouse\n");
      app_controller_quit_application(app_controller);
    }
    // Settings menu button clicks
    else if (elementId.id == UI_ID_SETTINGS_BACK_BUTTON.id) {
      printf("Settings back button clicked via mouse\n");
      app_controller_set_state(app_controller, APP_STATE_MAIN_MENU);
    }
    // Pause menu button clicks
    else if (elementId.id == UI_ID_PAUSE_RESUME_BUTTON.id) {
      printf("Resume button clicked via mouse\n");
      app_controller_resume_game(app_controller);
    } else if (elementId.id == UI_ID_PAUSE_SETTINGS_BUTTON.id) {
      printf("Pause settings button clicked via mouse\n");
      app_controller_open_settings(app_controller);
    } else if (elementId.id == UI_ID_PAUSE_QUIT_BUTTON.id) {
      printf("Pause quit button clicked via mouse\n");
      app_controller_quit_to_menu(app_controller);
    }
  }
}
