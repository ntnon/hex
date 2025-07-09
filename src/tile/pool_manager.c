#include "../../include/tile/pool_manager.h"
#include "../../include/grid/grid_cell_utils.h"
#include <stdio.h>

pool_manager_t *
pool_manager_create (void)
{
  pool_manager_t *pm = malloc (sizeof (pool_manager_t));
  if (!pm)
    return NULL;
  pm->pool_map = pool_map_create ();
  return pm;
}

pool_t *
pool_manager_create_pool (pool_manager_t *pm)
{
  return pool_map_create_pool (pm->pool_map);
}

void
pool_manager_clear (pool_manager_t *pm)
{
  pool_map_free (pm->pool_map);
}

void
pool_manager_free (pool_manager_t *pm)
{
  pool_map_free (pm->pool_map);
  free (pm);
}

size_t
pool_map_filter_by_tile_type (pool_t **pool_candidates, size_t num_candidates,
                              tile_type_t tile_type, pool_t **out_pools,
                              size_t max_out_pools)
{
  size_t count = 0;
  for (size_t i = 0; i < num_candidates && count < max_out_pools; ++i)
    {
      if (pool_candidates[i]
          && pool_accepts_tile_type (pool_candidates[i], tile_type))
        {
          out_pools[count++] = pool_candidates[i];
        }
    }
  return count;
};

pool_t *
pool_manager_merge_pools (pool_manager_t *pm,
                          tile_to_pool_entry_t **tile_to_pool_map,
                          pool_t **pools_to_merge, size_t num_pools)
{
  if (num_pools <= 1)
    return (num_pools == 1) ? pools_to_merge[0] : NULL;

  // Select the target pool based on the lowest ID.
  pool_t *target_pool = pools_to_merge[0];
  for (size_t i = 1; i < num_pools; i++)
    {
      if (pools_to_merge[i]->id < target_pool->id)
        {
          target_pool = pools_to_merge[i];
        }
    }

  // Merge each other pool into the target pool.
  for (size_t i = 0; i < num_pools; i++)
    {
      pool_t *source_pool = pools_to_merge[i];
      if (source_pool == target_pool)
        continue;

      // Reassign tiles from source_pool to target_pool.
      tile_to_pool_map_reassign_pool (tile_to_pool_map, source_pool,
                                      target_pool);

      // Remove the source pool.
      pool_map_remove (pm->pool_map, source_pool->id);
    }
  return target_pool;
}

void
pool_manager_add_pool (pool_manager_t *pm, pool_t *pool)
{
  pool_map_add (pm->pool_map, pool);
}

void
pool_manager_add_tile_to_pool (pool_manager_t *pm, pool_t *pool, tile_t *tile)
{
  pool_add_tile_to_pool (pool, tile);
}
