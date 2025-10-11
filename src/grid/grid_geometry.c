#include "../../include/grid/grid_geometry.h"
#include <stddef.h>
#include <stdlib.h>

// Static storage for registered grid implementations
static const grid_vtable_t *grid_vtables[4] = {NULL, NULL, NULL, NULL};

// External vtable declarations (defined in implementation files)
extern const grid_vtable_t hex_grid_vtable;
// extern const grid_vtable_t square_grid_vtable;
// extern const grid_vtable_t triangle_grid_vtable;

void grid_geometry_register_hex(void) {
  grid_vtables[GRID_TYPE_HEXAGON] = &hex_grid_vtable;
}

void grid_geometry_register_square(void) {
  // grid_vtables[GRID_TYPE_SQUARE] = &square_grid_vtable;
}

void grid_geometry_register_triangle(void) {
  // grid_vtables[GRID_TYPE_TRIANGLE] = &triangle_grid_vtable;
}

void grid_geometry_init(void) {
  grid_geometry_register_hex();
  // grid_geometry_register_square();
  // grid_geometry_register_triangle();
}

const grid_vtable_t *grid_geometry_get_vtable(grid_type_e type) {
  if (type < 0 || type >= GRID_TYPE_UNKNOWN) {
    return NULL;
  }
  return grid_vtables[type];
}

// --- Dispatch functions ---

point_t grid_geometry_cell_to_pixel(grid_type_e type, const layout_t *layout,
                                    grid_cell_t cell) {
  const grid_vtable_t *vtable = grid_geometry_get_vtable(type);
  if (!vtable || !vtable->cell_to_pixel) {
    return (point_t){0, 0};
  }
  return vtable->cell_to_pixel(layout, cell);
}

grid_cell_t grid_geometry_pixel_to_cell(grid_type_e type,
                                        const layout_t *layout, point_t p) {
  const grid_vtable_t *vtable = grid_geometry_get_vtable(type);
  if (!vtable || !vtable->pixel_to_cell) {
    return (grid_cell_t){.type = GRID_TYPE_UNKNOWN};
  }
  return vtable->pixel_to_cell(layout, p);
}

void grid_geometry_get_neighbor(grid_type_e type, grid_cell_t cell,
                                int direction, grid_cell_t *out_neighbor) {
  const grid_vtable_t *vtable = grid_geometry_get_vtable(type);
  if (!vtable || !vtable->get_neighbor) {
    return;
  }
  vtable->get_neighbor(cell, direction, out_neighbor);
}

void grid_geometry_get_all_neighbors(grid_type_e type, grid_cell_t cell,
                                     grid_cell_t *out_neighbors) {
  const grid_vtable_t *vtable = grid_geometry_get_vtable(type);
  if (!vtable || !vtable->get_all_neighbors) {
    return;
  }
  vtable->get_all_neighbors(cell, out_neighbors);
}

void grid_geometry_get_cells_in_range(grid_type_e type, grid_cell_t center,
                                      int range, grid_cell_t **out_cells,
                                      size_t *out_count) {
  const grid_vtable_t *vtable = grid_geometry_get_vtable(type);
  if (!vtable || !vtable->get_cells_in_range) {
    if (out_cells)
      *out_cells = NULL;
    if (out_count)
      *out_count = 0;
    return;
  }
  vtable->get_cells_in_range(center, range, out_cells, out_count);
}

int grid_geometry_distance(grid_type_e type, grid_cell_t a, grid_cell_t b) {
  const grid_vtable_t *vtable = grid_geometry_get_vtable(type);
  if (!vtable || !vtable->distance) {
    return -1;
  }
  return vtable->distance(a, b);
}

void grid_geometry_get_corners(grid_type_e type, const layout_t *layout,
                               grid_cell_t cell, point_t *corners) {
  const grid_vtable_t *vtable = grid_geometry_get_vtable(type);
  if (!vtable || !vtable->get_corners) {
    return;
  }
  vtable->get_corners(layout, cell, corners);
}

