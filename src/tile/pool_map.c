#include "../../include/tile/pool_map.h"
#include <stdio.h>

pool_map_t *pool_map_create(void) {
  pool_map_t *map = malloc(sizeof(pool_map_t));
  if (!map) {
    fprintf(stderr, "Out of memory!\n");
    return NULL;
  }
  map->root = NULL;
  map->num_pools = 0;
  map->next_id = 1; // Start from 1 since 0 means "no pool"
  return map;
}

void pool_map_free(pool_map_t *map) {
  if (!map)
    return;
  pool_map_entry_t *el, *tmp;
  HASH_ITER(hh, map->root, el, tmp) {
    HASH_DEL(map->root, el);
    free(el);
  }
  map->num_pools = 0;
  map->next_id = 1; // Reset to 1 since 0 means "no pool"
  free(map);
}

pool_t *pool_map_create_pool(pool_map_t *map) {
  if (!map)
    return NULL;

  // Create a new pool; assumes pool_create returns a pool with id = -1.
  pool_t *pool = pool_create();
  if (!pool)
    return NULL;

  // Add it to the map (this function will assign a unique id).
  pool_map_add(map, pool);

  // Return the registered pool to the caller.
  return pool;
}

void pool_map_add(pool_map_t *map, pool_t *pool) {
  if (!map || !pool)
    return;

  // Remove any existing entry with the same pool id to prevent duplicates.
  pool_map_entry_t *existing = pool_map_find_by_id(map, pool->id);
  if (existing) {
    fprintf(stderr, "ERROR: Duplicate pool ID %d\n", pool->id);
    HASH_DEL(map->root, existing);
    free(existing);
    map->num_pools--;
  }

  // Assign a new unique ID to the pool.
  pool->id = map->next_id++;

  pool_map_entry_t *entry = malloc(sizeof(pool_map_entry_t));
  if (!entry) {
    fprintf(stderr, "Out of memory!\n");
    return;
  }

  entry->id = pool->id;
  entry->pool = pool;
  HASH_ADD_INT(map->root, id, entry);
  map->num_pools++;
}

pool_map_entry_t *pool_map_find_by_tile(pool_map_t *map, tile_t *tile) {
  pool_map_entry_t *entry = NULL;
  for (entry = map->root; entry != NULL; entry = entry->hh.next) {
    if (pool_contains_tile(entry->pool, tile))
      return entry;
  }
  return NULL;
}

bool pool_map_contains_tile(pool_map_t *map, tile_t *tile) {
  if (!map || !tile)
    return false;
  pool_map_entry_t *entry = pool_map_find_by_tile(map, tile);
  return entry != NULL;
}

void pool_map_remove(pool_map_t *map, int id) {
  if (!map)
    return;
  pool_map_entry_t *entry_to_remove = NULL;
  HASH_FIND_INT(map->root, &id, entry_to_remove);
  if (entry_to_remove) {
    HASH_DEL(map->root, entry_to_remove);
    free(entry_to_remove);
    map->num_pools--;
  }
}

pool_map_entry_t *pool_map_find_by_id(pool_map_t *map, int pool_id) {
  pool_map_entry_t *entry = NULL;
  HASH_FIND_INT(map->root, &pool_id, entry);
  return entry;
}

// Direct pool access functions (better ergonomics)
pool_t *pool_map_get_pool(pool_map_t *map, int pool_id) {
  pool_map_entry_t *entry = pool_map_find_by_id(map, pool_id);
  return entry ? entry->pool : NULL;
}

pool_t *pool_map_get_pool_by_tile(pool_map_t *map, tile_t *tile) {
  pool_map_entry_t *entry = pool_map_find_by_tile(map, tile);
  return entry ? entry->pool : NULL;
}
