#ifndef POOL_MAP_H
#define POOL_MAP_H

#include "pool.h"
#include "../third_party/uthash.h"


// --- Pool Hash Table Entry ---
typedef struct pool_map_entry {
    int id;                 // The key: pool ID.
    pool_t *pool;           // The value: a pointer to the pool.
    UT_hash_handle hh;      // uthash handle.
} pool_map_entry_t;

pool_map_entry_t* pool_map_create(void);
void pool_map_free(pool_map_entry_t** map_root);
void pool_map_add(pool_map_entry_t **map_root, int id, pool_t *pool);

void pool_map_remove(pool_map_entry_t** map_root, pool_map_entry_t* entry_to_remove);
void pool_map_free(pool_map_entry_t** map_root);

pool_map_entry_t* find_pool_by_id(pool_map_entry_t* map_root, int pool_id);
pool_map_entry_t* find_pool_by_tile(pool_map_entry_t* map_root, int tile_id);

#endif /* POOL_MAP_H */
