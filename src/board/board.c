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
get_neighbor_pools (board_t *board, tile_t *tile, pool_t **out_pools,
                    size_t max_neighbors)
{
  grid_cell_t neighbor_cells[6];
  tile_t *neighbor_tiles[6];

  board->grid->vtable->get_neighbor_cells (tile->cell, neighbor_cells);
  cells_to_tile_ptrs (board->tile_manager, neighbor_cells, 6, neighbor_tiles);

  for (size_t i = 0; i < max_neighbors; ++i)
    {
      if (neighbor_tiles[i])
        {
          out_pools[i] = tile_to_pool_map_get_pool_by_tile (
              board->tile_to_pool, neighbor_tiles[i]);
        }
      else
        {
          out_pools[i] = NULL;
          fprintf (stderr, "The neighbor tile does not have a pool -> this "
                           "should not be possible\n");
        }
    }
}

void
add_tile (board_t *board, tile_t *tile)
{
  if (!valid_tile (board, tile))
    return;

  pool_t *candidate_pools[MAX_POOL_CANDIDATES];
  pool_t *filtered_candidate_pools[MAX_POOL_CANDIDATES];

  get_neighbor_pools (board, tile, candidate_pools, 6);

  size_t num_filtered_pools = pool_map_filter_by_tile_type (
      candidate_pools, 6, tile->type, filtered_candidate_pools,
      MAX_POOL_CANDIDATES);

  if (num_filtered_pools == 0)
    {
      fprintf (stderr, "No valid pool found for tile\n");
      pool_manager_create_pool_with_tile (board->pool_manager, tile);
      return;
    }
  else
    {
      qsort (filtered_candidate_pools, num_filtered_pools, sizeof (pool_t *),
             compare_pools_by_score);

      pool_t *target_pool = filtered_candidate_pools[0];
    }

  pool_t *target_pool = filtered_candidate_pools[0];

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
