#include "controller/settings_controller.h"
#include "controller/app_controller.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void settings_controller_init(settings_controller_t *settings_controller,
                              app_controller_t *app_controller) {
  settings_controller->current_category = SETTINGS_CATEGORY_GRAPHICS;
  settings_controller->selected_item_index = 0;
  settings_controller->selected_category_index = 0;
  settings_controller->last_clicked_element = (Clay_ElementId){0};
  settings_controller->hovered_element = (Clay_ElementId){0};
  settings_controller->app_controller = app_controller;

  // Initialize settings array
  settings_controller->settings_items = NULL;
  settings_controller->settings_count = 0;
  settings_controller->settings_capacity = 0;

  // Initialize default settings values
  settings_controller->fullscreen = false;
  settings_controller->vsync = true;
  settings_controller->master_volume = 1.0f;
  settings_controller->sfx_volume = 0.8f;
  settings_controller->music_volume = 0.7f;
  settings_controller->show_grid = true;
  settings_controller->show_coordinates = false;
  settings_controller->camera_sensitivity = 50;

  settings_controller->settings_modified = false;

  // Load settings from file
  settings_controller_load_settings(settings_controller);

  printf("Settings controller initialized\n");
}

void settings_controller_cleanup(settings_controller_t *settings_controller) {
  if (!settings_controller) {
    return;
  }

  // Save settings if modified
  if (settings_controller->settings_modified) {
    settings_controller_save_settings(settings_controller);
  }

  // Cleanup settings items array
  if (settings_controller->settings_items) {
    free(settings_controller->settings_items);
    settings_controller->settings_items = NULL;
  }

  settings_controller->app_controller = NULL;
  printf("Settings controller cleaned up\n");
}

void settings_controller_update(settings_controller_t *settings_controller,
                                input_state_t *input) {
  if (!settings_controller || !input) {
    return;
  }

  // Handle keyboard navigation
  if (input->key_up_pressed) {
    settings_controller_navigate_up(settings_controller);
  }
  if (input->key_down_pressed) {
    settings_controller_navigate_down(settings_controller);
  }
  if (input->key_left_pressed) {
    settings_controller_navigate_left(settings_controller);
  }
  if (input->key_right_pressed) {
    settings_controller_navigate_right(settings_controller);
  }
  if (input->key_enter_pressed || input->key_space_pressed) {
    settings_controller_select_item(settings_controller);
  }
  if (input->key_escape_pressed) {
    settings_controller_back(settings_controller);
  }

  // Tab to switch categories
  if (input->key_tab_pressed) {
    int next_category = (settings_controller->current_category + 1) % 4;
    settings_controller_set_category(settings_controller,
                                     (settings_category_t)next_category);
  }
}

void settings_controller_process_events(
  settings_controller_t *settings_controller) {
  if (!settings_controller) {
    return;
  }

  // TODO: Process UI events for settings interactions
  // This would handle mouse clicks on settings items, sliders, toggles, etc.
}

void settings_controller_navigate_up(
  settings_controller_t *settings_controller) {
  if (settings_controller->selected_item_index > 0) {
    settings_controller->selected_item_index--;
    printf("Settings: Navigate up to item %d\n",
           settings_controller->selected_item_index);
  }
}

void settings_controller_navigate_down(
  settings_controller_t *settings_controller) {
  // TODO: Calculate max items based on current category
  int max_items = 5; // Placeholder
  if (settings_controller->selected_item_index < max_items - 1) {
    settings_controller->selected_item_index++;
    printf("Settings: Navigate down to item %d\n",
           settings_controller->selected_item_index);
  }
}

void settings_controller_navigate_left(
  settings_controller_t *settings_controller) {
  // Adjust current setting value (decrease)
  switch (settings_controller->current_category) {
  case SETTINGS_CATEGORY_GRAPHICS:
    if (settings_controller->selected_item_index == 0) { // Fullscreen toggle
      settings_controller->fullscreen = false;
      settings_controller->settings_modified = true;
      printf("Settings: Fullscreen disabled\n");
    } else if (settings_controller->selected_item_index == 1) { // VSync toggle
      settings_controller->vsync = false;
      settings_controller->settings_modified = true;
      printf("Settings: VSync disabled\n");
    }
    break;

  case SETTINGS_CATEGORY_AUDIO:
    if (settings_controller->selected_item_index == 0) { // Master volume
      settings_controller->master_volume =
        fmaxf(0.0f, settings_controller->master_volume - 0.1f);
      settings_controller->settings_modified = true;
      printf("Settings: Master volume: %.1f\n",
             settings_controller->master_volume);
    }
    break;

  default:
    break;
  }
}

