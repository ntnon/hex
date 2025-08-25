#include "game/tile_grid.h"
#include "tile/tile_map.h"
#include <stdlib.h>

tile_grid_t *tile_grid_create(grid_type_e grid_type, layout_t layout,
                              int size) {
  tile_grid_t *tg = malloc(sizeof(tile_grid_t));
  if (!tg)
    return NULL;

  tg->grid = grid_create(grid_type, layout, size);
  tg->camera = (Camera2D){0};
  tg->tiles = tile_map_create();

  return tg;
}

void tile_grid_destroy(tile_grid_t *tg) {
  if (!tg)
    return;
  tile_map_free(tg->tiles);
  grid_free(tg->grid);
  free(tg);
}

void tile_grid_add(tile_grid_t *tg, tile_t tile) {
  if (!tg)
    return;
  tile_map_add(tg->tiles, &tile);
}

void tile_grid_remove(tile_grid_t *tg, grid_cell_t cell) {
  tile_map_remove(tg->tiles, cell);
}
