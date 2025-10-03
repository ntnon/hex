#include "controller/app_controller.h"
#include "tile/pool.h"
#include "ui.h"
#include "ui_types.h"
#include <stdio.h>
#include <string.h>

// UI element IDs are now defined in ui_types.h

// Layout constants
#define MENU_BUTTON_WIDTH 200
#define MENU_BUTTON_HEIGHT 50
#define MENU_BUTTON_GAP 20
#define MENU_PADDING 40

// Helper functions

void ui_build_main_menu(app_controller_t *app_controller) {
  CLAY({.id = UI_ID_MAIN_MENU,
        .layout = {.sizing = {.width = CLAY_SIZING_GROW(),
                              .height = CLAY_SIZING_GROW()},
                   .childAlignment = {.x = CLAY_ALIGN_X_CENTER,
                                      .y = CLAY_ALIGN_Y_CENTER},
                   .layoutDirection = CLAY_TOP_TO_BOTTOM,
                   .childGap = MENU_BUTTON_GAP},
        .backgroundColor = M_DARKBLUE}) {

    // Game title
    CLAY({.layout = {
            .sizing = {.width = CLAY_SIZING_FIT(), .height = CLAY_SIZING_FIT()},
            .padding = {0, 0, 50, 0}}}) {
      CLAY_TEXT(CLAY_STRING("HexHex Game"), &TEXT_CONFIG_LARGE);
    }

    // Menu buttons container
    CLAY({.layout = {.layoutDirection = CLAY_TOP_TO_BOTTOM,
                     .childGap = MENU_BUTTON_GAP,
                     .sizing = {.width = CLAY_SIZING_FIXED(MENU_BUTTON_WIDTH),
                                .height = CLAY_SIZING_FIT()}}}) {

      // New Game button
      CLAY(
        {.id = UI_ID_MENU_ITEM_NEW_GAME,
         .layout = {.sizing = {.width = CLAY_SIZING_GROW(),
                               .height = CLAY_SIZING_FIXED(MENU_BUTTON_HEIGHT)},
                    .childAlignment = {.x = CLAY_ALIGN_X_CENTER,
                                       .y = CLAY_ALIGN_Y_CENTER}},
         .backgroundColor =
           app_controller->selected_menu_item == 0 ? M_ORANGE : M_GRAY,
         .cornerRadius = CLAY_CORNER_RADIUS(5)}) {
        Clay_OnHover(handle_menu_button_hover, (intptr_t)app_controller);
        CLAY_TEXT(CLAY_STRING("New Game"), &TEXT_CONFIG_MEDIUM);
      }

      // Settings button
      CLAY(
        {.id = UI_ID_MENU_ITEM_SETTINGS,
         .layout = {.sizing = {.width = CLAY_SIZING_GROW(),
                               .height = CLAY_SIZING_FIXED(MENU_BUTTON_HEIGHT)},
                    .childAlignment = {.x = CLAY_ALIGN_X_CENTER,
                                       .y = CLAY_ALIGN_Y_CENTER}},
         .backgroundColor =
           app_controller->selected_menu_item == 1 ? M_ORANGE : M_GRAY,
         .cornerRadius = CLAY_CORNER_RADIUS(5)}) {
        Clay_OnHover(handle_menu_button_hover, (intptr_t)app_controller);
        CLAY_TEXT(CLAY_STRING("Settings"), &TEXT_CONFIG_MEDIUM);
      }

      // Quit button
      CLAY(
        {.id = UI_ID_MENU_ITEM_QUIT,
         .layout = {.sizing = {.width = CLAY_SIZING_GROW(),
                               .height = CLAY_SIZING_FIXED(MENU_BUTTON_HEIGHT)},
                    .childAlignment = {.x = CLAY_ALIGN_X_CENTER,
                                       .y = CLAY_ALIGN_Y_CENTER}},
         .backgroundColor =
           app_controller->selected_menu_item == 2 ? M_ORANGE : M_GRAY,
         .cornerRadius = CLAY_CORNER_RADIUS(5)}) {
        Clay_OnHover(handle_menu_button_hover, (intptr_t)app_controller);
        CLAY_TEXT(CLAY_STRING("Quit"), &TEXT_CONFIG_MEDIUM);
      }
    }

    // Instructions
    CLAY({.layout = {
            .sizing = {.width = CLAY_SIZING_FIT(), .height = CLAY_SIZING_FIT()},
            .padding = {0, 50, 0, 0}}}) {
      CLAY_TEXT(CLAY_STRING("Use arrow keys or mouse to navigate, Enter/Click "
                            "to select, ESC to quit"),
                &TEXT_CONFIG_MEDIUM);
    }
  }
}

