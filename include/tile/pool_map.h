#ifndef POOL_MAP_H
#define POOL_MAP_H

#include "pool.h"
#include "../third_party/uthash.h"

// --- Pool Hash Table Entry ---
// This struct is managed by UTHash.
typedef struct pool_map_entry {
    int id;            // Key: pool ID
    pool_t *pool;      // Value: pointer to the pool
    UT_hash_handle hh; // uthash handle for linking entries
} pool_map_entry_t;

// --- Pool Map Container ---
// Encapsulates the hash table and associated metadata.
typedef struct pool_map {
    pool_map_entry_t *root; // Root hash table pointer
    size_t num_pools;          // Number of pools in the map
    int next_id;
} pool_map_t;

// Create and initialize a new pool map.
pool_map_t *pool_map_create(void);

pool_t* pool_map_create_pool(pool_map_t *map);

// Free the entire pool map.
void pool_map_free(pool_map_t *map);

// Add a pool to the map, keyed by its id.
void pool_map_add(pool_map_t *map, pool_t *pool);

// Remove a pool from the map given its id.
void pool_map_remove(pool_map_t *map, int id);

// Find an entry in the pool map by pool id.
pool_map_entry_t *pool_map_find_by_id(pool_map_t *map, int pool_id);

// Optionally, if you need to map from a tile (or tile id) to a pool, add:
pool_map_entry_t *pool_map_find_by_tile(pool_map_t *map, tile_t* tile);

bool pool_map_contains_tile(pool_map_t *map, tile_t* tile);

#endif /* POOL_MAP_H */
