#include "../../include/tile/tile.h"
#include "../grid/grid_cell.c"
#include <stdlib.h>

tile_t *
tile_create_ptr (grid_cell_t cell, tile_type_t type, int value)
{
  tile_t *t = malloc (sizeof (tile_t));
  if (t)
    {
      t->cell = cell;
      t->type = type;
      t->value = value;
    }
  return t;
}

tile_t *
tile_create_random_ptr (grid_cell_t cell)
{
  tile_t tile;
  return tile_create_ptr (cell, (tile_type_t)rand () % 4, rand () % 2 ? 2 : 4);
}