bool grid_geometry_rotate_cell(grid_type_e type, grid_cell_t cell,
                               int rotations, grid_cell_t *out_cell) {
  const grid_vtable_t *vtable = grid_geometry_get_vtable(type);
  if (!vtable || !vtable->rotate_cell) {
    return false;
  }
  return vtable->rotate_cell(cell, rotations, out_cell);
}

grid_cell_t grid_geometry_get_origin(grid_type_e type) {
  const grid_vtable_t *vtable = grid_geometry_get_vtable(type);
  if (!vtable || !vtable->get_origin) {
    return (grid_cell_t){.type = GRID_TYPE_UNKNOWN};
  }
  return vtable->get_origin();
}

grid_cell_t grid_geometry_calculate_offset(grid_type_e type, grid_cell_t from,
                                           grid_cell_t to) {
  const grid_vtable_t *vtable = grid_geometry_get_vtable(type);
  if (!vtable || !vtable->calculate_offset) {
    return (grid_cell_t){.type = GRID_TYPE_UNKNOWN};
  }
  return vtable->calculate_offset(from, to);
}

grid_cell_t grid_geometry_apply_offset(grid_type_e type, grid_cell_t cell,
                                       grid_cell_t offset) {
  const grid_vtable_t *vtable = grid_geometry_get_vtable(type);
  if (!vtable || !vtable->apply_offset) {
    return (grid_cell_t){.type = GRID_TYPE_UNKNOWN};
  }
  return vtable->apply_offset(cell, offset);
}

void grid_geometry_get_ring(grid_type_e type, grid_cell_t center, int radius,
                            grid_cell_t **out_cells, size_t *out_count) {
  const grid_vtable_t *vtable = grid_geometry_get_vtable(type);
  if (!vtable || !vtable->get_ring) {
    if (out_cells)
      *out_cells = NULL;
    if (out_count)
      *out_count = 0;
    return;
  }
  vtable->get_ring(center, radius, out_cells, out_count);
}

void grid_geometry_get_line(grid_type_e type, grid_cell_t start,
                            grid_cell_t end, grid_cell_t **out_cells,
                            size_t *out_count) {
  const grid_vtable_t *vtable = grid_geometry_get_vtable(type);
  if (!vtable || !vtable->get_line) {
    if (out_cells)
      *out_cells = NULL;
    if (out_count)
      *out_count = 0;
    return;
  }
  vtable->get_line(start, end, out_cells, out_count);
}

bool grid_geometry_cells_equal(grid_type_e type, grid_cell_t a, grid_cell_t b) {
  const grid_vtable_t *vtable = grid_geometry_get_vtable(type);
  if (!vtable || !vtable->cells_equal) {
    return false;
  }
  return vtable->cells_equal(a, b);
}

bool grid_geometry_get_cell_mesh(grid_type_e type, const layout_t *layout,
                                 float **vertices, unsigned int **indices,
                                 size_t *vertex_count, size_t *index_count) {
  const grid_vtable_t *vtable = grid_geometry_get_vtable(type);
  if (!vtable || !vtable->get_cell_mesh) {
    return false;
  }
  return vtable->get_cell_mesh(layout, vertices, indices, vertex_count,
                               index_count);
}

int grid_geometry_get_neighbor_count(grid_type_e type) {
  const grid_vtable_t *vtable = grid_geometry_get_vtable(type);
  if (!vtable) {
    return 0;
  }
  return vtable->neighbor_count;
}

int grid_geometry_get_corner_count(grid_type_e type) {
  const grid_vtable_t *vtable = grid_geometry_get_vtable(type);
  if (!vtable) {
    return 0;
  }
  return vtable->corner_count;
}

// --- Utility functions for collections ---

