#include "board/board.h"
#include "raylib.h"
#include <stdio.h>
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
  camera.zoom = 0.1f;
  camera.rotation = 0.0f;

  // Simple layout test

  SetTargetFPS (5);

  board_t *board = board_create ();
  randomize_board (board);

  // Main game loop
  while (!WindowShouldClose ())
    {
      // Draw
      BeginDrawing ();
      ClearBackground (RAYWHITE);

      BeginMode2D (camera);
      // game_controller_draw (game_controller);

      board_draw (board);

      EndMode2D ();

      // mouse_controller_update (&mouse, &camera, game_controller);

      // render_stats (&r);
      EndDrawing ();
    }

  // Cleanup

  CloseWindow ();

  return 0;
}
