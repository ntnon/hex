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
  // board->tile_group_array = tile_group_array_create ();
  board->is_dirty = true;
  board->highlight_manager = highlight_manager_create (HOVER_HIGHLIGHT);
  board->ghost_tiles = 0;
  board->magentas = 0;
  board->cyans = 0;
  board->yellows = 0;
  board->magenta_production = 0;
  board->cyan_production = 0;
  board->yellow_production = 0;
  return board;
}

void
game_board_draw (game_board *board)
{
  // empty tiles
  for (int i = 0; i < board->hex_array.count; i++)
    {
      draw_hex (board->layout, board->hex_array.data[i], 0.7f, WHITE);
    }

  // tiles with content
  draw_tile_array (board->layout, board->tile_array);

  // highlighted tiles
  for (int i = 0; i < board->highlight_manager->tile_array->count; i++)
    {
      draw_hex (board->layout,
                board->highlight_manager->tile_array->data[i].hex, 1.0f,
                board->highlight_manager->highlight.color);
    }
}

void
game_board_free (game_board *board)
{
  if (!board)
    return;

  hex_array_free (&board->hex_array);
  tile_array_free (board->tile_array);

  if (board->tile_array)
    tile_array_free (board->tile_array);

  if (board->highlight_manager)
    highlight_manager_free (board->highlight_manager);

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
  clone->ghost_tiles = original->ghost_tiles;
  clone->magentas = original->magentas;
  clone->cyans = original->cyans;
  clone->yellows = original->yellows;
  clone->magenta_production = original->magenta_production;
  clone->cyan_production = original->cyan_production;
  clone->yellow_production = original->yellow_production;

  // Deep copy the hex array
  clone->highlight_manager
      = highlight_manager_create (original->highlight_manager->highlight);
  if (!clone->highlight_manager)
    {
      fprintf (stderr, "Failed to clone highlight_manager.\n");
      free (clone->hex_array.data);
      free (clone);
      return NULL;
    }

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

  clone->tile_array = tile_array_create ();
  if (!clone->tile_array)
    {
      fprintf (stderr, "Failed to allocate memory for tile_array clone.\n");
      free (clone->hex_array.data);
      free (clone);
      return NULL;
    }
  for (int i = 0; i < original->tile_array->count; i++)
    {
      tile_array_push (clone->tile_array, original->tile_array->data[i]);
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

void
game_board_randomize (game_board *board)
{
  for (int i = 0; i < board->hex_array.count; i++)
    {
      if (rand () % 6 != 0)
        continue;
      tile tile = tile_create_random (board->hex_array.data[i]);
      if (tile.type != TILE_EMPTY)
        {
          tile_array_push (board->tile_array, tile);
        }
    }
}

void
game_board_print (game_board *board)
{
  if (!board)
    {
      printf ("Game board is NULL.\n");
      return;
    }

  // Print basic information
  printf ("Game Board ID: %d\n", board->id);
  printf ("Radius: %d\n", board->radius);
  printf ("Center Hex: (%d, %d, %d)\n", board->center.q, board->center.r,
          board->center.s);
  printf ("Is Dirty: %s\n", board->is_dirty ? "Yes" : "No");

  // Print resource counts and production
  printf ("Magentas: %d (Production: %d)\n", board->magentas,
          board->magenta_production);
  printf ("Cyans: %d (Production: %d)\n", board->cyans,
          board->cyan_production);
  printf ("Yellows: %d (Production: %d)\n", board->yellows,
          board->yellow_production);

  // Print hex array information
  printf ("Hex Array Count: %d\n", board->hex_array.count);
  for (int i = 0; i < board->hex_array.count; i++)
    {
      hex h = board->hex_array.data[i];
      printf ("  Hex %d: (%d, %d, %d)\n", i, h.q, h.r, h.s);
    }

  // Print tile array information
  if (board->tile_array)
    {
      printf ("Tile Array Count: %d\n", board->tile_array->count);
      for (int i = 0; i < board->tile_array->count; i++)
        {
          tile *t = &board->tile_array->data[i];
          printf ("  Tile %d: Hex (%d, %d, %d), Type: %s, Value: %d\n", i,
                  t->hex.q, t->hex.r, t->hex.s, get_tile_type_name (t->type),
                  t->value);
        }
    }
  else
    {
      printf ("Tile Array: NULL\n");
    }

  // Print highlight manager information
  if (board->highlight_manager)
    {
      printf ("Highlight Manager: Active Highlights: %d\n",
              board->highlight_manager->tile_array->count);
    }
  else
    {
      printf ("Highlight Manager: NULL\n");
    }
}

void
game_board_cycle_tile (game_board *board, tile *tile)
{
  if (!tile || !board)
    return;
  int index = get_tile_index (board->tile_array, tile);
  tile_array_cycle_tile (board->tile_array, index);
  if (tile->type == TILE_EMPTY)
    {
      tile->value += 1;
    }
}
