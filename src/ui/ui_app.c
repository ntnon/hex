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

void ui_main_menu(app_controller_t *app_controller) {
    CLAY(ID_MAIN_MENU, {.layout = {.sizing = {.width = CLAY_SIZING_GROW(),
                                              .height = CLAY_SIZING_GROW()},
                                   .childAlignment = {.x = CLAY_ALIGN_X_CENTER,
                                                      .y = CLAY_ALIGN_Y_CENTER},
                                   .layoutDirection = CLAY_TOP_TO_BOTTOM,
                                   .childGap = MENU_BUTTON_GAP},
                        .backgroundColor = M_DARKBLUE}) {

        // Game title
        CLAY_AUTO_ID({.layout = {.sizing = {.width = CLAY_SIZING_FIT(),
                                            .height = CLAY_SIZING_FIT()},
                                 .padding = {0, 0, 50, 0}}}) {
            CLAY_TEXT(CLAY_STRING("HexHex Game"), &TEXT_CONFIG_LARGE);
        }

        // Menu buttons container
        CLAY_AUTO_ID(
          {.layout = {.layoutDirection = CLAY_TOP_TO_BOTTOM,
                      .childGap = MENU_BUTTON_GAP,
                      .sizing = {.width = CLAY_SIZING_FIXED(MENU_BUTTON_WIDTH),
                                 .height = CLAY_SIZING_FIT()}}}) {

            // New Game button
            CLAY(ID_MENU_ITEM_NEW_GAME,
                 {.layout = {.sizing = {.width = CLAY_SIZING_GROW(),
                                        .height = CLAY_SIZING_FIXED(
                                          MENU_BUTTON_HEIGHT)},
                             .childAlignment = {.x = CLAY_ALIGN_X_CENTER,
                                                .y = CLAY_ALIGN_Y_CENTER}},
                  .backgroundColor =
                    app_controller->selected_menu_item == 0 ? M_ORANGE : M_GRAY,
                  .cornerRadius = CLAY_CORNER_RADIUS(5)}) {
                Clay_OnHover(handle_menu_button_hover,
                             (intptr_t)app_controller);
                CLAY_TEXT(CLAY_STRING("New Game"), &TEXT_CONFIG_MEDIUM);
            }

            // Settings button
            CLAY(ID_MENU_ITEM_SETTINGS,
                 {.layout = {.sizing = {.width = CLAY_SIZING_GROW(),
                                        .height = CLAY_SIZING_FIXED(
                                          MENU_BUTTON_HEIGHT)},
                             .childAlignment = {.x = CLAY_ALIGN_X_CENTER,
                                                .y = CLAY_ALIGN_Y_CENTER}},
                  .backgroundColor =
                    app_controller->selected_menu_item == 1 ? M_ORANGE : M_GRAY,
                  .cornerRadius = CLAY_CORNER_RADIUS(5)}) {
                Clay_OnHover(handle_menu_button_hover,
                             (intptr_t)app_controller);
                CLAY_TEXT(CLAY_STRING("Settings"), &TEXT_CONFIG_MEDIUM);
            }

            // Quit button
            CLAY(ID_MENU_ITEM_QUIT,
                 {.layout = {.sizing = {.width = CLAY_SIZING_GROW(),
                                        .height = CLAY_SIZING_FIXED(
                                          MENU_BUTTON_HEIGHT)},
                             .childAlignment = {.x = CLAY_ALIGN_X_CENTER,
                                                .y = CLAY_ALIGN_Y_CENTER}},
                  .backgroundColor =
                    app_controller->selected_menu_item == 2 ? M_ORANGE : M_GRAY,
                  .cornerRadius = CLAY_CORNER_RADIUS(5)}) {
                Clay_OnHover(handle_menu_button_hover,
                             (intptr_t)app_controller);
                CLAY_TEXT(CLAY_STRING("Quit"), &TEXT_CONFIG_MEDIUM);
            }
        }

        // Instructions
        CLAY_AUTO_ID({.layout = {.sizing = {.width = CLAY_SIZING_FIT(),
                                            .height = CLAY_SIZING_FIT()},
                                 .padding = {0, 50, 0, 0}}}) {
            CLAY_TEXT(
              CLAY_STRING("Use arrow keys or mouse to navigate, Enter/Click "
                          "to select, ESC to quit"),
              &TEXT_CONFIG_MEDIUM);
        }
    }
}

