#include "../../include/tile/tile.h"
#include "../grid/grid_cell.c"
tile_t
tile_create (grid_cell_t cell, tile_type_t type, int value)
{
  tile_t tile;
  tile.cell = cell;
  tile.type = type;
  tile.value = value;
  return tile;
}
