#include <stdbool.h>
#include <stdio.h>

#include "controller/input_state.h"
#include "raylib.h"
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

  // Initialize Clay
  Clay_Initialize ((Clay_Arena){ 1024 * 1024 }, (Clay_Dimensions){ 1024 },
                   (Clay_ErrorHandler){});

  // Initialize UI context
  ui_context_t ui_ctx;
  ui_context_init (&ui_ctx);

  bool running = true;

  // Main game loop
  while (running && !WindowShouldClose ())
    {
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

      EndDrawing ();
    }

  ui_context_cleanup (&ui_ctx);
  CloseWindow ();
  return 0;
}
