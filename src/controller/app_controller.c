#include "controller/app_controller.h"
#include "controller/menu_controller.h"
#include "controller/settings_controller.h"
#include "game/game.h"
#include <stdio.h>
#include <stdlib.h>

void app_controller_init(app_controller_t *app_controller) {
  // Initialize application state
  app_controller->current_state = APP_STATE_MAIN_MENU;
  app_controller->previous_state = APP_STATE_MAIN_MENU;
  app_controller->is_initialized = true;
  app_controller->should_quit = false;

  // Initialize menu navigation
  app_controller->selected_menu_item = 0;
  app_controller->menu_item_count = 3; // New Game, Settings, Quit

  // Initialize input state
  input_state_init(&app_controller->input);

  // Initialize menu controller
  app_controller->menu_controller = malloc(sizeof(menu_controller_t));
  if (app_controller->menu_controller) {
    menu_controller_init(app_controller->menu_controller, app_controller);
  }

  // Initialize settings controller
  app_controller->settings_controller = malloc(sizeof(settings_controller_t));
  if (app_controller->settings_controller) {
    settings_controller_init(app_controller->settings_controller,
                             app_controller);
  }

  // Game will be initialized when entering PLAYING state
  app_controller->game = NULL;
  app_controller_start_game(app_controller);
  printf("App controller initialized in PLAYING state\n");
}

void app_controller_cleanup(app_controller_t *app_controller) {
  if (!app_controller->is_initialized) {
    return;
  }

  // Cleanup game controller and game if active
  if (app_controller->game) {
    free_game(app_controller->game);
    app_controller->game = NULL;
  }

  // Cleanup menu controller
  if (app_controller->menu_controller) {
    menu_controller_cleanup(app_controller->menu_controller);
    free(app_controller->menu_controller);
    app_controller->menu_controller = NULL;
  }

  // Cleanup settings controller
  if (app_controller->settings_controller) {
    settings_controller_cleanup(app_controller->settings_controller);
    free(app_controller->settings_controller);
    app_controller->settings_controller = NULL;
  }

  app_controller->is_initialized = false;
  printf("App controller cleaned up\n");
}

void app_controller_update(app_controller_t *app_controller,
                           input_state_t *input) {
  if (!app_controller->is_initialized) {
    printf("Warning: App controller not initialized\n");
    return;
  }

  // Store input state
  app_controller->input = *input;

  // Handle global input (ESC key, etc.)
  app_controller_handle_global_input(app_controller, input);

  // Update based on current state
  switch (app_controller->current_state) {
  case APP_STATE_MAIN_MENU:
    // Handle menu navigation
    if (input->key_up_pressed) {
      app_controller->selected_menu_item =
        (app_controller->selected_menu_item - 1 +
         app_controller->menu_item_count) %
        app_controller->menu_item_count;
      printf("Menu selection: %d\n", app_controller->selected_menu_item);
    }
    if (input->key_down_pressed) {
      app_controller->selected_menu_item =
        (app_controller->selected_menu_item + 1) %
        app_controller->menu_item_count;
      printf("Menu selection: %d\n", app_controller->selected_menu_item);
    }

    // Handle menu selection
    if (input->key_enter_pressed || input->key_space_pressed) {
      switch (app_controller->selected_menu_item) {
      case 0: // New Game
        app_controller_start_game(app_controller);
        break;
      case 1: // Settings
        app_controller_open_settings(app_controller);
        break;
      case 2: // Quit
        app_controller_quit_application(app_controller);
        break;
      }
    }

    if (app_controller->menu_controller) {
      menu_controller_update(app_controller->menu_controller, input);
    }
    break;

  case APP_STATE_SETTINGS:
    // Simple settings navigation - ESC to return to menu (handled globally)
    if (app_controller->settings_controller) {
      settings_controller_update(app_controller->settings_controller, input);
    }
    break;

  case APP_STATE_PLAYING:
    if (app_controller->game) {
      controller_update(&app_controller->game_controller, input);
    }
    break;

  case APP_STATE_PAUSED:
    // Paused state - no game updates, but could handle pause menu
    break;

  case APP_STATE_QUIT:
    app_controller->should_quit = true;
    break;
  }
}

