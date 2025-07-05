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
  pool_t *new_pool = pool_create (pool_manager_get_unique_id (pm), tile);
  if (!new_pool)
    return NULL;
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
  pool->id = pool_manager_get_unique_id (pm);
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
