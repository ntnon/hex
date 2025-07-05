#include "../../include/board/renderer.h"
#include <stdio.h>

void
draw_tile_wrapper (tile_map_entry_t *entry, void *user_data)
{
  const grid_t *grid = (const grid_t *)user_data;
  renderer_draw_tile (entry->tile, grid);
}

void
renderer_draw_board (const board_t *board)
{
  if (!board)
    {
      printf ("ERROR: board is null\n");
      return;
    }

  if (!board->grid)
    {
      printf ("ERROR: board->grid is null\n");
      return;
    }

  if (!board->tile_manager)
    {
      printf ("ERROR: board->tile_manager is null\n");
      return;
    }

  draw_grid (board->grid);
  tile_map_foreach (board->tile_manager->tiles, draw_tile_wrapper,
                    (void *)board->grid);
}

void
renderer_draw_tile (const tile_t *tile, const grid_t *grid)
{
  if (!tile)
    {
      printf ("ERROR: tile is null\n");
      return;
    }

  if (!grid)
    {
      printf ("ERROR: grid is null\n");
      return;
    }

  if (!grid->vtable)
    {
      printf ("ERROR: grid->vtable is null\n");
      return;
    }

  if (!grid->vtable->draw_cell_with_colors)
    {
      printf ("ERROR: grid->vtable->draw_cell_with_colors is null\n");
      return;
    }

  grid->vtable->draw_cell_with_colors (grid, tile->cell, tile_get_color (tile),
                                       BLANK);
}