void ui_build_settings_menu(app_controller_t *app_controller) {
  CLAY({.id = UI_ID_SETTINGS_MENU,
        .layout = {.sizing = {.width = CLAY_SIZING_GROW(),
                              .height = CLAY_SIZING_GROW()},
                   .childAlignment = {.x = CLAY_ALIGN_X_CENTER,
                                      .y = CLAY_ALIGN_Y_CENTER},
                   .layoutDirection = CLAY_TOP_TO_BOTTOM,
                   .childGap = 30},
        .backgroundColor = M_DARKGRAY}) {

    // Settings title
    CLAY({.layout = {
            .sizing = {.width = CLAY_SIZING_FIT(), .height = CLAY_SIZING_FIT()},
            .padding = {0, 0, 30, 0}}}) {
      CLAY_TEXT(CLAY_STRING("Settings"), &TEXT_CONFIG_LARGE);
    }

    // Settings content placeholder
    CLAY({.layout = {.sizing = {.width = CLAY_SIZING_FIXED(400),
                                .height = CLAY_SIZING_FIT()},
                     .layoutDirection = CLAY_TOP_TO_BOTTOM,
                     .childGap = 15,
                     .padding = CLAY_PADDING_ALL(20)},
          .backgroundColor = M_LIGHTGRAY,
          .cornerRadius = CLAY_CORNER_RADIUS(5)}) {

      CLAY({.layout = {.sizing = {.width = CLAY_SIZING_GROW(),
                                  .height = CLAY_SIZING_FIT()},
                       .childAlignment = {.x = CLAY_ALIGN_X_CENTER},
                       .padding = {10, 0, 0, 0}}}) {
        CLAY_TEXT(CLAY_STRING("Settings functionality coming soon!"),
                  &TEXT_CONFIG_MEDIUM);
      }
    }

    // Back button
    CLAY(
      {.id = UI_ID_SETTINGS_BACK_BUTTON,
       .layout = {.sizing = {.width = CLAY_SIZING_FIXED(MENU_BUTTON_WIDTH),
                             .height = CLAY_SIZING_FIXED(MENU_BUTTON_HEIGHT)},
                  .childAlignment = {.x = CLAY_ALIGN_X_CENTER,
                                     .y = CLAY_ALIGN_Y_CENTER}},
       .backgroundColor = M_ORANGE,
       .cornerRadius = CLAY_CORNER_RADIUS(5)}) {
      Clay_OnHover(handle_menu_button_hover, (intptr_t)app_controller);
      CLAY_TEXT(CLAY_STRING("Back to Menu"), &TEXT_CONFIG_MEDIUM);
    }

    // Instructions
    CLAY({.layout = {
            .sizing = {.width = CLAY_SIZING_FIT(), .height = CLAY_SIZING_FIT()},
            .padding = {30, 0, 0, 0}}}) {
      CLAY_TEXT(CLAY_STRING("ESC to return to main menu"), &TEXT_CONFIG_MEDIUM);
    }
  }
}

