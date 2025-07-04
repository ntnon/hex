#include "../../include/tile/pool_manager.h"
#include <stdio.h>

#define MAX_POOL_CANDIDATES 50
void
pool_manager_assign_tile_to_best_neighbor_pool (pool_manager_t *pm,
                                                tile_t *tile,
                                                tile_t **neighbor_tiles,
                                                size_t num_neighbor_tiles)
{
  size_t max_num_pools = MAX_POOL_CANDIDATES;
  pool_t *pool_candidates[max_num_pools];
  size_t num_neighbor_pools = pool_manager_find_neighbor_pools_for_tile (
      pm, neighbor_tiles, num_neighbor_tiles, pool_candidates, max_num_pools);

  pool_t *best_pool = pool_manager_find_best_neighbor_pool_for_tile (
      pm, tile, pool_candidates, num_neighbor_pools);
  if (!best_pool)
    {
      printf ("No viable pool for hexagon cell at (%d, %d, %d)\n",
              tile->cell.coord.hex.q, tile->cell.coord.hex.r,
              tile->cell.coord.hex.s);
      pool_manager_create_pool_with_tile (pm, tile);
      return;
    }
  else
    {
      pool_manager_add_tile_to_pool (pm, best_pool, tile);
    }
}

void
pool_manager_create_pool_with_tile (pool_manager_t *pm, const tile_t *tile)
{
  pool_t *pool = pool_create (pool_manager_get_unique_id (pm), tile);
  pool_manager_assign_tile_to_pool (pm, pool, tile);
}

int
pool_manager_get_unique_id (pool_manager_t *pm)
{
  return pm->next_pool_id++;
}

size_t
pool_manager_find_neighbor_pools_for_tile (pool_manager_t *pm,
                                           tile_t **neighbor_tiles,
                                           size_t num_neighbor_tiles,
                                           pool_t **out_pools,
                                           size_t max_pools)
{
  size_t num_neighbor_pools = 0;
  for (size_t i = 0; i < num_neighbor_tiles; ++i)
    {
      tile_t *neighbor = neighbor_tiles[i];
      pool_t *neighbor_pool
          = tile_to_pool_map_get_pool_by_tile (pm->tile_to_pool_map, neighbor);
      if (neighbor_pool)
        {
          // Uniqueness check
          int duplicate = 0;
          for (size_t j = 0; j < num_neighbor_pools; ++j)
            {
              if (out_pools[j] == neighbor_pool)
                {
                  duplicate = 1;
                  break;
                }
            }
          if (!duplicate)
            {
              if (num_neighbor_pools < max_pools)
                {
                  out_pools[num_neighbor_pools++] = neighbor_pool;
                }
              else
                {
                  printf ("Warning: Exceeded max_pools.\n");
                }
            }
        }
      else
        {
          printf ("Neighbor tile %zu is not associated with any pool.\n", i);
        }
    }
  return num_neighbor_pools;
}

pool_t *
pool_manager_find_best_neighbor_pool_for_tile (pool_manager_t *pm,
                                               tile_t *tile,
                                               pool_t **pool_candidates,
                                               size_t num_pool_candidates)
{
  tile_type_t tile_type = tile->type;
  pool_t *best_pool = NULL;
  int best_score = 0;

  for (size_t i = 0; i < num_pool_candidates; i++)
    {
      // Check if this pool accepts the tile's type
      int accepts = 0;
      for (size_t j = 0; j < pool_candidates[i]->num_accepted_tile_types; ++j)
        {
          if (pool_candidates[i]->accepted_tile_types[j] == tile_type)
            {
              accepts = 1;
              break;
            }
        }
      if (!accepts)
        continue;

      int score = pool_score (pool_candidates[i]);
      if (!best_pool || score > best_score)
        {
          best_pool = pool_candidates[i];
          best_score = score;
        }
    }

  if (!best_pool)
    {
      // TODO: implement singleton pool creation
      return NULL;
    }
  return best_pool;
}

void
pool_manager_assign_tile_to_pool (pool_manager_t *pm, pool_t *pool,
                                  const tile_t *tile)
{
  // Remove any existing mapping for this tile
  tile_to_pool_map_remove (&pm->tile_to_pool_map, (tile_t *)tile);

  // Add the new mapping
  tile_to_pool_map_add (&pm->tile_to_pool_map, (tile_t *)tile, pool);
}

void
pool_manager_add_tile_to_pool (pool_manager_t *pm, pool_t *pool,
                               const tile_t *tile)
{
  pool_add_tile (pool, tile);
  pool_manager_assign_tile_to_pool (pm, pool, tile);
}

void
pool_manager_clear (pool_manager_t *pm)
{
  tile_to_pool_map_clear (&pm->tile_to_pool_map);
  pool_map_clear (&pm->pool_map);
}

void
pool_manager_free (pool_manager_t *pm)
{
  pool_manager_clear (pm);
  pool_map_free (&pm->pool_map);
  tile_to_pool_map_free (&pm->tile_to_pool_map);
  free (pm);
}
