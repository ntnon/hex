#include "game_board.h"
#include "hex_grid.h"
#include <stdio.h>
#include <stdlib.h>

#define HOVER_HIGHLIGHT                                                       \
  (highlight) { GREEN }

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
  board->is_dirty = true;
  board->highlight_manager = highlight_manager_create (HOVER_HIGHLIGHT);
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

  // highlighted tiles
  printf ("Highlighted tiles: %d\n",
          board->highlight_manager->tile_array.count);
  for (int i = 0; i < board->highlight_manager->tile_array.count; i++)
    {
      printf ("Highlighted tile %d\n", i);
      draw_hex (board->layout,
                board->highlight_manager->tile_array.data[i].hex, 1.0f,
                board->highlight_manager->highlight.color);
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

game_board *
game_board_clone (const game_board *original)
{
  // Allocate memory for the new game board
  game_board *clone = malloc (sizeof (game_board));
  if (!clone)
    {
      fprintf (stderr, "Failed to allocate memory for game board clone.\n");
      return NULL;
    }

  // Copy the layout directly (assuming it's a simple struct)
  clone->layout = original->layout;

  // Deep copy the hex array
  clone->hex_array.count = original->hex_array.count;
  clone->hex_array.data = malloc (original->hex_array.count * sizeof (hex));
  if (!clone->hex_array.data)
    {
      fprintf (stderr, "Failed to allocate memory for hex array clone.\n");
      free (clone);
      return NULL;
    }
  for (int i = 0; i < original->hex_array.count; i++)
    {
      clone->hex_array.data[i] = original->hex_array.data[i];
    }

  // Return the cloned game board
  return clone;
}

void *
game_board_highlight_tiles (game_board *board, hex *hex)
{
  // Implement the logic to highlight the given hex
  // For example, you can change the color of the hex
  return NULL;
}
