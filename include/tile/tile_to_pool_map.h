#include "pool.h"
#include "../third_party/uthash.h"

// Hash table entry mapping a tile to its pool.
typedef struct {
    tile_t *tile;         // Pointer to the tile (hash key).
    pool_t *pool;         // Pointer to the associated pool (hash value).
    UT_hash_handle hh;    // uthash handle for linking entries in the hash table.
} tile_to_pool_entry_t;

pool_t* tile_to_pool_map_get_pool_by_tile(tile_to_pool_entry_t *tile_to_pool_map, tile_t *tile);

void tile_to_pool_map_add(tile_to_pool_entry_t **tile_to_pool_map, tile_t *tile, pool_t *pool);

void tile_to_pool_map_remove(tile_to_pool_entry_t **tile_to_pool_map, tile_t *tile);

void tile_to_pool_map_clear(tile_to_pool_entry_t **tile_to_pool_map);

void tile_to_pool_map_free(tile_to_pool_entry_t **tile_to_pool_map);
