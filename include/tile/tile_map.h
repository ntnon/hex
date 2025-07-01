#ifndef TILE_MAP_H
#define TILE_MAP_H


#include "../third_party/uthash.h"
#include "../grid/grid_types.h"
#include "tile.h"
#include "raylib.h"

// --- Tile Hash Table Entry ---
// This is the struct that uthash will manage.
// The 'cell' field is the key, and the 'tile' field is the value.
typedef struct tile_map_entry {
    grid_cell_t cell;      // The key: grid cell coordinates and type.
    tile_t tile;           // The value: the tile itself.
    UT_hash_handle hh;     // uthash handle must be the last member.
} tile_map_entry_t;


// --- Tile Map Functions ---

/**
 * @brief Creates and initializes a new tile map (hash table root).
 * @return A pointer to the newly created, empty tile map.
 */
tile_map_entry_t* tile_map_create(void);

/**
 * @brief Frees all memory associated with the tile map and its entries.
 * @param map_root A pointer to the root of the tile map hash table.
 */
void tile_map_free(tile_map_entry_t** map_root);

/**
 * @brief Adds a tile entry to the tile map.
 * @param map_root A pointer to the root of the tile map hash table.
 * @param entry_to_add A pointer to the tile_map_entry_t to add. It will be managed by the map.
 */
void tile_map_add(tile_map_entry_t** map_root, tile_map_entry_t* entry_to_add);

/**
 * @brief Finds a tile entry in the map by its grid cell coordinates.
 * @param map_root A pointer to the root of the tile map hash table.
 * @param search_cell The grid_cell_t to search for.
 * @return A pointer to the found tile_map_entry_t, or NULL if not found.
 */
tile_map_entry_t* tile_map_find(tile_map_entry_t* map_root, grid_cell_t search_cell);

/**
 * @brief Removes a tile entry from the map and frees its memory.
 * @param map_root A pointer to the root of the tile map hash table.
 * @param entry_to_remove A pointer to the tile_map_entry_t to remove.
 */
void tile_map_remove(tile_map_entry_t** map_root, tile_map_entry_t* entry_to_remove);

/**
 * @brief Clears all entries from the tile map and frees their memory.
 * @param map_root A pointer to the root of the tile map hash table.
 */
void tile_map_clear(tile_map_entry_t** map_root);

#endif // TILE_MAP_H





