#include "controller/menu_controller.h"
#include "controller/app_controller.h"
#include <stdio.h>
#include <string.h>

void menu_controller_init(menu_controller_t *menu_controller,
                          app_controller_t *app_controller) {
  menu_controller->current_state = MENU_STATE_MAIN;
  menu_controller->selected_item_index = 0;
  menu_controller->menu_item_count = 4; // NEW_GAME, CONTINUE, SETTINGS, QUIT
  menu_controller->last_clicked_element = (Clay_ElementId){0};
  menu_controller->hovered_element = (Clay_ElementId){0};
  menu_controller->app_controller = app_controller;
  menu_controller->show_continue_button = false; // TODO: Check for save files
  memset(menu_controller->confirmation_message, 0,
         sizeof(menu_controller->confirmation_message));

  printf("Menu controller initialized\n");
}

void menu_controller_cleanup(menu_controller_t *menu_controller) {
  if (menu_controller) {
    menu_controller->app_controller = NULL;
    printf("Menu controller cleaned up\n");
  }
}

void menu_controller_update(menu_controller_t *menu_controller,
                            input_state_t *input) {
  if (!menu_controller || !input) {
    return;
  }

  // Handle keyboard navigation
  if (input->key_up_pressed) {
    menu_controller_navigate_up(menu_controller);
  }
  if (input->key_down_pressed) {
    menu_controller_navigate_down(menu_controller);
  }
  if (input->key_enter_pressed || input->key_space_pressed) {
    menu_controller_select_item(menu_controller);
  }
  if (input->key_escape_pressed) {
    menu_controller_back(menu_controller);
  }
}

void menu_controller_process_events(menu_controller_t *menu_controller) {
  if (!menu_controller) {
    return;
  }

  // TODO: Process UI events for menu interactions
  // This would handle mouse clicks on menu items
}

void menu_controller_navigate_up(menu_controller_t *menu_controller) {
  if (menu_controller->selected_item_index > 0) {
    menu_controller->selected_item_index--;
    printf("Menu: Navigate up to item %d\n",
           menu_controller->selected_item_index);
  }
}

void menu_controller_navigate_down(menu_controller_t *menu_controller) {
  if (menu_controller->selected_item_index <
      menu_controller->menu_item_count - 1) {
    menu_controller->selected_item_index++;
    printf("Menu: Navigate down to item %d\n",
           menu_controller->selected_item_index);
  }
}

void menu_controller_select_item(menu_controller_t *menu_controller) {
  if (!menu_controller || !menu_controller->app_controller) {
    return;
  }

  switch (menu_controller->current_state) {
  case MENU_STATE_MAIN:
    switch (menu_controller->selected_item_index) {
    case 0: // NEW_GAME
      menu_controller_handle_new_game(menu_controller);
      break;
    case 1: // CONTINUE (if available)
      if (menu_controller->show_continue_button) {
        menu_controller_handle_continue(menu_controller);
      } else {
        menu_controller_handle_settings(menu_controller);
      }
      break;
    case 2: // SETTINGS or QUIT
      if (menu_controller->show_continue_button) {
        menu_controller_handle_settings(menu_controller);
      } else {
        menu_controller_handle_quit(menu_controller);
      }
      break;
    case 3: // QUIT (if continue button is shown)
      menu_controller_handle_quit(menu_controller);
      break;
    default:
      break;
    }
    break;

  case MENU_STATE_CONFIRMATION:
    // Handle confirmation dialog selection
    if (menu_controller->selected_item_index == 0) {
      // Confirm action
      menu_controller_handle_quit(menu_controller);
    } else {
      // Cancel action
      menu_controller_set_state(menu_controller, MENU_STATE_MAIN);
    }
    break;

  default:
    break;
  }
}

void menu_controller_back(menu_controller_t *menu_controller) {
  switch (menu_controller->current_state) {
  case MENU_STATE_CONFIRMATION:
    menu_controller_set_state(menu_controller, MENU_STATE_MAIN);
    break;
  case MENU_STATE_MAIN:
    // ESC from main menu could show quit confirmation
    menu_controller_show_confirmation(menu_controller,
                                      "Are you sure you want to quit?");
    break;
  default:
    break;
  }
}

void menu_controller_handle_new_game(menu_controller_t *menu_controller) {
  printf("Menu: Starting new game\n");
  app_controller_start_game(menu_controller->app_controller);
}

void menu_controller_handle_continue(menu_controller_t *menu_controller) {
  printf("Menu: Continuing saved game\n");
  // TODO: Load saved game state
  app_controller_start_game(menu_controller->app_controller);
}

void menu_controller_handle_settings(menu_controller_t *menu_controller) {
  printf("Menu: Opening settings\n");
  app_controller_open_settings(menu_controller->app_controller);
}

void menu_controller_handle_quit(menu_controller_t *menu_controller) {
  printf("Menu: Quitting application\n");
  app_controller_quit_application(menu_controller->app_controller);
}

void menu_controller_set_state(menu_controller_t *menu_controller,
                               menu_state_t state) {
  if (menu_controller->current_state != state) {
    printf("Menu state transition: %d -> %d\n", menu_controller->current_state,
           state);
    menu_controller->current_state = state;
    menu_controller->selected_item_index = 0; // Reset selection

    // Update menu item count based on state
    switch (state) {
    case MENU_STATE_MAIN:
      menu_controller->menu_item_count =
        menu_controller->show_continue_button ? 4 : 3;
      break;
    case MENU_STATE_CONFIRMATION:
      menu_controller->menu_item_count = 2; // Confirm, Cancel
      break;
    default:
      break;
    }
  }
}

menu_state_t menu_controller_get_state(menu_controller_t *menu_controller) {
  return menu_controller ? menu_controller->current_state : MENU_STATE_MAIN;
}

void menu_controller_show_confirmation(menu_controller_t *menu_controller,
                                       const char *message) {
  if (menu_controller && message) {
    strncpy(menu_controller->confirmation_message, message,
            sizeof(menu_controller->confirmation_message) - 1);
    menu_controller
      ->confirmation_message[sizeof(menu_controller->confirmation_message) -
                             1] = '\0';
    menu_controller_set_state(menu_controller, MENU_STATE_CONFIRMATION);
    printf("Menu: Showing confirmation: %s\n", message);
  }
}

int menu_controller_get_selected_item(menu_controller_t *menu_controller) {
  return menu_controller ? menu_controller->selected_item_index : 0;
}
