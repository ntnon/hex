#ifndef TILE_MAP_H
#define TILE_MAP_H

#include "third_party/uthash.h"
#include "grid/grid_types.h"
#include "tile.h"

/**
 * @brief Individual Hash Table Entry
 * 'cell' is the key, and 'tile' is the associated value.
 */
typedef struct tile_map_entry {
    grid_cell_t cell;      /* Key: grid cell coordinates */
    tile_t *tile;          /* Value: pointer to a tile */
    UT_hash_handle hh;     /* UTHash handle (must be last) */
} tile_map_entry_t;

/**
 * @brief Tile Map Container
 * This struct encapsulates the hash root and additional metadata.
 */
typedef struct tile_map {
    tile_map_entry_t *root;
    int num_tiles;         /* Total number of tiles in the map */
} tile_map_t;

/* Function declarations */

/* Create a new tile map. */
tile_map_t *tile_map_create(void);

/* Free the entire tile map. */
void tile_map_free(tile_map_t *map);

/* Find an entry in the tile map based on a grid cell key. */
tile_map_entry_t* tile_map_find(tile_map_t *map, grid_cell_t cell);

bool tile_map_contains(tile_map_t *map, grid_cell_t cell);

/* Remove an entry identified by a cell from the map. */
void tile_map_remove(tile_map_t *map, grid_cell_t cell);

/* Add a tile into the map. */
void tile_map_add(tile_map_t *map, tile_t *tile);

/* Iterate over each tile map entry. */
void tile_map_foreach_tile(tile_map_t *map, void (*fn)(tile_t *, void *),
                           void *user_data);
#endif // TILE_MAP_H
