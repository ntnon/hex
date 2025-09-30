#include "../../include/grid/grid_cell_utils.h"
#include <stdio.h>

void print_grid_cell(const grid_cell_t *cell) {
  char buffer[32];
  grid_cell_to_string(cell, buffer, sizeof(buffer));
  printf("Cell: %s", buffer);
}

void grid_cell_to_string(const grid_cell_t *cell, char *buffer,
                         size_t buffer_size) {
  // In a valid hex (cube coordinate) cell, the invariant is:
  //    q + r + s == 0.
  // If the invariant isn't satisfied, it's a sign something went wrong.
  if ((cell->coord.hex.q + cell->coord.hex.r + cell->coord.hex.s) != 0) {
    snprintf(buffer, buffer_size, "Invalid: %d,%d,%d", cell->coord.hex.q,
             cell->coord.hex.r, cell->coord.hex.s);
  } else {
    snprintf(buffer, buffer_size, "%d,%d,%d", cell->coord.hex.q,
             cell->coord.hex.r, cell->coord.hex.s);
  }
}

bool grid_cells_equal(const grid_cell_t *a, const grid_cell_t *b) {
  return (a->coord.hex.q == b->coord.hex.q &&
          a->coord.hex.r == b->coord.hex.r && a->coord.hex.s == b->coord.hex.s);
}
