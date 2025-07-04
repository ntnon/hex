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
    grid_cell_t cell;      // Key: grid cell coordinates (and type, if applicable).
    tile_t *tile;           // Value: the tile data for this cell.
    UT_hash_handle hh;     // uthash handle for hash table linkage (must be last).
} tile_map_entry_t;

tile_map_entry_t *
tile_map_create (void);
void
tile_map_free (tile_map_entry_t **map_root);

tile_map_entry_t* tile_map_find(tile_map_entry_t *map_root, grid_cell_t cell);
void tile_map_remove(tile_map_entry_t **map_root, grid_cell_t cell);
void tile_map_add(tile_map_entry_t **map_root, tile_t *tile); // still takes tile, since you need to store it

int
tile_map_size (tile_map_entry_t *map_root);

void
tile_map_foreach (tile_map_entry_t *map_root,
                  void (*fn) (tile_map_entry_t *, void *), void *user_data);


#endif // TILE_MAP_H
