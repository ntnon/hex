#ifndef TILE_TO_POOL_MAP_H
#define TILE_TO_POOL_MAP_H

#include "tile/tile.h"
#include "tile/pool.h"
#include "../third_party/uthash.h"

/*==============================
  Tile-to-Pool Entry Definition
  This defines a single mapping between a tile and its pool.
===============================*/
typedef struct {
    tile_t *tile;         // Hash key.
    pool_t *pool;         // Associated pool.
    UT_hash_handle hh;    // UTHash handle.
} tile_to_pool_entry_t;

// Create a new tile-to-pool mapping entry.
tile_to_pool_entry_t *tile_to_pool_entry_create(tile_t *tile, pool_t *pool);

/*==============================
  Tile-to-Pool Map Operations
  These functions operate on the hash table of tile-to-pool entries.
===============================*/

// Creates an empty tile-to-pool map.
tile_to_pool_entry_t *tile_to_pool_map_create(void);

// Returns the pool associated with a given tile.
pool_t *tile_to_pool_map_get_pool_by_tile(tile_to_pool_entry_t *tile_to_pool_map, tile_t *tile);

// Adds a new mapping from tile to pool.
void tile_to_pool_map_add(tile_to_pool_entry_t **tile_to_pool_map, tile_t *tile, pool_t *pool);

// Removes the mapping for a given tile.
void tile_to_pool_map_remove(tile_to_pool_entry_t **tile_to_pool_map, tile_t *tile);

// Frees all mapping entries in the tile-to-pool map.
void tile_to_pool_map_free(tile_to_pool_entry_t **tile_to_pool_map);

// Reassigns all mappings from one pool to another.
void tile_to_pool_map_reassign_pool(tile_to_pool_entry_t **tile_to_pool_map, pool_t *from_pool, pool_t *to_pool);

#endif // TILE_TO_POOL_MAP_H
