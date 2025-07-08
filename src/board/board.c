#include "../../include/board/renderer.h"
#include "../../include/utility.h"
#include <stdio.h>

#define MAX_POOL_CANDIDATES 10

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
  .size = { 60.0, 60.0 }, // Hex size (adjust as needed)
  .origin = { 0.0, 0.0 }  // Center of the screen (adjust as needed)
};

int radius = 10; // For example

board_t *
board_create (void)
{
  board_t *board = malloc (sizeof (board_t));
  if (!board)
    return NULL; // Always check malloc
  board->pool_manager = pool_manager_create ();
  board->tile_manager = tile_manager_create ();
  board->tile_to_pool = tile_to_pool_map_create ();
  board->grid = grid_create (GRID_TYPE_HEXAGON, layout, radius);
  return board;
}

void
clear_board (board_t *board)
{
  tile_manager_clear (board->tile_manager);
  pool_manager_clear (board->pool_manager);
  tile_to_pool_map_free (&board->tile_to_pool);
}

void
free_board (board_t *board)
{
  tile_manager_free (board->tile_manager);
  pool_manager_free (board->pool_manager);
  tile_to_pool_map_free (&board->tile_to_pool);
  grid_free (board->grid);
  free (board);
}

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

          // This is the actual error case - tile exists but has no pool
          if (!out_pools[i])
            {
              fprintf (
                  stderr,
                  "ERROR: Neighbor tile exists but has no pool assignment!\n");
            }
        }
      else
        {
          out_pools[i] = NULL;
          // This is normal - no tile at this neighbor position
        }
    }
}

void
add_tile (board_t *board, tile_t *tile)
{
  if (!valid_tile (board, tile))
    return;

  pool_t *target_pool = NULL;
  pool_t *candidate_pools[MAX_POOL_CANDIDATES];
  pool_t *filtered_candidate_pools[MAX_POOL_CANDIDATES];

  get_neighbor_pools (board, tile, candidate_pools, 6);

  size_t num_filtered_pools = pool_map_filter_by_tile_type (
      candidate_pools, 6, tile->type, filtered_candidate_pools,
      MAX_POOL_CANDIDATES);

  if (num_filtered_pools == 0)
    {

      target_pool = pool_manager_create_pool (
          board->pool_manager); // create new pool for the tile
    }
  else
    {
      qsort (filtered_candidate_pools, num_filtered_pools, sizeof (pool_t *),
             compare_pools_by_score);

      target_pool = filtered_candidate_pools[0];
    }

  if (!target_pool)
    {
      return;
    }
  tile_manager_add_tile (board->tile_manager, tile); // add to general tile map
  pool_manager_add_tile_to_pool (board->pool_manager, target_pool,
                                 tile); // add tile to pool
  tile_to_pool_map_add (&board->tile_to_pool, tile,
                        target_pool); // add tile to "tile_to_pool_map"
  if (num_filtered_pools > 1)
    {
      printf ("Merging %zu pools\n", num_filtered_pools);
      // Merge all connected pools into the target pool
      target_pool = pool_manager_merge_pools (
          board->pool_manager, &board->tile_to_pool, filtered_candidate_pools,
          num_filtered_pools);
    }

  pool_update (target_pool, board->grid);
}

void
remove_tile (board_t *board, tile_t *tile)
{
  if (!valid_tile (board, tile))
    return;
}

void
randomize_board (board_t *board)
{
  printf ("randomizing board\n");
  grid_cell_t *cells = board->grid->cells;
  shuffle_array (cells, board->grid->num_cells, sizeof (grid_cell_t),
                 swap_grid_cell);

  for (size_t i = 0; i < board->grid->num_cells; i++)
    {
      if (rand () % 1 == 0)
        {
          // In randomize_board()
          tile_t *tile = tile_create_random_ptr (cells[i]);
          if (tile->type != TILE_EMPTY)
            { // Only add non-empty tiles
              add_tile (board, tile);
            }
          else
            {
              // Free the empty tile since we're not using it
              free (tile);
            }
        }
    }
}
void cycle_tile_type (board_t *board, tile_t *tile);