void app_controller_process_events(app_controller_t *app_controller) {
  if (!app_controller->is_initialized) {
    return;
  }

  // Process events based on current state
  switch (app_controller->current_state) {
  case APP_STATE_MAIN_MENU:
    if (app_controller->menu_controller) {
      menu_controller_process_events(app_controller->menu_controller);
    }
    break;

  case APP_STATE_SETTINGS:
    if (app_controller->settings_controller) {
      settings_controller_process_events(app_controller->settings_controller);
    }
    break;

  case APP_STATE_PLAYING:
    if (app_controller->game) {
      controller_process_events(&app_controller->game_controller);
    }
    break;

  case APP_STATE_PAUSED:
    // Handle pause menu events
    break;

  case APP_STATE_QUIT:
    // No events to process when quitting
    break;
  }
}

void app_controller_set_state(app_controller_t *app_controller,
                              app_state_t new_state) {
  if (app_controller->current_state == new_state) {
    return;
  }

  app_state_t old_state = app_controller->current_state;
  app_controller->previous_state = old_state;
  app_controller->current_state = new_state;

  printf("App state transition: %d -> %d\n", old_state, new_state);

  // Handle state exit logic
  switch (old_state) {
  case APP_STATE_PLAYING:
    // Don't cleanup game when pausing, only when going to menu
    if (new_state == APP_STATE_MAIN_MENU || new_state == APP_STATE_QUIT) {
      if (app_controller->game) {
        free_game(app_controller->game);
        free(app_controller->game);
        app_controller->game = NULL;
      }
    }
    break;
  default:
    break;
  }

  // Handle state entry logic
  switch (new_state) {
  case APP_STATE_PLAYING:
    if (!app_controller->game) {
      app_controller->game = malloc(sizeof(game_t));
      game_init(app_controller->game);
      controller_init(&app_controller->game_controller, app_controller->game);
      printf("Game initialized for PLAYING state\n");
    }
    break;

  case APP_STATE_MAIN_MENU:
    // Reset menu selection when entering main menu
    app_controller->selected_menu_item = 0;
    if (app_controller->menu_controller) {
      menu_controller_set_state(app_controller->menu_controller,
                                MENU_STATE_MAIN);
    }
    break;

  case APP_STATE_SETTINGS:
    if (app_controller->settings_controller) {
      settings_controller_set_category(app_controller->settings_controller,
                                       SETTINGS_CATEGORY_GRAPHICS);
    }
    break;

  default:
    break;
  }
}

app_state_t app_controller_get_state(app_controller_t *app_controller) {
  return app_controller->current_state;
}

bool app_controller_should_quit(app_controller_t *app_controller) {
  return app_controller->should_quit;
}

void app_controller_start_game(app_controller_t *app_controller) {
  printf("Starting new game\n");
  app_controller_set_state(app_controller, APP_STATE_PLAYING);
}

void app_controller_pause_game(app_controller_t *app_controller) {
  if (app_controller->current_state == APP_STATE_PLAYING) {
    printf("Pausing game\n");
    app_controller_set_state(app_controller, APP_STATE_PAUSED);
  }
}

void app_controller_resume_game(app_controller_t *app_controller) {
  if (app_controller->current_state == APP_STATE_PAUSED) {
    printf("Resuming game\n");
    app_controller_set_state(app_controller, APP_STATE_PLAYING);
  }
}

void app_controller_quit_to_menu(app_controller_t *app_controller) {
  printf("Returning to main menu\n");
  app_controller_set_state(app_controller, APP_STATE_MAIN_MENU);
}

void app_controller_open_settings(app_controller_t *app_controller) {
  printf("Opening settings\n");
  app_controller_set_state(app_controller, APP_STATE_SETTINGS);
}

void app_controller_quit_application(app_controller_t *app_controller) {
  printf("Quitting application\n");
  app_controller_set_state(app_controller, APP_STATE_QUIT);
}

void app_controller_handle_global_input(app_controller_t *app_controller,
                                        input_state_t *input) {
  // ESC key handling based on current state
  if (input->key_escape_pressed) {
    switch (app_controller->current_state) {
    case APP_STATE_PLAYING:
      app_controller_pause_game(app_controller);
      break;

    case APP_STATE_PAUSED:
      app_controller_resume_game(app_controller);
      break;

    case APP_STATE_SETTINGS:
      // Check if settings have unsaved changes
      if (app_controller->settings_controller &&
          settings_controller_has_unsaved_changes(
            app_controller->settings_controller)) {
        // Could show confirmation dialog here
      }
      app_controller_quit_to_menu(app_controller);
      break;

    case APP_STATE_MAIN_MENU:
      app_controller_quit_application(app_controller);
      break;

    default:
      break;
    }
  }

  // F1 for settings (global hotkey)
  if (input->key_f1_pressed) {
    app_controller_open_settings(app_controller);
  }
}
