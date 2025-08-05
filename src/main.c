#include <stdbool.h>
#include <stdio.h>

#include "raylib.h"
#include "render/renderer.h"
#include "screen/game_screen.h"
#include "screen/menu_screen.h"
#include "screen/screen_manager.h"
#include "screen/settings_screen.h"
#include "ui/ui_context.h"

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
  ui_context_t ui_ctx;
  ui_context_init (&ui_ctx);

  screen_manager_t screen_mgr;
  screen_manager_init (&screen_mgr, &ui_ctx);

  // Initialize screen data
  menu_screen_t menu_screen;
  menu_screen_init (&menu_screen, initial_width, initial_height);

  game_screen_t game_screen;
  game_screen_init (&game_screen, initial_width, initial_height);

  settings_screen_t settings_screen;
  settings_screen_init (&settings_screen, initial_width, initial_height);

  screen_manager_register (
      &screen_mgr, SCREEN_MENU,
      (screen_callbacks_t){ .screen_data = &menu_screen,
                            .input_handler = menu_input_handler,
                            .action_handler = menu_action_handler,
                            .render_handler = menu_render_handler,
                            .unload_handler = menu_screen_unload,
                            .registered = true });

  screen_manager_register (
      &screen_mgr, SCREEN_GAME,
      (screen_callbacks_t){ .screen_data = &game_screen,
                            .input_handler = game_input_handler,
                            .action_handler = game_action_handler,
                            .render_handler = game_render_handler,
                            .unload_handler = game_screen_unload,
                            .registered = true });

  // screen_manager_register (
  //     &screen_mgr, SCREEN_SETTINGS,
  //     (screen_callbacks_t){ .screen_data = &settings_screen,
  //                           .input_handler = settings_input_handler,
  //                           .action_handler = settings_action_handler,
  //                           .render_handler = settings_render_handler,
  //                           .unload_handler = settings_screen_unload,
  //                           .registered = true});

  // Set initial screen
  screen_manager_switch (&screen_mgr, SCREEN_MENU);

  // Main game loop
  bool running = true;
  while (running && !WindowShouldClose ())
    {
      input_state_t input;
      get_input_state (&input);

      screen_callbacks_t *screen = &screen_mgr.screens[screen_mgr.current];

      if (screen->input_handler)
        screen->input_handler (screen->screen_data, &input);

      if (screen->action_handler)
        screen->action_handler (screen->screen_data, &screen_mgr, &running);

      BeginDrawing ();
      ClearBackground (WHITE);

      ui_context_begin_frame (&ui_ctx);

      if (screen->render_handler)
        screen->render_handler (screen->screen_data);

      // Render UI using screen handlers
      ui_screen_handlers_t *handlers = &ui_ctx.handlers[screen_mgr.current];
      if (handlers->render)
        {
          handlers->render (&ui_ctx);
        }

      ui_context_end_frame (&ui_ctx);
      EndDrawing ();
    }

  screen_manager_cleanup (&screen_mgr);
  ui_context_cleanup (&ui_ctx);

  CloseWindow ();
  return 0;
}
