#include "../include/utility/array_shuffle.h"
#include "../include/grid/grid_types.h"
#include <stdio.h>

void shuffle_array(void *array, size_t n, size_t size,
                   void (*swap)(void *, void *)) {
  char *arr = (char *)array;
  for (size_t i = n - 1; i > 0; --i) {
    size_t j = rand() % (i + 1);
    swap(arr + i * size, arr + j * size);
  }
}

// Example swap function for int arrays
void swap_int(void *a, void *b) {
  int tmp = *(int *)a;
  *(int *)a = *(int *)b;
  *(int *)b = tmp;
}

void swap_grid_cell(void *a, void *b) {
  grid_cell_t tmp = *(grid_cell_t *)a;
  *(grid_cell_t *)a = *(grid_cell_t *)b;
  *(grid_cell_t *)b = tmp;
}

void print_cell(grid_cell_t cell) {
  printf("Cell type: %d\n", cell.type);
  switch (cell.type) {
  case GRID_TYPE_HEXAGON:
    printf("Hexagon cell at (%d, %d, %d)\n", cell.coord.hex.q, cell.coord.hex.r,
           cell.coord.hex.s);
    break;
  case GRID_TYPE_TRIANGLE:
    printf("Triangle cell at (%d, %d, %d)\n", cell.coord.triangle.u,
           cell.coord.triangle.v, cell.coord.triangle.w);
    break;
  case GRID_TYPE_SQUARE:
    printf("Square cell at (%d, %d)\n", cell.coord.square.x,
           cell.coord.square.y);
    break;
  case GRID_TYPE_UNKNOWN:
    printf("Unknown cell type\n");
    break;
  default:
    printf("Cell type not recognized\n");
    break;
  }
}
