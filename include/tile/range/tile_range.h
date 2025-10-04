/**************************************************************************//**
 * @file tile_range.h
 * @brief Functions for calculating tiles and coordinates within range of a tile.
 *****************************************************************************/

#ifndef TILE_RANGE_H
#define TILE_RANGE_H

#include "../tile.h"
#include "../tile_map.h"
#include "../../grid/grid_geometry.h"
#include <stddef.h>

/**
 * @brief Gets all tiles within range of a given tile.
 * @param tile The center tile to calculate range from.
 * @param tile_map The tile map to search for tiles.
 * @param grid The grid system for coordinate calculations.
 * @param out_tiles Pointer to store array of tile pointers within range.
 * @param out_count Pointer to store the number of tiles found.
 * @note Caller is responsible for freeing the allocated array.
 */
void tile_get_tiles_in_range(const tile_t *tile, const tile_map_t *tile_map, 
                            const grid_t *grid, tile_t ***out_tiles, size_t *out_count);

/**
 * @brief Gets all coordinates within range of a tile (regardless of whether tiles exist there).
 * @param tile The center tile to calculate range from.
 * @param grid The grid system for coordinate calculations.
 * @param out_cells Pointer to store array of coordinates within range.
 * @param out_count Pointer to store the number of coordinates.
 * @note Caller is responsible for freeing the allocated array.
 */
void tile_get_coordinates_in_range(const tile_t *tile, const grid_t *grid,
                                  grid_cell_t **out_cells, size_t *out_count);

#endif // TILE_RANGE_H