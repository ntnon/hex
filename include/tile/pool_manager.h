#ifndef pool_manager_H
#define pool_manager_H

#include "pool.h"
#include "../third_party/uthash.h"

// --- Pool Hash Table Entry ---
// This struct is managed by UTHash.
typedef struct pool_manager_entry {
    int id;            // Key: pool ID
    pool_t *pool;      // Value: pointer to the pool
    UT_hash_handle hh; // uthash handle for linking entries
} pool_manager_entry_t;

// --- Pool Map Container ---
// Encapsulates the hash table and associated metadata.
typedef struct pool_manager {
    pool_manager_entry_t *root; // Root hash table pointer
    size_t num_pools;          // Number of pools in the map
    int next_id;
} pool_manager_t;

// Create and initialize a new pool map.
pool_manager_t *pool_manager_create(void);

pool_t* pool_manager_create_pool(pool_manager_t *map);

// Free the entire pool map.
void pool_manager_free(pool_manager_t *map);

// Add a pool to the map, keyed by its id.
void pool_manager_add(pool_manager_t *map, pool_t *pool);

// Remove a pool from the map given its id.
void pool_manager_remove(pool_manager_t *map, int id);

// Find an entry in the pool map by pool id.
pool_manager_entry_t *pool_manager_find_by_id(pool_manager_t *map, int pool_id);

// Direct pool access functions (better ergonomics)
pool_t *pool_manager_get_pool(pool_manager_t *map, int pool_id);
pool_t *pool_manager_get_pool_by_tile(pool_manager_t *map, tile_t* tile);

// Optionally, if you need to map from a tile (or tile id) to a pool, add:
pool_manager_entry_t *pool_manager_find_by_tile(pool_manager_t *map, tile_t* tile);

bool pool_manager_contains_tile(pool_manager_t *map, tile_t* tile);

#endif /* pool_manager_H */
