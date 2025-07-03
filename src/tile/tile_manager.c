#include "../../include/tile/tile_manager.h"

tile_manager_t *
tile_manager_create (grid_t *grid)
{
  tile_manager_t *tm = malloc (sizeof (tile_manager_t));
  if (!tm)
    {
      return NULL;
    }
  tm->tiles = tile_map_create ();
  tm->pools = pool_map_create ();

  tm->grid = grid;

  return tm;
}

void
tile_manager_free (tile_manager_t *tm)
{
  tile_map_free (&tm->tiles);
  pool_map_free (&tm->pools);
  free (tm);
}

void
tile_manager_add_tile (tile_manager_t *tm, tile_map_entry_t *tile)
{
  tile_map_add (&tm->tiles, tile);
}

bool tile_manager_remove_tile (tile_manager_t *tm, grid_cell_t cell);
tile_t *tile_manager_get_tile (const tile_manager_t *tm, grid_cell_t cell);

pool_t *tile_manager_create_pool_for_tile (tile_manager_t *tm,
                                           tile_t *tile_ptr, pool_type_t type,
                                           Color color);
pool_t *tile_manager_get_pool_by_id (const tile_manager_t *tm, int pool_id);
bool tile_manager_add_tile_to_pool (tile_manager_t *tm, int pool_id,
                                    tile_t *tile_ptr);
bool tile_manager_remove_tile_from_pool (tile_manager_t *tm, tile_t *tile_ptr);
void tile_manager_update_all_pools (tile_manager_t *tm);
