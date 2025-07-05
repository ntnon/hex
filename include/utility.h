#include "../include/grid/grid_types.h"
#include <stdlib.h> // for rand()
#include <time.h>   // for seeding rand()

void
shuffle_array (void *array, size_t n, size_t size,
               void (*swap) (void *, void *));

// Example swap function for int arrays
void
swap_int (void *a, void *b);

void
swap_grid_cell (void *a, void *b);

void print_cell(grid_cell_t cell);
