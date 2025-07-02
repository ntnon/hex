#include "../../include/tile/pool.h"
#include "../../include/grid/grid_system.h"
#include "../../include/third_party/uthash.h"

pool_t *
pool_create (int id, pool_type_t type, Color color)
{
  pool_t *pool;
  pool->id = id;
  pool->type = type;
  pool->color = color;
  pool->tiles = tile_map_create ();
  return pool;
};

void
pool_free (pool_t *pool)
{
  tile_map_free (&pool->tiles);
  free (pool);
}

void
pool_update (pool_t *pool)
{
}
void
pool_clear (pool_t *pool)
{
  tile_map_clear (&pool->tiles);
}

void
pool_update_edges (const grid_t *grid, pool_t *pool)
{
  edge_map_entry_t *collected_edges = edge_map_create ();

  tile_map_entry_t *tile_entry, *tmp;
  HASH_ITER (hh, pool->tiles, tile_entry, tmp)
  {
    get_cell_edges (grid, tile_entry->cell, &collected_edges);
  }
  edge_map_free (&pool->edges);  // Free old edges if needed
  pool->edges = collected_edges; // Assign the new set
}


