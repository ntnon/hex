#ifndef TILE_GRID_H
#define TILE_GRID_H

#include "raylib.h"
#include "grid/grid_system.h"
#include "tile/tile.h"

#include "tile/tile_map.h"

// Use kvac to manage dynamic array of tiles
typedef struct {
    Camera2D camera;
    grid_t *grid;
    tile_map_t *tiles;
} tile_grid_t;

tile_grid_t *tile_grid_create(grid_type_e grid_type, layout_t layout, int size);

void tile_grid_destroy(tile_grid_t *tg);

void tile_grid_add(tile_grid_t *tg, tile_t tile);

void tile_grid_remove(tile_grid_t *tg, grid_cell_t cell );

#endif // TILE_GRID_H
