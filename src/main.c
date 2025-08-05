<<<<<<< HEAD
#include <stdbool.h>
#include <stdio.h>

#include "controller/input_state.h"
#include "raylib.h"
#include "ui/ui_context.h"
=======
#define RAYGUI_IMPLEMENTATION
#include "../include/third_party/raygui.h"
#include "controller/input_controller.h"
#include "raylib.h"
#include "render/renderer.h"
#include "screen/game_screen.h"
#include "screen/menu_screen.h"
#include "screen/pause_screen.h"
#include "screen/screen_manager.h"
#include <stdbool.h>
#include <stdio.h>
>>>>>>> parent of 53165fd (continued decoupling of raylib and game logic.)

int
main (void)
{
  // Window initialization
  const int initial_width = 800;
  const int initial_height = 600;

  SetConfigFlags (FLAG_WINDOW_HIGHDPI);
  SetConfigFlags (FLAG_WINDOW_RESIZABLE);
  SetTargetFPS (60);
  InitWindow (initial_width, initial_height, "HexHex");

  // Initialize Clay
  Clay_Initialize ((Clay_Arena){ 1024 * 1024 }, (Clay_Dimensions){ 1024 },
                   (Clay_ErrorHandler){});

<<<<<<< HEAD
  // Initialize UI context
  ui_context_t ui_ctx;
  ui_context_init (&ui_ctx);

  bool running = true;
=======
  input_controller_t input_ctrl;
  input_controller_init (&input_ctrl);

  // Initialize screen data
  menu_screen_t menu_screen;
  menu_screen_init (&menu_screen, initial_width, initial_height);

  game_screen_t game_screen;
  game_screen_init (&game_screen, initial_width, initial_height);

  pause_screen_t pause_screen;
  pause_screen_init (&pause_screen, initial_width, initial_height);

  // Register screens with input controller
  input_controller_register_screen (&input_ctrl, SCREEN_MENU,
                                    menu_input_handler, menu_action_handler,
                                    menu_render_handler, &menu_screen);

  input_controller_register_screen (&input_ctrl, SCREEN_GAME,
                                    game_input_handler, game_action_handler,
                                    game_render_handler, &game_screen);

  input_controller_register_screen (&input_ctrl, SCREEN_PAUSE,
                                    pause_input_handler, pause_action_handler,
                                    pause_render_handler, &pause_screen);

  // Set initial screen
  screen_manager_switch (&screen_mgr, SCREEN_MENU);
>>>>>>> parent of 53165fd (continued decoupling of raylib and game logic.)

  // Main game loop
  while (running && !WindowShouldClose ())
    {
<<<<<<< HEAD
      // Handle input
      input_state_t input;
      get_input_state (&input);

      // Update based on current screen
      switch (ui_ctx.current_screen)
        {
        case SCREEN_MENU:
          handle_menu_input (&ui_ctx, &input);
          break;
        case SCREEN_GAME:
          handle_game_input (&ui_ctx, &input);
          break;
        case SCREEN_SETTINGS:
          handle_settings_input (&ui_ctx, &input);
          break;
        }

      // Render
      BeginDrawing ();
      ClearBackground (WHITE);

      // Begin Clay frame
      Clay_BeginLayout ();

      // Render current screen
      switch (ui_ctx.current_screen)
        {
        case SCREEN_MENU:
          render_menu (&ui_ctx);
          break;
        case SCREEN_GAME:
          render_game (&ui_ctx);
          break;
        case SCREEN_SETTINGS:
          render_settings (&ui_ctx);
          break;
        }

      // End Clay frame and render
      Clay_RenderCommandArray clay_commands = Clay_EndLayout ();

      // TODO: Render Clay commands with Raylib renderer
      // For now, basic rendering will work
=======

      // Handle input and actions for current screen
      input_controller_update (&input_ctrl, &screen_mgr, &running);

      // Render current screen
      BeginDrawing ();
      ClearBackground (WHITE);

      input_controller_render (&input_ctrl, &screen_mgr);
>>>>>>> parent of 53165fd (continued decoupling of raylib and game logic.)

      EndDrawing ();
    }

<<<<<<< HEAD
  ui_context_cleanup (&ui_ctx);
=======
  // Cleanup
  game_screen_unload (&game_screen);
  menu_screen_unload (&menu_screen);
  pause_screen_unload (&pause_screen);
  input_controller_destroy (&input_ctrl);

>>>>>>> parent of 53165fd (continued decoupling of raylib and game logic.)
  CloseWindow ();
  return 0;
}
