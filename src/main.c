#include "mouse_controller.h"
#include "raylib.h"
#include <stdatomic.h>

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
  game_board_controller *game_board_controller
      = game_board_controller_create (game_board_new (6));
  mouse_controller mouse;
  mouse_controller_init (&mouse, &camera, game_board_controller);

  SetTargetFPS (60);

  // Main game loop
  while (!WindowShouldClose ())
    {
      mouse_controller_update (&mouse, &camera, game_board_controller);

      // Update
      // Pan with right mouse button

      // Draw
      BeginDrawing ();
      ClearBackground (RAYWHITE);

      BeginMode2D (camera);

      game_board_draw (game_board_controller->main_board);

      EndMode2D ();
      EndDrawing ();
    }

  // Cleanup
  game_board_controller_free (game_board_controller);
  mouse_controller_free (&mouse);
  CloseWindow ();

  return 0;
}
