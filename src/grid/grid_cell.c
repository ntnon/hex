#include "../../include/grid/grid_types.h"
#include <stdio.h>

void
print_cell (grid_cell_t cell)
{
  printf ("Cell type: %d\n", cell.type);
  switch (cell.type)
    {
    case GRID_TYPE_HEXAGON:
      printf ("Hexagon cell at (%d, %d, %d)\n", cell.coord.hex.q,
              cell.coord.hex.r, cell.coord.hex.s);
      break;
    case GRID_TYPE_TRIANGLE:
      printf ("Triangle cell at (%d, %d, %d)\n", cell.coord.triangle.u,
              cell.coord.triangle.v, cell.coord.triangle.w);
      break;
    case GRID_TYPE_SQUARE:
      printf ("Square cell at (%d, %d)\n", cell.coord.square.x,
              cell.coord.square.y);
      break;
    case GRID_TYPE_UNKNOWN:
      printf ("Unknown cell type\n");
      break;
    default:
      printf ("Cell type not recognized\n");
      break;
    }
}