void ui_settings_menu(app_controller_t *app_controller) {
    CLAY(ID_SETTINGS_MENU,
         {.layout = {.sizing = {.width = CLAY_SIZING_GROW(),
                                .height = CLAY_SIZING_GROW()},
                     .childAlignment = {.x = CLAY_ALIGN_X_CENTER,
                                        .y = CLAY_ALIGN_Y_CENTER},
                     .layoutDirection = CLAY_TOP_TO_BOTTOM,
                     .childGap = 30},
          .backgroundColor = M_DARKGRAY}) {

        // Settings title
        CLAY_AUTO_ID({.layout = {.sizing = {.width = CLAY_SIZING_FIT(),
                                            .height = CLAY_SIZING_FIT()},
                                 .padding = {0, 0, 30, 0}}}) {
            CLAY_TEXT(CLAY_STRING("Settings"), &TEXT_CONFIG_LARGE);
        }

        // Settings content placeholder
        CLAY_AUTO_ID({.layout = {.sizing = {.width = CLAY_SIZING_FIXED(400),
                                            .height = CLAY_SIZING_FIT()},
                                 .layoutDirection = CLAY_TOP_TO_BOTTOM,
                                 .childGap = 15,
                                 .padding = CLAY_PADDING_ALL(20)},
                      .backgroundColor = M_LIGHTGRAY,
                      .cornerRadius = CLAY_CORNER_RADIUS(5)}) {

            CLAY_AUTO_ID(
              {.layout = {.sizing = {.width = CLAY_SIZING_GROW(),
                                     .height = CLAY_SIZING_FIT()},
                          .childAlignment = {.x = CLAY_ALIGN_X_CENTER},
                          .padding = {10, 0, 0, 0}}}) {
                CLAY_TEXT(CLAY_STRING("Settings functionality coming soon!"),
                          &TEXT_CONFIG_MEDIUM);
            }
        }

        // Back button
        CLAY(
          ID_SETTINGS_BACK_BUTTON,
          {.layout = {.sizing = {.width = CLAY_SIZING_FIXED(MENU_BUTTON_WIDTH),
                                 .height =
                                   CLAY_SIZING_FIXED(MENU_BUTTON_HEIGHT)},
                      .childAlignment = {.x = CLAY_ALIGN_X_CENTER,
                                         .y = CLAY_ALIGN_Y_CENTER}},
           .backgroundColor = M_ORANGE,
           .cornerRadius = CLAY_CORNER_RADIUS(5)}) {
            Clay_OnHover(handle_menu_button_hover, (intptr_t)app_controller);
            CLAY_TEXT(CLAY_STRING("Back to Menu"), &TEXT_CONFIG_MEDIUM);
        }

        // Instructions
        CLAY_AUTO_ID({.layout = {.sizing = {.width = CLAY_SIZING_FIT(),
                                            .height = CLAY_SIZING_FIT()},
                                 .padding = {30, 0, 0, 0}}}) {
            CLAY_TEXT(CLAY_STRING("ESC to return to main menu"),
                      &TEXT_CONFIG_MEDIUM);
        }
    }
}

