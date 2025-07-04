#include "../../include/board/board.h"
#include <stdio.h>

#define MAX_POOL_CANDIDATES 10

void
draw_board (board_t *board)
{
}

board_t board_create (void);

void clear_board (board_t *board);
void free_board (board_t *board);

bool
valid_tile (board_t *board, tile_t *tile)
{
  if (!board->grid->vtable->is_valid_cell (board->grid, tile->cell))
    {
      fprintf (stderr, "Tile cell not in grid\n");
      return false;
    }
  return true;
}

void
add_tile (board_t *board, tile_t *tile)
{
  if (!valid_tile (board, tile))
    return;

  pool_t *target_pool = NULL;
  // a cell can only have 6 neighbors
  grid_cell_t neighbor_cell[6];
  tile_t *neighbor_tiles[6];
  pool_t *candidate_pools[6];
  pool_t *filtered_candidate_pools[6];
  int neighbor_count = 0;

  // Get neighbor cells
  board->grid->vtable->get_neighbor_cells (tile->cell, neighbor_cell);

  // Get neighbor tiles
  cells_to_tile_ptrs (board->tile_manager, neighbor_cell, 6, neighbor_tiles);

  // For each neighbor tile, get its pool
  for (int i = 0; i < 6; i++)
    {
      if (neighbor_tiles[i])
        {
          candidate_pools[i] = tile_to_pool_map_get_pool_by_tile (
              board->tile_to_pool, neighbor_tiles[i]);
          neighbor_count++;
        }
      else
        {
          candidate_pools[i] = NULL;
          fprintf (stderr, "The neighbor tile does not have a pool -> this "
                           "should not be possible\n");
        }
    }

  // remove pools that do not accept tiles of the incoming tiles type
  size_t num_filtered_pools = pool_map_filter_by_tile_type (
      candidate_pools, tile->type, filtered_candidate_pools, MAX_POOL_CANDIDATES);
  tile_to_pool_map_add (&board->tile_to_pool, tile, target_pool);
}

void
remove_tile (board_t *board, tile_t *tile)
{
  if (!valid_tile (board, tile))
    return;
}

void board_draw (board_t *board);

void randomize_board (board_t *board);

void cycle_tile_type (board_t *board, tile_t *tile);
