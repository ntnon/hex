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

bool tile_map_contains(const tile_map_t *map, grid_cell_t cell);

/* Remove an entry identified by a cell from the map. */
void tile_map_remove(tile_map_t *map, grid_cell_t cell);

/* Add a tile into the map. */
void tile_map_add(tile_map_t *map, tile_t *tile);

/* Iterate over each tile map entry. */
void tile_map_foreach_tile(tile_map_t *map, void (*fn)(tile_t *, void *),
                           void *user_data);

/**
 * @brief Applies an offset to all tiles in the tile map.
 * @param tile_map The tile map to offset.
 * @param offset The offset to apply to all tile coordinates.
 * @return True if offset was successful, false on memory allocation failure.
 */
bool tile_map_apply_offset(tile_map_t *tile_map, grid_cell_t offset);

/**
 * @brief Rotates all tiles in the tile map around a center point.
 * @param tile_map The tile map to rotate.
 * @param center The center point to rotate around.
 * @param rotation_steps Number of 60-degree clockwise rotation steps (0-5).
 * @return True if rotation was successful, false on memory allocation failure.
 */
bool tile_map_rotate(tile_map_t *tile_map, grid_cell_t center, int rotation_steps);

/**
 * @brief Merges source tile map into destination tile map.
 * @param dest The destination tile map to merge into.
 * @param source The source tile map to merge from.
 * @return True if merge was successful, false if any conflicts or memory allocation failed.
 * @note Source tile map is not modified. Conflicting positions will cause merge to fail.
 * @note Use tile_map_apply_offset on source before merging if offset is needed.
 */
bool tile_map_merge(tile_map_t *dest, const tile_map_t *source);

/**
 * @brief Creates a deep copy of a tile map.
 * @param source The source tile map to clone.
 * @return A new tile map containing copies of all tiles from source, or NULL on failure.
 * @note Caller is responsible for freeing the returned tile map.
 */
tile_map_t *tile_map_clone(const tile_map_t *source);

/**
 * @brief Finds overlapping coordinates between two tile maps.
 * @param map1 The first tile map.
 * @param map2 The second tile map.
 * @param out_overlaps Pointer to store the allocated array of overlapping coordinates.
 * @param out_count Pointer to store the number of overlapping coordinates.
 * @return True if successful, false on memory allocation failure.
 * @note Caller is responsible for freeing the allocated array.
 */
bool tile_map_find_overlaps(const tile_map_t *map1, const tile_map_t *map2, 
                           grid_cell_t **out_overlaps, size_t *out_count);

/**
 * @brief Finds conflicts when merging a source tile map with offset into a destination tile map.
 * @param source The source tile map to be merged.
 * @param dest The destination tile map to check against.
 * @param offset The offset to apply to source tiles before checking.
 * @param out_conflicts Pointer to store the allocated array of conflicting coordinates.
 * @param out_count Pointer to store the number of conflicting coordinates.
 * @return True if successful, false on memory allocation failure.
 * @note Caller is responsible for freeing the allocated array.
 * @note Only reports positions that would overlap between the two tile maps.
 */
bool tile_map_find_merge_conflicts(const tile_map_t *source, const tile_map_t *dest,
                                  grid_cell_t offset, grid_cell_t **out_conflicts, 
                                  size_t *out_count);

/**
 * @brief Checks if a source tile map can be merged with offset into destination without conflicts.
 * @param source The source tile map to be merged.
 * @param dest The destination tile map to check against.
 * @param offset The offset to apply to source tiles before checking.
 * @return True if merge is valid (no overlapping positions), false otherwise.
 */
bool tile_map_can_merge_with_offset(const tile_map_t *source, const tile_map_t *dest, 
                                    grid_cell_t offset);

#endif // TILE_MAP_H
