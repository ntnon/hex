#include "../../include/tile/tile_manager.h"
#include "../utility.c"
#include <stdio.h>

/*
tile_manager_t *
tile_manager_create (grid_t *grid)
{
  tile_manager_t *tm = malloc (sizeof (tile_manager_t));
  if (!tm)
    {
      return NULL;
    }
  tm->tiles = tile_map_create ();
  tm->pool_manager = pool_manager_create ();

  tm->grid = grid;

  return tm;
}

void
tile_manager_find_best_pool_for_tile (tile_manager_t *tm, tile_t *tile)
{
  size_t num_neighbors = tm->grid->vtable->num_neighbors;
  grid_cell_t neighbors[num_neighbors];
  tile_t *neighbor_tiles[num_neighbors];
  tm->grid->vtable->get_neighbors (tile->cell, neighbors);
  pool_manager_assign_tile_to_best_neighbor_pool (
      tm->pool_manager, tile, neighbor_tiles, num_neighbors);
}

void
tile_manager_add_tile (tile_manager_t *tm, tile_t *tile)
{
  tile_map_add (&tm->tiles, tile);
  tile_manager_find_best_pool_for_tile (tm, tile);
}

void
tile_manager_clear (tile_manager_t *tm)
{
  pool_manager_clear (tm->pool_manager);
  tile_map_free (&tm->tiles);
}

void
tile_manager_free (tile_manager_t *tm)
{
  pool_manager_free (tm->pool_manager);
  tile_manager_clear (tm);
  free (tm);
}

void
tile_manager_randomize_board (tile_manager_t *tm)
{
  tile_manager_clear (tm); // always clears the board
  size_t cell_count = tm->grid->num_cells;
  grid_cell_t *cells = tm->grid->cells;
  shuffle_array (cells, cell_count, sizeof (grid_cell_t), swap_grid_cell);

  for (size_t i = 0; i < cell_count; i++)
    {
      if (rand () % 2 == 0) // only generate some tiles.
        {

          tile_t *new_tile_ptr = tile_create_random_ptr (cells[i]);
          tile_manager_add_tile (tm, new_tile_ptr);
        }
    }
}

*/
tile_t *
cell_to_tile_ptr (tile_manager_t *tm, grid_cell_t cell)
{
  tile_map_entry_t *entry = tile_map_find (tm->tiles, cell);
  if (!entry)
    {
      printf ("Tile not found at cell\n");
      return NULL;
    }
  return entry->tile;
}

void
cells_to_tile_ptrs (tile_manager_t *tm, const grid_cell_t *cells,
                    size_t num_cells, tile_t **out_tile_ptrs)
{
  for (size_t i = 0; i < num_cells; ++i)
    {
      out_tile_ptrs[i] = cell_to_tile_ptr (tm, cells[i]);
    }
}

/*
 * NOT IMPLEMENTED YET
 */
bool tile_manager_remove_tile (tile_manager_t *tm, grid_cell_t cell);

bool tile_manager_add_tile_to_pool (tile_manager_t *tm, int pool_id,
                                    tile_t *tile_ptr);
bool tile_manager_remove_tile_from_pool (tile_manager_t *tm, tile_t *tile_ptr);
void tile_manager_update_all_pools (tile_manager_t *tm);