void ui_build_pause_menu(app_controller_t *app_controller) {
  CLAY({.id = UI_ID_PAUSE_MENU,
        .layout = {.sizing = {.width = CLAY_SIZING_GROW(),
                              .height = CLAY_SIZING_GROW()},
                   .childAlignment = {.x = CLAY_ALIGN_X_CENTER,
                                      .y = CLAY_ALIGN_Y_CENTER},
                   .layoutDirection = CLAY_TOP_TO_BOTTOM,
                   .childGap = MENU_BUTTON_GAP},
        .backgroundColor = {0, 0, 0, 180}}) {

    // Pause menu container
    CLAY({.layout = {.sizing = {.width = CLAY_SIZING_FIXED(300),
                                .height = CLAY_SIZING_FIT()},
                     .layoutDirection = CLAY_TOP_TO_BOTTOM,
                     .childGap = MENU_BUTTON_GAP,
                     .padding = CLAY_PADDING_ALL(30)},
          .backgroundColor = M_DARKGRAY,
          .cornerRadius = CLAY_CORNER_RADIUS(10)}) {

      // Pause title
      CLAY({.layout = {.sizing = {.width = CLAY_SIZING_FIT(),
                                  .height = CLAY_SIZING_FIT()},
                       .padding = {0, 0, 20, 0}}}) {
        CLAY_TEXT(CLAY_STRING("Game Paused"), &TEXT_CONFIG_LARGE);
      }

      // Resume button
      CLAY(
        {.id = UI_ID_PAUSE_RESUME_BUTTON,
         .layout = {.sizing = {.width = CLAY_SIZING_GROW(),
                               .height = CLAY_SIZING_FIXED(MENU_BUTTON_HEIGHT)},
                    .childAlignment = {.x = CLAY_ALIGN_X_CENTER,
                                       .y = CLAY_ALIGN_Y_CENTER}},
         .backgroundColor = M_GREEN,
         .cornerRadius = CLAY_CORNER_RADIUS(5)}) {
        Clay_OnHover(handle_menu_button_hover, (intptr_t)app_controller);
        CLAY_TEXT(CLAY_STRING("Resume"), &TEXT_CONFIG_MEDIUM);
      }

      // Settings button
      CLAY(
        {.id = UI_ID_PAUSE_SETTINGS_BUTTON,
         .layout = {.sizing = {.width = CLAY_SIZING_GROW(),
                               .height = CLAY_SIZING_FIXED(MENU_BUTTON_HEIGHT)},
                    .childAlignment = {.x = CLAY_ALIGN_X_CENTER,
                                       .y = CLAY_ALIGN_Y_CENTER}},
         .backgroundColor = M_GRAY,
         .cornerRadius = CLAY_CORNER_RADIUS(5)}) {
        Clay_OnHover(handle_menu_button_hover, (intptr_t)app_controller);
        CLAY_TEXT(CLAY_STRING("Settings"), &TEXT_CONFIG_MEDIUM);
      }

      // Quit to menu button
      CLAY(
        {.id = UI_ID_PAUSE_QUIT_BUTTON,
         .layout = {.sizing = {.width = CLAY_SIZING_GROW(),
                               .height = CLAY_SIZING_FIXED(MENU_BUTTON_HEIGHT)},
                    .childAlignment = {.x = CLAY_ALIGN_X_CENTER,
                                       .y = CLAY_ALIGN_Y_CENTER}},
         .backgroundColor = M_RED,
         .cornerRadius = CLAY_CORNER_RADIUS(5)}) {
        Clay_OnHover(handle_menu_button_hover, (intptr_t)app_controller);
        CLAY_TEXT(CLAY_STRING("Quit to Menu"), &TEXT_CONFIG_MEDIUM);
      }

      // Instructions
      CLAY({.layout = {.sizing = {.width = CLAY_SIZING_FIT(),
                                  .height = CLAY_SIZING_FIT()},
                       .padding = {30, 0, 0, 0}}}) {
        CLAY_TEXT(CLAY_STRING("ESC to return to main menu"),
                  &TEXT_CONFIG_MEDIUM);
      }
    }
  }
}

void ui_build_game_ui(app_controller_t *app_controller) {
  // The game UI is handled in the main ui_build_layout function
  // This is a placeholder for any game-specific overlay UI
}

