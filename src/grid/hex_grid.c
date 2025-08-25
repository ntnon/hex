#include "../../include/grid/grid_system.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// --- Static (private) implementations ---

static point_t to_pixel(const grid_t *grid, grid_cell_t cell) {
  // Assumes cell.type == GRID_TYPE_HEXAGON
  const hex_coord_t *h = &cell.coord.hex;
  const layout_t *layout = &grid->layout;
  const orientation_t *M = &layout->orientation;
  double x = (M->f0 * h->q + M->f1 * h->r) * layout->size.x;
  double y = (M->f2 * h->q + M->f3 * h->r) * layout->size.y;

  // Apply scaling to the calculated coordinates before adding origin
  return (point_t){(x * layout->scale) + layout->origin.x,
                   (y * layout->scale) + layout->origin.y};
}

static grid_cell_t from_pixel(const grid_t *grid, point_t p) {
  const layout_t *layout = &grid->layout;
  const orientation_t *M = &layout->orientation;

  // Remove origin offset and apply inverse scaling before dividing by size
  point_t pt = {((p.x - layout->origin.x) / layout->scale) / layout->size.x,
                ((p.y - layout->origin.y) / layout->scale) / layout->size.y};

  double q = M->b0 * pt.x + M->b1 * pt.y;
  double r = M->b2 * pt.x + M->b3 * pt.y;
  double s = -q - r;
  // Round to nearest hex
  int qi = (int)round(q), ri = (int)round(r), si = (int)round(s);
  double q_diff = fabs(qi - q), r_diff = fabs(ri - r), s_diff = fabs(si - s);
  if (q_diff > r_diff && q_diff > s_diff)
    qi = -ri - si;
  else if (r_diff > s_diff)
    ri = -qi - si;
  else
    si = -qi - ri;
  grid_cell_t cell = {.type = GRID_TYPE_HEXAGON};
  cell.coord.hex.q = qi;
  cell.coord.hex.r = ri;
  cell.coord.hex.s = si;
  return cell;
}

static void get_neighbor_cell(grid_cell_t cell, int direction,
                              grid_cell_t *out_neighbor) {
  // Hex direction vectors
  static const int dq[6] = {1, 1, 0, -1, -1, 0};
  static const int dr[6] = {0, -1, -1, 0, 1, 1};
  static const int ds[6] = {-1, 0, 1, 1, 0, -1};
  if (cell.type != GRID_TYPE_HEXAGON || direction < 0 || direction > 5)
    return;
  *out_neighbor = cell;
  out_neighbor->coord.hex.q += dq[direction];
  out_neighbor->coord.hex.r += dr[direction];
  out_neighbor->coord.hex.s += ds[direction];
}

static void get_neighbor_cells(grid_cell_t cell, grid_cell_t neighbors[6]) {
  for (int dir = 0; dir < 6; ++dir) {
    get_neighbor_cell(cell, dir, &neighbors[dir]);
  }
}

static int distance(grid_cell_t a, grid_cell_t b) {
  if (a.type != GRID_TYPE_HEXAGON || b.type != GRID_TYPE_HEXAGON)
    return -1;
  int dq = abs(a.coord.hex.q - b.coord.hex.q);
  int dr = abs(a.coord.hex.r - b.coord.hex.r);
  int ds = abs(a.coord.hex.s - b.coord.hex.s);
  return (dq + dr + ds) / 2;
}

static void get_corners(const grid_t *grid, grid_cell_t cell,
                        point_t corners[6]) {
  const layout_t *layout = &grid->layout;
  point_t center = to_pixel(grid, cell);
  double angle_offset = layout->orientation.start_angle;
  for (int i = 0; i < 6; i++) {
    double angle = 2.0 * M_PI * (angle_offset - i) / 6.0;
    // Apply scaling to the corner offsets
    corners[i].x = center.x + (layout->size.x * layout->scale) * cos(angle);
    corners[i].y = center.y + (layout->size.y * layout->scale) * sin(angle);
  }
}

static grid_cell_t calculate_offset(grid_cell_t target, grid_cell_t source) {
  grid_cell_t offset = {0};
  offset.type = target.type;

  if (target.type == GRID_TYPE_HEXAGON && source.type == GRID_TYPE_HEXAGON) {
    offset.coord.hex.q = target.coord.hex.q - source.coord.hex.q;
    offset.coord.hex.r = target.coord.hex.r - source.coord.hex.r;
    offset.coord.hex.s = target.coord.hex.s - source.coord.hex.s;
  }

  return offset;
}

