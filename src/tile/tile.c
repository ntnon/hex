#include "../../include/tile/tile.h"
#include <stdio.h>
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
  return tile_create_ptr (cell, (tile_type_t)rand () % 3 + 1,
                          rand () % 2 ? 2 : 4);
}

Color
tile_get_color (const tile_t *tile)
{
  switch (tile->type)
    {
    case TILE_MAGENTA:
      return MAGENTA;
    case TILE_CYAN:
      return SKYBLUE;
    case TILE_YELLOW:
      return YELLOW;
    case TILE_EMPTY:
      return BLACK;
    default:
      return BLANK; // Or any color you want for empty/unknown
    }
}
