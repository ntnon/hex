#include "../../include/tile/tile.h"
#include "../../include/grid/grid_types.h"
#include <stdio.h>
#include <stdlib.h>

tile_t *
tile_create_ptr (grid_cell_t cell, tile_data_t data)
{
  tile_t *t = malloc (sizeof (tile_t));
  if (t)
    {
      t->cell = cell;
      t->data = data;
    }
  return t;
}

tile_data_t
tile_data_create (tile_type_t type, int value)
{
  tile_data_t data;
  data.type = type;
  data.value = value;
  return data;
}

tile_data_t
tile_data_create_random (void)
{
  tile_data_t data;
  data.type = (tile_type_t)rand () % 3 + 1;
  data.value = rand () % 5 + 1;
  return data;
}

tile_t *
tile_create_random_ptr (grid_cell_t cell)
{
  return tile_create_ptr (cell, tile_data_create_random ());
}

Color
tile_get_color (const tile_data_t tile_data)
{
  switch (tile_data.type)
    {
    case TILE_MAGENTA:
      return MAGENTA;
    case TILE_CYAN:
      return SKYBLUE;
    case TILE_YELLOW:
      return GREEN;
    case TILE_EMPTY:
      return BLACK;
    default:
      return LIGHTGRAY; // Or any color you want for empty/unknown
    }
}

void
tile_set_coords (tile_t *tile, grid_cell_t coord)
{
  if (tile == NULL)
    {

      printf ("tile is NULL\n");
      return;
    }

  if (tile->cell.type != coord.type)
    {
      printf ("cell type mismatch\n");
      return;
    }
  switch (tile->data.type)
    {
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
      printf ("unknown type\n");
      break;
    }
}
void
tile_add_coords (tile_t *tile, grid_cell_t coord)
{
  if (tile == NULL)
    {

      printf ("tile is NULL\n");
      return;
    }

  if (tile->cell.type != coord.type)
    {
      printf ("cell type mismatch\n");
      return;
    }
  switch (tile->cell.type)
    {
    case GRID_TYPE_SQUARE:
      tile->cell.coord.square.x
          = coord.coord.square.x + tile->cell.coord.square.x;
      tile->cell.coord.square.y
          = coord.coord.square.y + tile->cell.coord.square.y;
      break;
    case GRID_TYPE_HEXAGON:
      tile->cell.coord.hex.q = coord.coord.hex.q + tile->cell.coord.hex.q;
      tile->cell.coord.hex.r = coord.coord.hex.r + tile->cell.coord.hex.r;
      tile->cell.coord.hex.s = coord.coord.hex.s + tile->cell.coord.hex.s;
      break;
    case GRID_TYPE_TRIANGLE:
      tile->cell.coord.triangle.u
          = coord.coord.triangle.u + tile->cell.coord.triangle.u;
      tile->cell.coord.triangle.v
          = coord.coord.triangle.v + tile->cell.coord.triangle.v;
      tile->cell.coord.triangle.w
          = coord.coord.triangle.w + tile->cell.coord.triangle.w;
      break;
    default:
      printf ("unknown type\n");
      break;
    }
}

void
tile_cycle (tile_t *tile)
{
  tile->data.type = (tile->data.type + 1) % 3;
}

void
tile_destroy (tile_t *tile)
{
  printf ("Tile destroyed\n");
  free (tile);
}
