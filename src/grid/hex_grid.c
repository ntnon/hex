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

static grid_cell_t *get_cell_at_pixel(const grid_t *grid, point_t p) {
  if (!grid) {
    return NULL;
  }

  // Convert pixel to grid coordinates
  static grid_cell_t cell; // Static storage for the cell
  cell = from_pixel(grid, p);

  // Check if the cell is valid within the grid
  if (!is_valid_cell(grid, cell)) {
    return NULL;
  }

  return &cell;
}

static void get_all_cells(const grid_t *grid, grid_cell_t **out_cells,
                          size_t *out_count) {
  if (!grid || !out_cells || !out_count) {
    if (out_cells)
      *out_cells = NULL;
    if (out_count)
      *out_count = 0;
    return;
  }

  int radius = grid->radius;
  size_t capacity = (3 * radius * (radius + 1)) + 1;
  *out_cells = malloc(capacity * sizeof(grid_cell_t));
  if (!*out_cells) {
    *out_count = 0;
    return;
  }

  size_t count = 0;
  for (int q = -radius; q <= radius; q++) {
    int r1 = fmax(-radius, -q - radius);
    int r2 = fmin(radius, -q + radius);
    for (int r = r1; r <= r2; r++) {
      int s = -q - r;
      (*out_cells)[count].type = GRID_TYPE_HEXAGON;
      (*out_cells)[count].coord.hex.q = q;
      (*out_cells)[count].coord.hex.r = r;
      (*out_cells)[count].coord.hex.s = s;
      count++;
    }
  }
  *out_count = count;
}

grid_t *grid_create(grid_type_e type, layout_t layout, int size) {
  grid_t *grid = malloc(sizeof(grid_t));
  if (!grid)
    return NULL;
  grid->type = type;
  grid->layout = layout;
  grid->radius = size;
  grid->initial_radius = size;
  grid->total_growth = 0;

  switch (type) {
  case GRID_TYPE_HEXAGON:
    grid->vtable = &hex_grid_vtable;
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

  if (check_cell.type != grid->type)
    return false;

  if (check_cell.type == GRID_TYPE_HEXAGON) {
    // For hex grid, cell is valid if distance from origin <= radius
    int q = check_cell.coord.hex.q;
    int r = check_cell.coord.hex.r;
    int s = check_cell.coord.hex.s;

    // Validate cube coordinate constraint
    if (q + r + s != 0)
      return false;

    // Check if within radius
    int distance = (abs(q) + abs(r) + abs(s)) / 2;
    return distance <= grid->radius;
  }

  return false;
}

void grid_free(grid_t *grid) {
  if (!grid)
    return;

  free(grid); // Free the grid struct itself
}

grid_cell_t grid_get_center_cell(const grid_t *grid) {
  if (!grid || !grid->vtable || !grid->vtable->get_center_cell) {
    grid_cell_t invalid = {.type = GRID_TYPE_UNKNOWN};
    return invalid;
  }
  return grid->vtable->get_center_cell(grid);
}

grid_cell_t *grid_get_cell_at_pixel(const grid_t *grid, point_t p) {
  if (!grid || !grid->vtable || !grid->vtable->get_cell_at_pixel) {
    return NULL;
  }
  return grid->vtable->get_cell_at_pixel(grid, p);
}

void print_cell(const grid_t *grid, grid_cell_t cell) {
  if (cell.type != GRID_TYPE_HEXAGON)
    printf("Invalid cell");
  if (is_valid_cell(grid, cell)) {
    printf("Hex Coordinate: (%d, %d, %d)\n", cell.coord.hex.q, cell.coord.hex.r,
           cell.coord.hex.s);
  } else {
    printf("Cell not in grid: (%d, %d, %d)\n", cell.coord.hex.q,
           cell.coord.hex.r, cell.coord.hex.s);
  }
}

void grid_get_all_cells(const grid_t *grid, grid_cell_t **out_cells,
                        size_t *out_count) {
  if (!grid || !grid->vtable || !grid->vtable->get_all_cells) {
    if (out_cells)
      *out_cells = NULL;
    if (out_count)
      *out_count = 0;
    return;
  }
  grid->vtable->get_all_cells(grid, out_cells, out_count);
}

bool grid_grow(grid_t *grid, int growth_amount) {
  if (!grid || growth_amount <= 0) {
    return false;
  }

  grid->radius += growth_amount;
  grid->total_growth += growth_amount;
  return true;
}

int grid_get_total_growth(const grid_t *grid) {
  if (!grid) {
    return -1;
  }
  return grid->total_growth;
}

int grid_get_initial_radius(const grid_t *grid) {
  if (!grid) {
    return -1;
  }
  return grid->initial_radius;
}

// --- Public vtable instance ---

const grid_vtable_t hex_grid_vtable = {.to_pixel = to_pixel,
                                       .from_pixel = from_pixel,
                                       .get_neighbor_cell = get_neighbor_cell,
                                       .get_neighbor_cells = get_neighbor_cells,
                                       .distance = distance,
                                       .get_corners = get_corners,
                                       .grid_create = grid_create,
                                       .num_neighbors = 6,
                                       .is_valid_cell = is_valid_cell,
                                       .grid_free = grid_free,
                                       .calculate_offset = calculate_offset,
                                       .apply_offset = apply_offset,
                                       .get_center_cell = get_center_cell,
                                       .get_cell_at_pixel = get_cell_at_pixel,
                                       .print_cell = print_cell,
                                       .get_all_cells = get_all_cells};