int grid_geometry_calculate_diameter(grid_type_e type, grid_cell_t *cells,
                                     size_t cell_count) {
  if (!cells || cell_count < 2) {
    return 0;
  }

  const grid_vtable_t *vtable = grid_geometry_get_vtable(type);
  if (!vtable || !vtable->distance) {
    return 0;
  }

  int max_distance = 0;
  for (size_t i = 0; i < cell_count; i++) {
    for (size_t j = i + 1; j < cell_count; j++) {
      int dist = vtable->distance(cells[i], cells[j]);
      if (dist > max_distance) {
        max_distance = dist;
      }
    }
  }

  return max_distance;
}

grid_cell_t grid_geometry_calculate_center(grid_type_e type, grid_cell_t *cells,
                                           size_t cell_count) {
  if (!cells || cell_count == 0) {
    return (grid_cell_t){.type = GRID_TYPE_UNKNOWN};
  }

  // For hex grids, we can average the coordinates
  // This is a simplistic approach that works for hex
  // Other grid types might need different approaches
  if (type == GRID_TYPE_HEXAGON) {
    int sum_q = 0, sum_r = 0, sum_s = 0;
    for (size_t i = 0; i < cell_count; i++) {
      if (cells[i].type != GRID_TYPE_HEXAGON) {
        return (grid_cell_t){.type = GRID_TYPE_UNKNOWN};
      }
      sum_q += cells[i].coord.hex.q;
      sum_r += cells[i].coord.hex.r;
      sum_s += cells[i].coord.hex.s;
    }

    grid_cell_t center = {.type = GRID_TYPE_HEXAGON};
    center.coord.hex.q = sum_q / (int)cell_count;
    center.coord.hex.r = sum_r / (int)cell_count;
    center.coord.hex.s = -(center.coord.hex.q + center.coord.hex.r);
    return center;
  }

  // Default: return first cell as a fallback
  return cells[0];
}

int grid_geometry_count_external_edges(grid_type_e type, grid_cell_t *cells,
                                       size_t cell_count) {
  if (!cells || cell_count == 0) {
    return 0;
  }

  const grid_vtable_t *vtable = grid_geometry_get_vtable(type);
  if (!vtable) {
    return 0;
  }

  int neighbor_count = vtable->neighbor_count;
  int external_edges = 0;

  // For each cell, check how many neighbors are NOT in the collection
  for (size_t i = 0; i < cell_count; i++) {
    grid_cell_t *neighbors = malloc(neighbor_count * sizeof(grid_cell_t));
    if (!neighbors)
      continue;

    vtable->get_all_neighbors(cells[i], neighbors);

    for (int n = 0; n < neighbor_count; n++) {
      bool found = false;
      for (size_t j = 0; j < cell_count; j++) {
        if (vtable->cells_equal(neighbors[n], cells[j])) {
          found = true;
          break;
        }
      }
      if (!found) {
        external_edges++;
      }
    }

    free(neighbors);
  }

  return external_edges;
}

int grid_geometry_count_internal_edges(grid_type_e type, grid_cell_t *cells,
                                       size_t cell_count) {
  if (!cells || cell_count == 0) {
    return 0;
  }

  const grid_vtable_t *vtable = grid_geometry_get_vtable(type);
  if (!vtable) {
    return 0;
  }

  int neighbor_count = vtable->neighbor_count;
  int internal_edges = 0;

  // For each cell, check how many neighbors ARE in the collection
  for (size_t i = 0; i < cell_count; i++) {
    grid_cell_t *neighbors = malloc(neighbor_count * sizeof(grid_cell_t));
    if (!neighbors)
      continue;

    vtable->get_all_neighbors(cells[i], neighbors);

    for (int n = 0; n < neighbor_count; n++) {
      for (size_t j = 0; j < cell_count; j++) {
        if (vtable->cells_equal(neighbors[n], cells[j])) {
          internal_edges++;
          break;
        }
      }
    }

    free(neighbors);
  }

  // Each internal edge is counted twice (once from each side)
  return internal_edges / 2;
}
