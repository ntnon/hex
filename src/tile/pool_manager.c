#include "../../include/tile/pool_manager.h"
#include <stdio.h>

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
pool_map_filter_by_tile_type (pool_manager_t *pm, tile_type_t tile_type,
                              pool_t **out_pools, size_t max_out_pools)
{
  size_t count = 0;
  pool_map_entry_t *entry, *tmp;
  HASH_ITER (hh, pm->pool_map, entry, tmp)
  {
    if (pool_accepts_tile_type (entry->pool, tile_type))
      {
        if (count < max_out_pools)
          {
            out_pools[count++] = entry->pool;
          }
      }
  }
  return count;
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
pool_manager_add_tile_to_pool (pool_manager_t *pm, pool_t *pool,
                               const tile_t *tile)
{
  pool_add_tile (pool, tile);
  pool_manager_assign_tile_to_pool (pm, pool, tile);
}

void
pool_manager_clear (pool_manager_t *pm)
{
  pool_map_free (&pm->pool_map);
  pm->num_pools = 0;
  pm->next_pool_id = 0;
}

void
pool_manager_free (pool_manager_t *pm)
{
  pool_manager_free (pm);
  free (pm);
}
