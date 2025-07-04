#include "../../include/tile/pool.h"
#include "../../include/grid/grid_system.h"
#include "../../include/third_party/uthash.h"
#include <stdio.h>

pool_t *
pool_create (int id, const tile_t *tile)
{
  pool_t *pool;
  pool->id = id;
  pool->accepted_tile_types[0] = tile->type;
  pool->num_accepted_tile_types = 1;
  pool->tiles = tile_map_create ();
  pool->edges = edge_map_create ();
  return pool;
}

void
pool_add_tile (pool_t *pool, const tile_t *tile_ptr)
{
  tile_map_add (&pool->tiles, (tile_t *)tile_ptr);
}

void
pool_add_accepted_tile_type (pool_t *pool, tile_type_t type)
{
  // Check for duplicate
  for (size_t i = 0; i < pool->num_accepted_tile_types; ++i)
    {
      if (pool->accepted_tile_types[i] == type)
        return; // Already present, do not add
    }
  // Add if not present and within bounds
  if (pool->num_accepted_tile_types < MAX_ACCEPTED_TILE_TYPES)
    {
      pool->accepted_tile_types[pool->num_accepted_tile_types++] = type;
    }
  else
    {
      // Optionally handle error: too many types
      printf ("Error: Maximum number of accepted tile types reached.\n");
    }
}

void
pool_update (pool_t *pool)
{
  printf ("Pool update logic not implemented yet.\n");
}

void
pool_free (pool_t *pool)
{
  tile_map_free (&pool->tiles);
  edge_map_free (&pool->edges);
  free (pool);
};

int
pool_score (pool_t *pool)
{
  // Example: prioritize size, then lower ID as tiebreaker
  // You can adjust weights as needed
  return 100000 * tile_map_size (pool->tiles) - pool->id;
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
};