static grid_cell_t apply_offset(grid_cell_t cell, grid_cell_t offset) {
  grid_cell_t result = cell;

  if (cell.type == GRID_TYPE_HEXAGON && offset.type == GRID_TYPE_HEXAGON) {
    result.coord.hex.q += offset.coord.hex.q;
    result.coord.hex.r += offset.coord.hex.r;
    result.coord.hex.s += offset.coord.hex.s;
  }

  return result;
}

static grid_cell_t get_center_cell(const grid_t *grid) {
  grid_cell_t center = {0};
  center.type = GRID_TYPE_HEXAGON;
  center.coord.hex.q = 0;
  center.coord.hex.r = 0;
  center.coord.hex.s = 0;
  return center;
}

static void generate_cells(grid_t *grid, int radius) {
  // Generates all hexes within the given radius
  size_t count = 0;
  int capacity = (3 * radius * (radius + 1)) + 1;
  grid->cells = malloc(capacity * sizeof(grid_cell_t));
  if (!grid->cells)
    return;
  for (int q = -radius; q <= radius; q++) {
    int r1 = fmax(-radius, -q - radius);
    int r2 = fmin(radius, -q + radius);
    for (int r = r1; r <= r2; r++) {
      int s = -q - r;
      grid->cells[count].type = GRID_TYPE_HEXAGON;
      grid->cells[count].coord.hex.q = q;
      grid->cells[count].coord.hex.r = r;
      grid->cells[count].coord.hex.s = s;
      count++;
    }
  }
  grid->num_cells = count;
}

grid_t *grid_create(grid_type_e type, layout_t layout, int size) {
  grid_t *grid = malloc(sizeof(grid_t));
  if (!grid)
    return NULL;
  grid->type = type;
  grid->layout = layout;
  grid->cells = NULL;
  grid->num_cells = 0;

  switch (type) {
  case GRID_TYPE_HEXAGON:
    grid->vtable = &hex_grid_vtable;
    grid->vtable->generate_cells(grid, size);

    break;
  // Add cases for other grid types as you implement them
  default:
    free(grid);
    return NULL;
  }
  return grid;
}

bool is_valid_cell(const grid_t *grid, grid_cell_t check_cell) {
  if (!grid)
    return false;

  if (check_cell.type != grid->cells->type)
    return false;

  for (size_t i = 0; i < grid->num_cells; ++i) {
    grid_cell_t cell = grid->cells[i];
    if (cell.coord.hex.s == check_cell.coord.hex.s &&
        cell.coord.hex.q == check_cell.coord.hex.q)
      return true;
  }
  return false;
}

void grid_free(grid_t *grid) {
  if (!grid)
    return;

  free(grid->cells); // Free the cells array
  free(grid);        // Free the grid struct itself
}

// --- Public vtable instance ---

const grid_vtable_t hex_grid_vtable = {
  .to_pixel = to_pixel,
  .from_pixel = from_pixel,
  .get_neighbor_cell = get_neighbor_cell,
  .get_neighbor_cells = get_neighbor_cells,
  .distance = distance,
  .get_corners = get_corners,
  .generate_cells = generate_cells,
  .grid_create = grid_create,
  .num_neighbors = 6,
  .is_valid_cell = is_valid_cell,
  .grid_free = grid_free,
  .calculate_offset = calculate_offset,
  .apply_offset = apply_offset,
  .get_center_cell = get_center_cell,
};

grid_cell_t grid_get_center_cell(const grid_t *grid) {
  if (!grid || !grid->vtable || !grid->vtable->get_center_cell) {
    grid_cell_t invalid = {.type = GRID_TYPE_UNKNOWN};
    return invalid;
  }
  return grid->vtable->get_center_cell(grid);
}

/*
const orientation_t layout_pointy = { .f0 = 1.732050808,
                                      .f1 = 0.866025404,
                                      .f2 = 0.0,
                                      .f3 = 1.5,
                                      .b0 = 0.577350269,
                                      .b1 = -0.333333333,
                                      .b2 = 0.0,
                                      .b3 = 0.666666667,
                                      .start_angle = 0.5 };

 */