void ui_pause_menu(app_controller_t *app_controller) {
    CLAY(ID_PAUSE_MENU,
         {.layout = {.sizing = {.width = CLAY_SIZING_GROW(),
                                .height = CLAY_SIZING_GROW()},
                     .childAlignment = {.x = CLAY_ALIGN_X_CENTER,
                                        .y = CLAY_ALIGN_Y_CENTER},
                     .layoutDirection = CLAY_TOP_TO_BOTTOM,
                     .childGap = MENU_BUTTON_GAP},
          .backgroundColor = {0, 0, 0, 180}}) {

        // Pause menu container
        CLAY_AUTO_ID({.layout = {.sizing = {.width = CLAY_SIZING_FIXED(300),
                                            .height = CLAY_SIZING_FIT()},
                                 .layoutDirection = CLAY_TOP_TO_BOTTOM,
                                 .childGap = MENU_BUTTON_GAP,
                                 .padding = CLAY_PADDING_ALL(30)},
                      .backgroundColor = M_DARKGRAY,
                      .cornerRadius = CLAY_CORNER_RADIUS(10)}) {

            // Pause title
            CLAY_AUTO_ID({.layout = {.sizing = {.width = CLAY_SIZING_FIT(),
                                                .height = CLAY_SIZING_FIT()},
                                     .padding = {0, 0, 20, 0}}}) {
                CLAY_TEXT(CLAY_STRING("Game Paused"), &TEXT_CONFIG_LARGE);
            }

            // Resume button
            CLAY(ID_PAUSE_RESUME_BUTTON,
                 {.layout = {.sizing = {.width = CLAY_SIZING_GROW(),
                                        .height = CLAY_SIZING_FIXED(
                                          MENU_BUTTON_HEIGHT)},
                             .childAlignment = {.x = CLAY_ALIGN_X_CENTER,
                                                .y = CLAY_ALIGN_Y_CENTER}},
                  .backgroundColor = M_GREEN,
                  .cornerRadius = CLAY_CORNER_RADIUS(5)}) {
                Clay_OnHover(handle_menu_button_hover,
                             (intptr_t)app_controller);
                CLAY_TEXT(CLAY_STRING("Resume"), &TEXT_CONFIG_MEDIUM);
            }

            // Settings button
            CLAY(ID_PAUSE_SETTINGS_BUTTON,
                 {.layout = {.sizing = {.width = CLAY_SIZING_GROW(),
                                        .height = CLAY_SIZING_FIXED(
                                          MENU_BUTTON_HEIGHT)},
                             .childAlignment = {.x = CLAY_ALIGN_X_CENTER,
                                                .y = CLAY_ALIGN_Y_CENTER}},
                  .backgroundColor = M_GRAY,
                  .cornerRadius = CLAY_CORNER_RADIUS(5)}) {
                Clay_OnHover(handle_menu_button_hover,
                             (intptr_t)app_controller);
                CLAY_TEXT(CLAY_STRING("Settings"), &TEXT_CONFIG_MEDIUM);
            }

            // Quit to menu button
            CLAY(ID_PAUSE_QUIT_BUTTON,
                 {.layout = {.sizing = {.width = CLAY_SIZING_GROW(),
                                        .height = CLAY_SIZING_FIXED(
                                          MENU_BUTTON_HEIGHT)},
                             .childAlignment = {.x = CLAY_ALIGN_X_CENTER,
                                                .y = CLAY_ALIGN_Y_CENTER}},
                  .backgroundColor = M_RED,
                  .cornerRadius = CLAY_CORNER_RADIUS(5)}) {
                Clay_OnHover(handle_menu_button_hover,
                             (intptr_t)app_controller);
                CLAY_TEXT(CLAY_STRING("Quit to Menu"), &TEXT_CONFIG_MEDIUM);
            }

            // Instructions
            CLAY_AUTO_ID({.layout = {.sizing = {.width = CLAY_SIZING_FIT(),
                                                .height = CLAY_SIZING_FIT()},
                                     .padding = {30, 0, 0, 0}}}) {
                CLAY_TEXT(CLAY_STRING("ESC to return to main menu"),
                          &TEXT_CONFIG_MEDIUM);
            }
        }
    }
}

Clay_RenderCommandArray ui_root(app_controller_t *app_controller,
                                const input_state_t *input) {
    // Set pointer state BEFORE layout begins so hover callbacks work
    Clay_SetPointerState((Clay_Vector2){input->mouse.x, input->mouse.y},
                         input->mouse_left_down);

    Clay_SetLayoutDimensions((Clay_Dimensions){.width = GetScreenWidth(),
                                               .height = GetScreenHeight()});
    Clay_BeginLayout();

    CLAY(ID_ROOT, {.layout = {
                     .sizing = (Clay_Sizing){.width = CLAY_SIZING_GROW(),
                                             .height = CLAY_SIZING_GROW()},
                   }}) {
        switch (app_controller->current_state) {
        case APP_STATE_MAIN_MENU:
            ui_main_menu(app_controller);
            break;
        case APP_STATE_GAME:
            if (app_controller->game) {
                ui_game(&app_controller->game_controller, input);
            }
            break;
        case APP_STATE_SETTINGS:
            ui_settings_menu(app_controller);
            break;
        default:
            printf("no app state found");
            break;
        }
    }
    Clay_RenderCommandArray commands = Clay_EndLayout();
    return commands;
}
// Helper function implementations
// Removed problematic ui_menu_button functions that caused memory
// corruption
