#include "board/renderer.h"
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
  /*

    Camera2D camera = { 0 };
    camera.target = (Vector2){ 0, 0 };
    camera.offset = (Vector2){ screenWidth / 2.0f, screenHeight / 2.0f };
    camera.zoom = 0.1f;
    camera.rotation = 0.0f;
   */

  // Simple layout test≤

  SetTargetFPS (60);

  board_t *board = board_create ();

  randomize_board (board);

  board_input_controller_t input_ctrl;
  board_input_controller_init (&input_ctrl);
  // SetTraceLogLevel (LOG_WARNING);
  //  Main game loop
  //
  printf ("Tile count %d\n", board->tile_manager->tiles->num_tiles);
  while (!WindowShouldClose ())
    {
      board_input_controller_update (&input_ctrl, board, screenWidth,
                                     screenHeight);
      BeginDrawing ();

      ClearBackground (RAYWHITE);

      renderer_draw (board, &input_ctrl);
      EndDrawing ();
    }

  //
  free_board (board);

  CloseWindow ();

  return 0;
}
