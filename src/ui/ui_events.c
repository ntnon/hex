#include "controller/app_controller.h"
#include "ui.h"
#include "ui_types.h"
#include <stdio.h>

// Global UI state - what's hovered and what was clicked
static Clay_ElementId g_current_hovered_element = {0};
static Clay_ElementId g_last_clicked_element = {0};
static bool g_click_occurred = false;

// Generic hover handler - just tracks what's hovered
void ui_hover_handler(Clay_ElementId elementId, Clay_PointerData pointerData,
                      intptr_t userData) {
  g_current_hovered_element = elementId;

  // Track clicks
  if (pointerData.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
    g_last_clicked_element = elementId;
    g_click_occurred = true;
  }
}

// Get current hover
Clay_ElementId ui_get_hovered_element(void) {
  return g_current_hovered_element;
}

// Check if something was clicked this frame
bool ui_was_clicked(Clay_ElementId elementId) {
  if (g_click_occurred && g_last_clicked_element.id == elementId.id) {
    return true;
  }
  return false;
}

// Clear click state (call this at end of frame)
void ui_clear_click(void) {
  if (g_click_occurred) {
    g_click_occurred = false;
    g_last_clicked_element = (Clay_ElementId){0};
    printf("click unclicked\n");
  }
}

// Special handler for menu buttons that need immediate visual feedback
void handle_menu_button_hover(Clay_ElementId elementId,
                              Clay_PointerData pointerData, intptr_t userData) {
  app_controller_t *app_controller = (app_controller_t *)userData;

  // Track hover globally
  ui_hover_handler(elementId, pointerData, userData);

  // Update hover state for visual feedback
  if (elementId.id == UI_ID_MENU_ITEM_NEW_GAME.id) {
    app_controller->selected_menu_item = 0;
  } else if (elementId.id == UI_ID_MENU_ITEM_SETTINGS.id) {
    app_controller->selected_menu_item = 1;
  } else if (elementId.id == UI_ID_MENU_ITEM_QUIT.id) {
    app_controller->selected_menu_item = 2;
  }

  // Handle clicks
  if (pointerData.state == CLAY_POINTER_DATA_RELEASED_THIS_FRAME) {
    if (elementId.id == UI_ID_MENU_ITEM_NEW_GAME.id) {
      app_controller_start_game(app_controller);
    } else if (elementId.id == UI_ID_MENU_ITEM_SETTINGS.id) {
      app_controller_open_settings(app_controller);
    } else if (elementId.id == UI_ID_MENU_ITEM_QUIT.id) {
      app_controller_quit_application(app_controller);
    } else if (elementId.id == UI_ID_SETTINGS_BACK_BUTTON.id) {
      app_controller_set_state(app_controller, APP_STATE_MAIN_MENU);
    } else if (elementId.id == UI_ID_PAUSE_RESUME_BUTTON.id) {
      app_controller_resume_game(app_controller);
    } else if (elementId.id == UI_ID_PAUSE_SETTINGS_BUTTON.id) {
      app_controller_open_settings(app_controller);
    } else if (elementId.id == UI_ID_PAUSE_QUIT_BUTTON.id) {
      app_controller_quit_to_menu(app_controller);
    }
  }
}