void ui_build_tile_info_card(game_t *game, Vector2 mouse_pos) {
  if (!game->should_show_tile_info || !game->hovered_tile) {
    return;
  }

  tile_t *tile = game->hovered_tile;
  pool_t *pool = pool_map_get_pool_by_tile(game->board->pools, tile);
  int score = pool_tile_score(pool);
  // Position the info card near the mouse, but keep it on screen
  float card_width = 200;
  float card_height = 120;
  float screen_width = GetScreenWidth();
  float screen_height = GetScreenHeight();

  // Default position: offset from mouse
  float card_x = mouse_pos.x + 20;
  float card_y = mouse_pos.y - card_height / 2;

  // Adjust if card would go off screen
  if (card_x + card_width > screen_width) {
    card_x = mouse_pos.x - card_width - 20;
  }
  if (card_y < 0) {
    card_y = 10;
  } else if (card_y + card_height > screen_height) {
    card_y = screen_height - card_height - 10;
  }

  CLAY({
    .id = UI_ID_TILE_INFO_CARD,
    .floating = {.attachTo = CLAY_ATTACH_TO_ROOT,
                 .offset = {.x = (int)card_x, .y = (int)card_y},
                 .zIndex = 1000,
                 .pointerCaptureMode = CLAY_POINTER_CAPTURE_MODE_PASSTHROUGH},
    .layout = {.sizing = {.width = CLAY_SIZING_FIT(),
                          .height = CLAY_SIZING_FIT()},
               .padding = CLAY_PADDING_ALL(12),
               .childGap = 8,
               .layoutDirection = CLAY_TOP_TO_BOTTOM},
  }) {

    // Tile card
    CLAY({.layout = {.sizing = {.width = CLAY_SIZING_GROW(),
                                .height = CLAY_SIZING_GROW()},
                     .padding = CLAY_PADDING_ALL(12),
                     .childGap = 8,
                     .layoutDirection = CLAY_TOP_TO_BOTTOM},

          .backgroundColor = (Clay_Color){40, 40, 40, 240},
          .cornerRadius = CLAY_CORNER_RADIUS(6),
          .border = {.color = (Clay_Color){80, 80, 80, 255}, .width = 1}}) {
      switch (tile->data.type) {
      case TILE_MAGENTA:
        CLAY_TEXT(CLAY_STRING("Magenta tile"), &TEXT_CONFIG_MEDIUM);
        break;
      case TILE_CYAN:
        CLAY_TEXT(CLAY_STRING("Cyan tile"), &TEXT_CONFIG_MEDIUM);
        break;
      case TILE_YELLOW:
        CLAY_TEXT(CLAY_STRING("Yellow tile"), &TEXT_CONFIG_MEDIUM);
        break;
      case TILE_GREEN:
        CLAY_TEXT(CLAY_STRING("Green tile"), &TEXT_CONFIG_MEDIUM);
        break;
      default:
        CLAY_TEXT(CLAY_STRING("Unknown tile"), &TEXT_CONFIG_MEDIUM);
        break;
      }

      static char value_text[32];
      snprintf(value_text, sizeof(value_text), "Value: %d", tile->data.value);
      Clay_String value_string = {.chars = value_text,
                                  .length = strlen(value_text)};
      CLAY_TEXT(value_string, &TEXT_CONFIG_MEDIUM);

      static char tile_modifier_text[32];
      snprintf(tile_modifier_text, sizeof(tile_modifier_text), "Modifier: %.2f",
               tile_get_modifier(tile));
      Clay_String tile_modifier = {.chars = tile_modifier_text,
                                   .length = strlen(tile_modifier_text)};
      CLAY_TEXT(tile_modifier, &TEXT_CONFIG_MEDIUM);

      static char tile_range_text[32];
      snprintf(tile_range_text, sizeof(tile_range_text), "Range: %d",
               tile_get_range(tile));
      Clay_String tile_range = {.chars = tile_range_text,
                                .length = strlen(tile_modifier_text)};
      CLAY_TEXT(tile_range, &TEXT_CONFIG_MEDIUM);
    }

    // Pool card
    CLAY({.layout = {.sizing = {.width = CLAY_SIZING_GROW(),
                                .height = CLAY_SIZING_GROW()},
                     .padding = CLAY_PADDING_ALL(12),
                     .childGap = 8,
                     .layoutDirection = CLAY_TOP_TO_BOTTOM},

          .backgroundColor = (Clay_Color){40, 40, 40, 240},
          .cornerRadius = CLAY_CORNER_RADIUS(6),
          .border = {.color = (Clay_Color){80, 80, 80, 255}, .width = 1}}) {
      CLAY_TEXT(CLAY_STRING("Pool"), &TEXT_CONFIG_MEDIUM);
      static char pool_text[32];
      snprintf(pool_text, sizeof(pool_text), "Tiles: %u", score);
      Clay_String pool_score = {.chars = pool_text,
                                .length = strlen(pool_text)};
      CLAY_TEXT(pool_score, &TEXT_CONFIG_MEDIUM);

      static char pool_modifier_text[32];
      snprintf(pool_modifier_text, sizeof(pool_text), "Modifier: %.2f",
               pool_get_modifier(pool));
      Clay_String pool_modifier = {.chars = pool_modifier_text,
                                   .length = strlen(pool_modifier_text)};
      CLAY_TEXT(pool_modifier, &TEXT_CONFIG_MEDIUM);

      static char pool_range_text[32];
      snprintf(pool_range_text, sizeof(pool_text), "Range: %d",
               pool_get_range(pool));
      Clay_String pool_range = {.chars = pool_range_text,
                                .length = strlen(pool_modifier_text)};
      CLAY_TEXT(pool_range, &TEXT_CONFIG_MEDIUM);
    }
  }
}

// Helper function implementations
// Removed problematic ui_build_menu_button functions that caused memory
// corruption
