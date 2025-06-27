#include "game_board.h"
#include "raylib.h"
#include "raymath.h"

int
main (void)
{
  // Initialization
  const int screenWidth = 800;
  const int screenHeight = 450;
  SetConfigFlags (FLAG_WINDOW_HIGHDPI);
  InitWindow (screenWidth, screenHeight, "hex Grid - Edge Highlighting Demo");

  Camera2D camera = { 0 };
  camera.target = (Vector2){ 0, 0 };
  camera.offset = (Vector2){ screenWidth / 2.0f, screenHeight / 2.0f };
  camera.zoom = 1.0f;
  camera.rotation = 0.0f;

  // Simple layout test
  game_board *board = game_board_new (6);

  SetTargetFPS (60);

  // Main game loop
  while (!WindowShouldClose ())
    {
      // Update
      // Pan with right mouse button
      if (IsMouseButtonDown (MOUSE_BUTTON_RIGHT)
          || IsMouseButtonDown (MOUSE_BUTTON_LEFT))
        {
          Vector2 delta = GetMouseDelta ();
          delta = Vector2Scale (delta, -1.0f / camera.zoom);
          camera.target = Vector2Add (camera.target, delta);
        }

      // Zoom with mouse wheel
      float wheel = GetMouseWheelMove ();
      if (wheel != 0)
        {
          float scale = 0.1f * wheel;
          camera.zoom = Clamp (expf (logf (camera.zoom) + scale), 0.1f, 10.0f);
        }

      // Draw
      BeginDrawing ();
      ClearBackground (RAYWHITE);

      BeginMode2D (camera);

      game_board_draw (board);

      EndMode2D ();
      EndDrawing ();
    }

  // Cleanup
  game_board_free (board);
  CloseWindow ();

  return 0;
}
