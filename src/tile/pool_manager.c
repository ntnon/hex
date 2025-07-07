#include "../../include/tile/pool_manager.h"
#include <stdio.h>

pool_manager_t *
pool_manager_create (void)
{
  pool_manager_t *pm = malloc (sizeof (pool_manager_t));
  if (!pm)
    return NULL;
  pm->pool_map = NULL;
  pm->num_pools = 0;
  pm->next_pool_id = 0;
  return pm;
}

pool_t *
pool_manager_create_pool_with_tile (pool_manager_t *pm, const tile_t *tile)
{
  pool_t *new_pool = pool_create (0, tile);
  if (new_pool)
    {
      new_pool->id = pool_manager_get_unique_id (pm);
      printf ("Created pool %d, total pools: %zu\n", new_pool->id,
              pm->num_pools + 1);
    }
  pool_map_add (&pm->pool_map, new_pool->id, new_pool);
  pm->num_pools++;
  // Optionally, assign the tile to the pool here if needed
  return new_pool;
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
  pool_map_free (&pm->pool_map);
  free (pm);
}

int
pool_manager_get_unique_id (pool_manager_t *pm)
{
  if (!pm)
    {
      fprintf (stderr,
               "ERROR: pool_manager_get_unique_id called with NULL pm\n");
      abort ();
    }
  return pm->next_pool_id++;
}

void
pool_manager_add_pool (pool_manager_t *pm, pool_t *pool)
{
  pool_map_add (&pm->pool_map, pool->id, pool);
  pm->num_pools++;
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

void
pool_manager_merge_pools (pool_manager_t *pm,
                          tile_to_pool_entry_t **tile_to_pool_map,
                          pool_t **pools_to_merge, size_t num_pools)
{
  if (num_pools <= 1)
    return;

  // Find lowest ID pool
  pool_t *target_pool = pools_to_merge[0];
  for (size_t i = 1; i < num_pools; i++)
    {
      if (pools_to_merge[i]->id < target_pool->id)
        {
          target_pool = pools_to_merge[i];
        }
    }

  // Merge each pool into target
  for (size_t i = 0; i < num_pools; i++)
    {
      pool_t *source_pool = pools_to_merge[i];
      if (source_pool == target_pool)
        continue;

      // Delegate tile reassignment to tile_to_pool_map
      tile_to_pool_map_reassign_pool (tile_to_pool_map, source_pool,
                                      target_pool);

      // Remove source pool
      pool_map_remove (&pm->pool_map, source_pool->id);
      pm->num_pools--;
    }
}