void settings_controller_navigate_right(
  settings_controller_t *settings_controller) {
  // Adjust current setting value (increase)
  switch (settings_controller->current_category) {
  case SETTINGS_CATEGORY_GRAPHICS:
    if (settings_controller->selected_item_index == 0) { // Fullscreen toggle
      settings_controller->fullscreen = true;
      settings_controller->settings_modified = true;
      printf("Settings: Fullscreen enabled\n");
    } else if (settings_controller->selected_item_index == 1) { // VSync toggle
      settings_controller->vsync = true;
      settings_controller->settings_modified = true;
      printf("Settings: VSync enabled\n");
    }
    break;

  case SETTINGS_CATEGORY_AUDIO:
    if (settings_controller->selected_item_index == 0) { // Master volume
      settings_controller->master_volume =
        fminf(1.0f, settings_controller->master_volume + 0.1f);
      settings_controller->settings_modified = true;
      printf("Settings: Master volume: %.1f\n",
             settings_controller->master_volume);
    }
    break;

  default:
    break;
  }
}

void settings_controller_select_item(
  settings_controller_t *settings_controller) {
  // Toggle boolean settings or activate actions
  printf("Settings: Selected item %d in category %d\n",
         settings_controller->selected_item_index,
         settings_controller->current_category);
}

void settings_controller_back(settings_controller_t *settings_controller) {
  if (settings_controller->settings_modified) {
    printf("Settings: Saving changes before returning to menu\n");
    settings_controller_save_settings(settings_controller);
  }

  if (settings_controller->app_controller) {
    app_controller_quit_to_menu(settings_controller->app_controller);
  }
}

void settings_controller_set_category(
  settings_controller_t *settings_controller, settings_category_t category) {
  if (settings_controller->current_category != category) {
    printf("Settings: Category changed from %d to %d\n",
           settings_controller->current_category, category);
    settings_controller->current_category = category;
    settings_controller->selected_item_index = 0; // Reset selection
  }
}

settings_category_t
settings_controller_get_category(settings_controller_t *settings_controller) {
  return settings_controller ? settings_controller->current_category
                             : SETTINGS_CATEGORY_GRAPHICS;
}

void settings_controller_toggle_setting(
  settings_controller_t *settings_controller, int item_index) {
  // TODO: Implement based on current category and item index
  settings_controller->settings_modified = true;
  printf("Settings: Toggled setting at index %d\n", item_index);
}

void settings_controller_adjust_slider(
  settings_controller_t *settings_controller, int item_index, float delta) {
  // TODO: Implement slider adjustment
  settings_controller->settings_modified = true;
  printf("Settings: Adjusted slider at index %d by %f\n", item_index, delta);
}

void settings_controller_cycle_dropdown(
  settings_controller_t *settings_controller, int item_index, int direction) {
  // TODO: Implement dropdown cycling
  settings_controller->settings_modified = true;
  printf("Settings: Cycled dropdown at index %d in direction %d\n", item_index,
         direction);
}

void settings_controller_save_settings(
  settings_controller_t *settings_controller) {
  if (!settings_controller) {
    return;
  }

  // TODO: Implement actual file saving
  // For now, just clear the modified flag
  settings_controller->settings_modified = false;
  printf("Settings: Saved to file\n");
}

void settings_controller_load_settings(
  settings_controller_t *settings_controller) {
  if (!settings_controller) {
    return;
  }

  // TODO: Implement actual file loading
  // For now, settings remain at their initialized defaults
  printf("Settings: Loaded from file (using defaults)\n");
}

void settings_controller_reset_to_defaults(
  settings_controller_t *settings_controller) {
  if (!settings_controller) {
    return;
  }

  // Reset all settings to defaults
  settings_controller->fullscreen = false;
  settings_controller->vsync = true;
  settings_controller->master_volume = 1.0f;
  settings_controller->sfx_volume = 0.8f;
  settings_controller->music_volume = 0.7f;
  settings_controller->show_grid = true;
  settings_controller->show_coordinates = false;
  settings_controller->camera_sensitivity = 50;

  settings_controller->settings_modified = true;

  printf("Settings: Reset to defaults\n");
}

int settings_controller_get_selected_item(
  settings_controller_t *settings_controller) {
  return settings_controller ? settings_controller->selected_item_index : 0;
}

bool settings_controller_has_unsaved_changes(
  settings_controller_t *settings_controller) {
  return settings_controller ? settings_controller->settings_modified : false;
}
