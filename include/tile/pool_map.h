#ifndef POOL_MAP_H
#define POOL_MAP_H

#include "pool.h"
#include "../third_party/uthash.h"


// --- Pool Hash Table Entry ---
// If you need to manage multiple pools and look them up by ID:
typedef struct pool_map_entry {
    int id;                 // The key: pool ID.
    pool_t *pool;           // The value: a pointer to the pool.
    UT_hash_handle hh;      // uthash handle.
} pool_map_entry_t;


// --- Pool Map Functions ---

/**
 * @brief Creates and initializes a new tile map (hash table root).
 * @return A pointer to the newly created, empty pool map.
 */
pool_map_entry_t* pool_map_create(void);

/**
 * @brief Frees all memory associated with the tile map and its entries.
 * @param map_root A pointer to the root of the tile map hash table.
 */
void pool_map_free(pool_map_entry_t** map_root);

/**
 * @brief Adds a tile entry to the tile map.
 * @param map_root A pointer to the root of the tile map hash table.
 * @param entry_to_add A pointer to the pool_map_entry_t to add. It will be managed by the map.
 */
void pool_map_add(pool_map_entry_t** map_root, pool_map_entry_t* entry_to_add);

/**
 * @brief Finds a tile entry in the map by its grid cell coordinates.
 * @param map_root A pointer to the root of the tile map hash table.
 * @param search_cell The grid_cell_t to search for.
 * @return A pointer to the found pool_map_entry_t, or NULL if not found.
 */
pool_map_entry_t* pool_map_find(pool_map_entry_t* map_root, grid_cell_t search_cell);

/**
 * @brief Removes a tile entry from the map and frees its memory.
 * @param map_root A pointer to the root of the tile map hash table.
 * @param entry_to_remove A pointer to the pool_map_entry_t to remove.
 */
void pool_map_remove(pool_map_entry_t** map_root, pool_map_entry_t* entry_to_remove);

/**
 * @brief Clears all entries from the tile map and frees their memory.
 * @param map_root A pointer to the root of the tile map hash table.
 */
void pool_map_clear(pool_map_entry_t** map_root);

#endif // POOL_MAP_H
