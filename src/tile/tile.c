#include "../../include/tile/tile.h"
#include "../../include/grid/grid_geometry.h"
#include "../../include/grid/grid_types.h"
#include "../../include/tile/tile_map.h"

#include <stdio.h>
#include <stdlib.h>

tile_t *tile_create_ptr(grid_cell_t cell, tile_data_t data) {
  tile_t *t = malloc(sizeof(tile_t));
  if (t) {
    t->cell = cell;
    t->data = data;
    t->range = 0;       // Initialize range to 0
    t->modifier = 0.0f; // Initialize modifier to 0
  }
  return t;
}

tile_data_t tile_data_create(tile_type_t type, int value) {
  tile_data_t data;
  data.type = type;
  data.value = value;
  return data;
}

tile_data_t tile_data_create_random(void) {
  tile_data_t data;
  // Skip TILE_UNDEFINED and TILE_EMPTY, only use TILE_MAGENTA, TILE_CYAN,
  // TILE_YELLOW Don't include TILE_GREEN in random generation as it's reserved
  // for center tiles
  data.type = (tile_type_t)(rand() % 3 + 1); // 1, 2, 3 (MAGENTA, CYAN, YELLOW)
  data.value = rand() % 5 + 1;
  return data;
}

tile_t *tile_create_random_ptr(grid_cell_t cell) {
  return tile_create_ptr(cell, tile_data_create_random());
}

tile_t *tile_create_center_ptr(grid_cell_t cell) {
  tile_data_t center_data = tile_data_create(TILE_GREEN, 5);
  return tile_create_ptr(cell, center_data);
}

void tile_set_coords(tile_t *tile, grid_cell_t coord) {
  if (tile == NULL) {

    printf("tile is NULL\n");
    return;
  }

  if (tile->cell.type != coord.type) {
    printf("cell type mismatch\n");
    return;
  }
  switch (tile->data.type) {
  case GRID_TYPE_SQUARE:
    tile->cell.coord.square.x = coord.coord.square.x;
    tile->cell.coord.square.y = coord.coord.square.y;
    break;
  case GRID_TYPE_HEXAGON:
    tile->cell.coord.hex.q = coord.coord.hex.q;
    tile->cell.coord.hex.r = coord.coord.hex.r;
    tile->cell.coord.hex.s = coord.coord.hex.s;
    break;
  case GRID_TYPE_TRIANGLE:
    tile->cell.coord.triangle.u = coord.coord.triangle.u;
    tile->cell.coord.triangle.v = coord.coord.triangle.v;
    tile->cell.coord.triangle.w = coord.coord.triangle.w;
    break;
  default:
    printf("unknown type\n");
    break;
  }
}
void tile_add_coords(tile_t *tile, grid_cell_t coord) {
  if (tile == NULL) {

    printf("tile is NULL\n");
    return;
  }

  if (tile->cell.type != coord.type) {
    printf("cell type mismatch\n");
    return;
  }
  switch (tile->cell.type) {
  case GRID_TYPE_SQUARE:
    tile->cell.coord.square.x =
      coord.coord.square.x + tile->cell.coord.square.x;
    tile->cell.coord.square.y =
      coord.coord.square.y + tile->cell.coord.square.y;
    break;
  case GRID_TYPE_HEXAGON:
    tile->cell.coord.hex.q = coord.coord.hex.q + tile->cell.coord.hex.q;
    tile->cell.coord.hex.r = coord.coord.hex.r + tile->cell.coord.hex.r;
    tile->cell.coord.hex.s = coord.coord.hex.s + tile->cell.coord.hex.s;
    break;
  case GRID_TYPE_TRIANGLE:
    tile->cell.coord.triangle.u =
      coord.coord.triangle.u + tile->cell.coord.triangle.u;
    tile->cell.coord.triangle.v =
      coord.coord.triangle.v + tile->cell.coord.triangle.v;
    tile->cell.coord.triangle.w =
      coord.coord.triangle.w + tile->cell.coord.triangle.w;
    break;
  default:
    printf("unknown type\n");
    break;
  }
}

void tile_cycle(tile_t *tile) { tile->data.type = (tile->data.type + 1) % 3; }

void tile_destroy(tile_t *tile) {
  printf("Tile destroyed\n");
  free(tile);
}

// --- Range and Modifier Functions ---

void tile_set_range(tile_t *tile, uint8_t range) {
  if (tile) {
    tile->range = range & 0x07; // Clamp to 0-7 (3 bits)
  }
}

uint8_t tile_get_range(const tile_t *tile) { return tile ? tile->range : 0; }

void tile_set_modifier(tile_t *tile, float modifier) {
  if (tile) {
    tile->modifier = modifier;
  }
}

void tile_add_modifier(tile_t *tile, float modifier_delta) {
  if (tile) {
    tile->modifier += modifier_delta;
  }
}

float tile_get_modifier(const tile_t *tile) {
  return tile ? tile->modifier : 0.0f;
}

float tile_get_effective_production(const tile_t *tile) {
  if (!tile)
    return 0.0f;
  return (float)tile->data.value + tile->modifier;
}

// --- Range Calculation Functions ---

void tile_get_coordinates_in_range(grid_type_e grid_type, const tile_t *tile,
                                   grid_cell_t **out_cells, size_t *out_count) {
  if (!tile || !out_cells || !out_count) {
    if (out_cells)
      *out_cells = NULL;
    if (out_count)
      *out_count = 0;
    return;
  }

  // Use the grid's get_cells_in_range function
  grid_geometry_get_cells_in_range(grid_type, tile->cell, tile->range,
                                   out_cells, out_count);
}

void tile_get_tiles_in_range(grid_type_e grid_type, const tile_t *tile,
                             const tile_map_t *tile_map, tile_t ***out_tiles,
                             size_t *out_count) {
  if (!tile || !tile_map || !out_tiles || !out_count) {
    if (out_tiles)
      *out_tiles = NULL;
    if (out_count)
      *out_count = 0;
    return;
  }

  // First get all coordinates in range
  grid_cell_t *coords_in_range = NULL;
  size_t coord_count = 0;
  tile_get_coordinates_in_range(grid_type, tile, &coords_in_range,
                                &coord_count);

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
