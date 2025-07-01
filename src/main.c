#include "mouse_controller.h"
#include "raylib.h"
#include "renderer.h"
#include <stdatomic.h>
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
  camera.zoom = 1.0f;
  camera.rotation = 0.0f;

  // Simple layout test
  renderer r;

  game_controller *game_controller
      = game_controller_create (game_board_new (6));
  mouse_controller mouse;

  renderer_init (&r, &camera, game_controller, &mouse);
  mouse_controller_init (&mouse, &camera, game_controller);
  game_board_randomize (game_controller->preview_board);
  SetTargetFPS (60);

  // Main game loop
  while (!WindowShouldClose ())
    {
      // Draw
      BeginDrawing ();
      ClearBackground (RAYWHITE);

      BeginMode2D (camera);
      game_controller_draw (game_controller);
      EndMode2D ();

      mouse_controller_update (&mouse, &camera, game_controller);

      render_stats (&r);
      EndDrawing ();
    }

  // Cleanup
  game_controller_free (game_controller);
  mouse_controller_free (&mouse);
  CloseWindow ();

  return 0;
}
