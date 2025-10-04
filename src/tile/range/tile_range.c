#include "../../../include/tile/range/tile_range.h"
#include "../../../include/grid/grid_geometry.h"
#include "../../../include/tile/tile_map.h"

#include <stdio.h>
#include <stdlib.h>

void tile_get_coordinates_in_range(const tile_t *tile, const grid_t *grid,
                                   grid_cell_t **out_cells, size_t *out_count) {
  if (!tile || !grid || !out_cells || !out_count) {
    if (out_cells)
      *out_cells = NULL;
    if (out_count)
      *out_count = 0;
    return;
  }

  // Use the grid's get_cells_in_range function
  grid_get_cells_in_range(grid, tile->cell, tile->range, out_cells, out_count);
}

void tile_get_tiles_in_range(const tile_t *tile, const tile_map_t *tile_map,
                             const grid_t *grid, tile_t ***out_tiles,
                             size_t *out_count) {
  if (!tile || !tile_map || !grid || !out_tiles || !out_count) {
    if (out_tiles)
      *out_tiles = NULL;
    if (out_count)
      *out_count = 0;
    return;
  }

  // First get all coordinates in range
  grid_cell_t *coords_in_range = NULL;
  size_t coord_count = 0;
  tile_get_coordinates_in_range(tile, grid, &coords_in_range, &coord_count);

  if (!coords_in_range || coord_count == 0) {
    *out_tiles = NULL;
    *out_count = 0;
    return;
  }

  // Allocate maximum possible space for tiles
  tile_t **tiles_in_range = malloc(coord_count * sizeof(tile_t *));
  if (!tiles_in_range) {
    free(coords_in_range);
    *out_tiles = NULL;
    *out_count = 0;
    return;
  }

  // Check each coordinate for existing tiles
  size_t tile_count = 0;
  for (size_t i = 0; i < coord_count; i++) {
    tile_map_entry_t *entry =
      tile_map_find((tile_map_t *)tile_map, coords_in_range[i]);
    if (entry && entry->tile) {
      tiles_in_range[tile_count] = entry->tile;
      tile_count++;
    }
  }

  // Resize array to actual count (or free if no tiles found)
  if (tile_count == 0) {
    free(tiles_in_range);
    *out_tiles = NULL;
  } else {
    tile_t **resized = realloc(tiles_in_range, tile_count * sizeof(tile_t *));
    *out_tiles = resized ? resized : tiles_in_range;
  }

  *out_count = tile_count;
  free(coords_in_range);
}
