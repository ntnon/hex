#include "../../include/bye/tile_to_pool_map.h"
#include "stdio.h"

tile_to_pool_entry_t *tile_to_pool_map_create(void) {
  return NULL; // An empty uthash table
}

pool_t *
tile_to_pool_map_get_pool_by_tile(tile_to_pool_entry_t *tile_to_pool_map,
                                  tile_t *tile) {
  tile_to_pool_entry_t *entry = NULL;
  HASH_FIND_PTR(tile_to_pool_map, &tile, entry);
  return entry ? entry->pool : NULL;
}

void tile_to_pool_map_add(tile_to_pool_entry_t **tile_to_pool_map, tile_t *tile,
                          pool_t *pool) {
  tile_to_pool_entry_t *entry = NULL;
  HASH_FIND_PTR(*tile_to_pool_map, &tile, entry);
  if (!entry) {
    entry = malloc(sizeof(tile_to_pool_entry_t));
    entry->tile = tile;
    entry->pool = pool;
    HASH_ADD_PTR(*tile_to_pool_map, tile, entry);
  } else {
    fprintf(stderr, "Tile %p already exists in the map\n", tile);
  }
}

void tile_to_pool_map_remove(tile_to_pool_entry_t **tile_to_pool_map,
                             tile_t *tile) {
  tile_to_pool_entry_t *entry = NULL;
  HASH_FIND_PTR(*tile_to_pool_map, &tile, entry);
  if (entry) {
    HASH_DEL(*tile_to_pool_map, entry);
    free(entry);
  }
}

void tile_to_pool_map_free(tile_to_pool_entry_t **tile_to_pool_map) {
  tile_to_pool_entry_t *entry, *tmp;
  HASH_ITER(hh, *tile_to_pool_map, entry, tmp) {
    HASH_DEL(*tile_to_pool_map, entry);
    free(entry);
  }
  *tile_to_pool_map = NULL;
};

void tile_to_pool_map_reassign_pool(tile_to_pool_entry_t **tile_to_pool_map,
                                    pool_t *from_pool, pool_t *to_pool) {
  if (!tile_to_pool_map || !from_pool || !to_pool)
    return;

  // First, transfer tiles from source pool's internal tiles map to target
  // pool's internal tiles map
  tile_map_entry_t *tile_entry, *tmp;
  HASH_ITER(hh, from_pool->tiles->root, tile_entry, tmp) {
    // Add tile to target pool's internal tiles map
    tile_map_add(to_pool->tiles, tile_entry->tile);

    // Remove tile from source pool's internal tiles map
    tile_map_remove(from_pool->tiles, tile_entry->cell);
  }

  // Then update the global tile-to-pool mapping
  tile_to_pool_entry_t *entry, *tmp_entry;
  HASH_ITER(hh, *tile_to_pool_map, entry, tmp_entry) {
    if (entry->pool == from_pool) {
      // Update the pool pointer in the global mapping
      entry->pool = to_pool;
    }
  }

  // Debug output
}
