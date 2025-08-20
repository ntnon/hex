#include "../include/game/input_state.h"
#include "../include/ui/clay_ui.h"
#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>

// Simple input state getter for this example
void
get_input_state_example (input_state_t *input)
{
  if (!input)
    return;

  // Clear previous state
  input->key_escape = false;
  input->key_enter = false;
  input->mouse_left_pressed = false;
  input->mouse_right_pressed = false;

  // Update with current frame input
  input->key_escape = IsKeyPressed (KEY_ESCAPE);
  input->key_enter = IsKeyPressed (KEY_ENTER);
  input->mouse_left_pressed = IsMouseButtonPressed (MOUSE_BUTTON_LEFT);
  input->mouse_right_pressed = IsMouseButtonPressed (MOUSE_BUTTON_RIGHT);

  input->mouse_x = GetMouseX ();
  input->mouse_y = GetMouseY ();
}

int
main (void)
{
  // Window setup
  const int initial_width = 1024;
  const int initial_height = 768;

  SetConfigFlags (FLAG_WINDOW_RESIZABLE);
  InitWindow (initial_width, initial_height, "HexHex - Clay UI Example");
  SetTargetFPS (60);

  // Initialize Clay UI system
  clay_ui_t ui;
  if (!clay_ui_init (&ui, initial_width, initial_height))
    {
      printf ("Failed to initialize Clay UI\n");
      CloseWindow ();
      return -1;
    }

  // Main loop
  while (!WindowShouldClose () && !clay_ui_should_quit (&ui))
    {
      // Handle window resize
      if (IsWindowResized ())
        {
          int width = GetScreenWidth ();
          int height = GetScreenHeight ();
          clay_ui_resize (&ui, width, height);
        }

      // Get input state
      input_state_t input;
      get_input_state_example (&input);

      // Update Clay pointer state
      Clay_SetPointerState ((Clay_Vector2){ GetMouseX (), GetMouseY () },
                            IsMouseButtonDown (MOUSE_BUTTON_LEFT));

      // Update UI
      clay_ui_handle_input (&ui, &input);

      // Render frame
      BeginDrawing ();
      ClearBackground (RAYWHITE);

      // Render UI
      clay_ui_render (&ui);

      // Optional: Draw FPS
      DrawFPS (10, 10);

      EndDrawing ();
    }

  // Cleanup
  clay_ui_cleanup (&ui);
  CloseWindow ();

  return 0;
}
