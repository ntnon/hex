#include "../include/grid/grid_types.h"
#include <stdlib.h> // for rand()
#include <time.h>   // for seeding rand()

void
shuffle_array (void *array, size_t n, size_t size,
               void (*swap) (void *, void *))
{
  char *arr = (char *)array;
  for (size_t i = n - 1; i > 0; --i)
    {
      size_t j = rand () % (i + 1);
      swap (arr + i * size, arr + j * size);
    }
}

// Example swap function for int arrays
void
swap_int (void *a, void *b)
{
  int tmp = *(int *)a;
  *(int *)a = *(int *)b;
  *(int *)b = tmp;
}

void
swap_grid_cell (void *a, void *b)
{
  grid_cell_t tmp = *(grid_cell_t *)a;
  *(grid_cell_t *)a = *(grid_cell_t *)b;
  *(grid_cell_t *)b = tmp;
}
