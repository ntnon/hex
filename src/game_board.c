#include "game_board.h"
#include "hex_grid.h"
#include <stdlib.h>

game_board *
game_board_new (int radius)
{
  hex center = hex_create (0, 0, 0);

  game_board *board = (game_board *)malloc (sizeof (game_board));
  if (board == NULL)
    {
      return NULL;
    }
  board->layout = layout_create (layout_pointy, point_create (10, 10),
                                 point_create (0, 0));
  board->hex_array = generate_hex_grid_radius (center, radius);
  board->tile_array = tile_array_create ();
  board->piece_array = piece_array_create ();
  board->tile_group_array = tile_group_array_create ();
  return board;
}

void
game_board_draw (game_board *board)
{
  // empty tiles
  for (int i = 0; i < board->hex_array.count; i++)
    {
      draw_hex (board->layout, board->hex_array.data[i], 1.0f, WHITE);
    }

  for (int i = 0; i < board->tile_array.count; i++)
    {
      draw_tile (board->layout, board->tile_array.data[i]);
    }
}

void
game_board_free (game_board *board)
{
  hex_array_free (&board->hex_array);
  piece_array_free (&board->piece_array);
  tile_group_array_free (&board->tile_group_array);
  free (board);
}
