#include "../../include/tile/pool.h"

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
