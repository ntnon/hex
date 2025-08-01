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

  // Initialize core systems
  screen_manager_t screen_mgr;
  screen_manager_init (&screen_mgr);

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

  // Main game loop
  bool running = true;
  while (running && !WindowShouldClose ())
    {

      // Handle input and actions for current screen
      input_controller_update (&input_ctrl, &screen_mgr, &running);

      // Render current screen
      BeginDrawing ();
      ClearBackground (WHITE);

      input_controller_render (&input_ctrl, &screen_mgr);

      EndDrawing ();
    }

  // Cleanup
  game_screen_unload (&game_screen);
  menu_screen_unload (&menu_screen);
  pause_screen_unload (&pause_screen);
  input_controller_destroy (&input_ctrl);

  CloseWindow ();
  return 0;
}
