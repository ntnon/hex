#include "../include/grid/grid_system.h"
#include "grid/grid_types.h"
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

  SetTargetFPS (60);

  const orientation_t layout_pointy_t = { .f0 = 1.732050808,
                                          .f1 = 0.866025404,
                                          .f2 = 0.0,
                                          .f3 = 1.5,
                                          .b0 = 0.577350269,
                                          .b1 = -0.333333333,
                                          .b2 = 0.0,
                                          .b3 = 0.666666667,
                                          .start_angle = 0.5 };
  layout_t layout = {
    .orientation = layout_pointy_t,
    .size = { 30.0, 30.0 }, // Hex size (adjust as needed)
    .origin = { 0.0, 0.0 }  // Center of the screen (adjust as needed)
  };

  int radius = 90; // For example
  grid_t *grid = grid_create (GRID_TYPE_HEXAGON, layout, radius);

  // Main game loop
  while (!WindowShouldClose ())
    {
      // Draw
      BeginDrawing ();
      ClearBackground (RAYWHITE);

      BeginMode2D (camera);
      // game_controller_draw (game_controller);
      grid_t *grid = grid_create (GRID_TYPE_HEXAGON, layout, radius);
      draw_grid (grid);

      EndMode2D ();

      // mouse_controller_update (&mouse, &camera, game_controller);

      // render_stats (&r);
      EndDrawing ();
    }

  // Cleanup

  CloseWindow ();

  return 0;
}
