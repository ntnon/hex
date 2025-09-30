#include "../../include/grid/grid_geometry.h"
#include "ui.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
  // Hex direction vectors
  static const int dq[6] = {1, 1, 0, -1, -1, 0};
  static const int dr[6] = {0, -1, -1, 0, 1, 1};
  static const int ds[6] = {-1, 0, 1, 1, 0, -1};

  if (cell.type != GRID_TYPE_HEXAGON)
    return;

  for (int dir = 0; dir < 6; ++dir) {
    neighbors[dir] = cell;
    neighbors[dir].coord.hex.q += dq[dir];
    neighbors[dir].coord.hex.r += dr[dir];
    neighbors[dir].coord.hex.s += ds[dir];
  }
}

static void get_cells_in_range(const grid_t *grid, grid_cell_t center,
                               int range, grid_cell_t **out_cells,
                               size_t *out_count) {
  if (!grid || !out_cells || !out_count || range < 0) {
    if (out_cells)
      *out_cells = NULL;
    if (out_count)
      *out_count = 0;
    return;
  }

  if (center.type != GRID_TYPE_HEXAGON) {
    *out_cells = NULL;
    *out_count = 0;
    return;
  }

  // Estimate capacity: range 0 = 1 cell, range 1 = 7 cells, range 2 = 19 cells,
  // etc.
  size_t estimated_capacity = 3 * range * (range + 1) + 1;
  *out_cells = malloc(estimated_capacity * sizeof(grid_cell_t));
  if (!*out_cells) {
    *out_count = 0;
    return;
  }

  size_t count = 0;
  int center_q = center.coord.hex.q;
  int center_r = center.coord.hex.r;

  // Generate all coordinates within range using hex coordinate math
  for (int dq = -range; dq <= range; dq++) {
    int dr1 = fmax(-range, -dq - range);
    int dr2 = fmin(range, -dq + range);

    for (int dr = dr1; dr <= dr2; dr++) {
      int ds = -dq - dr;

      grid_cell_t candidate = {0};
      candidate.type = GRID_TYPE_HEXAGON;
      candidate.coord.hex.q = center_q + dq;
      candidate.coord.hex.r = center_r + dr;
      candidate.coord.hex.s = center.coord.hex.s + ds;

      // Only include if within grid bounds
      if (is_valid_cell(grid, candidate)) {
        (*out_cells)[count] = candidate;
        count++;
      }
    }
  }

  *out_count = count;

  // Resize to exact size if we allocated more than needed
  if (count < estimated_capacity) {
    grid_cell_t *resized = realloc(*out_cells, count * sizeof(grid_cell_t));
    if (resized || count == 0) {
      *out_cells = resized;
    }
  }
}

static bool rotate_coordinates(const grid_cell_t *coordinates, size_t count,
                               grid_cell_t center, int rotation_steps,
                               grid_cell_t **out_rotated) {
  if (!coordinates || count == 0 || !out_rotated) {
    if (out_rotated)
      *out_rotated = NULL;
    return false;
  }

  // Normalize rotation steps to 0-5 range
  rotation_steps = ((rotation_steps % 6) + 6) % 6;
  if (rotation_steps == 0) {
    // No rotation needed, just copy the coordinates
    *out_rotated = malloc(count * sizeof(grid_cell_t));
    if (!*out_rotated)
      return false;
    memcpy(*out_rotated, coordinates, count * sizeof(grid_cell_t));
    return true;
  }

  // Allocate output array
  *out_rotated = malloc(count * sizeof(grid_cell_t));
  if (!*out_rotated) {
    return false;
  }

  // Rotate each coordinate
  for (size_t i = 0; i < count; i++) {
    if (coordinates[i].type != GRID_TYPE_HEXAGON ||
        center.type != GRID_TYPE_HEXAGON) {
      free(*out_rotated);
      *out_rotated = NULL;
      return false;
    }

    // Convert to coordinates relative to center
    int rel_q = coordinates[i].coord.hex.q - center.coord.hex.q;
    int rel_r = coordinates[i].coord.hex.r - center.coord.hex.r;
    int rel_s = coordinates[i].coord.hex.s - center.coord.hex.s;

    // Apply rotation transformation (repeated rotation_steps times)
    for (int step = 0; step < rotation_steps; step++) {
      // 60-degree clockwise rotation: (q,r,s) -> (-r,-s,-q)
      int temp_q = -rel_r;
      int temp_r = -rel_s;
      int temp_s = -rel_q;

      rel_q = temp_q;
      rel_r = temp_r;
      rel_s = temp_s;
    }

    // Convert back to absolute coordinates
    (*out_rotated)[i] =
      (grid_cell_t){.type = GRID_TYPE_HEXAGON,
                    .coord.hex = {.q = rel_q + center.coord.hex.q,
                                  .r = rel_r + center.coord.hex.r,
                                  .s = rel_s + center.coord.hex.s}};
  }

  return true;
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
    double angle = 2.0 * M_PI * (angle_offset + i) / 6.0;
    // Don't apply scaling here since center is already scaled in to_pixel
    corners[i].x = center.x + layout->size.x * cos(angle);
    corners[i].y = center.y + layout->size.y * sin(angle);
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

static bool get_hex_mesh(const grid_t *grid, float **vertices,
                         unsigned int **indices, size_t *vertex_count,
                         size_t *index_count) {
  if (!grid || !vertices || !indices || !vertex_count || !index_count) {
    return false;
  }

  // Create canonical hex mesh centered at origin with unit size
  // Vertices for a regular hexagon (6 vertices + center for triangle fan)
  static float hex_vertices[] = {
    // Center vertex
    0.0f, 0.0f,
    // Corner vertices (unit circle, starting from right, counter-clockwise)
    1.0f, 0.0f,        // Right
    0.5f, 0.866025f,   // Top-right
    -0.5f, 0.866025f,  // Top-left
    -1.0f, 0.0f,       // Left
    -0.5f, -0.866025f, // Bottom-left
    0.5f, -0.866025f   // Bottom-right
  };

  // Triangle fan indices (center + 6 triangles)
  static unsigned int hex_indices[] = {
    0, 1, 2, // Triangle 1: center, right, top-right
    0, 2, 3, // Triangle 2: center, top-right, top-left
    0, 3, 4, // Triangle 3: center, top-left, left
    0, 4, 5, // Triangle 4: center, left, bottom-left
    0, 5, 6, // Triangle 5: center, bottom-left, bottom-right
    0, 6, 1  // Triangle 6: center, bottom-right, right
  };

  *vertices = hex_vertices;
  *indices = hex_indices;
  *vertex_count = 14; // 7 vertices * 2 coordinates each
  *index_count = 18;  // 6 triangles * 3 indices each

  return true;
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

void grid_get_cells_in_range(const grid_t *grid, grid_cell_t center, int range,
                             grid_cell_t **out_cells, size_t *out_count) {
  if (!grid || !grid->vtable || !grid->vtable->get_cells_in_range) {
    if (out_cells)
      *out_cells = NULL;
    if (out_count)
      *out_count = 0;
    return;
  }
  grid->vtable->get_cells_in_range(grid, center, range, out_cells, out_count);
}

grid_cell_t grid_apply_offset(grid_cell_t cell, grid_cell_t offset) {
  // Use the hex grid implementation directly for now
  // TODO: Make this geometry-agnostic when we add other grid types
  return apply_offset(cell, offset);
}

bool grid_rotate_coordinates(const grid_cell_t *coordinates, size_t count,
                             grid_cell_t center, int rotation_steps,
                             grid_cell_t **out_rotated) {
  if (!coordinates || count == 0 || !out_rotated) {
    if (out_rotated)
      *out_rotated = NULL;
    return false;
  }

  // Use the hex grid vtable implementation directly
  return rotate_coordinates(coordinates, count, center, rotation_steps,
                            out_rotated);
}

bool grid_is_valid_cell_with_radius(grid_cell_t cell, int radius) {
  if (radius < 0)
    return false;

  if (cell.type == GRID_TYPE_HEXAGON) {
    // Validate cube coordinate constraint
    if (cell.coord.hex.q + cell.coord.hex.r + cell.coord.hex.s != 0)
      return false;

    // Check if within radius using hex distance
    int distance =
      (abs(cell.coord.hex.q) + abs(cell.coord.hex.r) + abs(cell.coord.hex.s)) /
      2;
    return distance <= radius;
  }

  // Add other geometry types here as needed
  return false;
}

int grid_distance(grid_cell_t a, grid_cell_t b) {
  if (a.type != b.type)
    return -1;

  if (a.type == GRID_TYPE_HEXAGON) {
    int dq = abs(a.coord.hex.q - b.coord.hex.q);
    int dr = abs(a.coord.hex.r - b.coord.hex.r);
    int ds = abs(a.coord.hex.s - b.coord.hex.s);
    return (dq + dr + ds) / 2;
  }

  // Add other geometry types here as needed
  return -1;
}

grid_cell_t grid_get_center_cell(grid_type_e geometry_type) {
  if (geometry_type == GRID_TYPE_HEXAGON) {
    return (grid_cell_t){.type = GRID_TYPE_HEXAGON, .coord.hex = {0, 0, 0}};
  }

  // Add other geometry types as needed
  return (grid_cell_t){.type = GRID_TYPE_UNKNOWN};
}

grid_cell_t grid_calculate_offset(grid_cell_t target, grid_cell_t source) {
  if (target.type != source.type) {
    return (grid_cell_t){.type = GRID_TYPE_UNKNOWN};
  }

  if (target.type == GRID_TYPE_HEXAGON) {
    grid_cell_t offset = {.type = GRID_TYPE_HEXAGON};
    offset.coord.hex.q = target.coord.hex.q - source.coord.hex.q;
    offset.coord.hex.r = target.coord.hex.r - source.coord.hex.r;
    offset.coord.hex.s = target.coord.hex.s - source.coord.hex.s;
    return offset;
  }

  // Add other geometry types as needed
  return (grid_cell_t){.type = GRID_TYPE_UNKNOWN};
}

bool grid_get_all_coordinates_in_radius(grid_type_e geometry_type, int radius,
                                        grid_cell_t **out_cells,
                                        size_t *out_count) {
  if (!out_cells || !out_count || radius < 0) {
    if (out_cells)
      *out_cells = NULL;
    if (out_count)
      *out_count = 0;
    return false;
  }

  if (geometry_type == GRID_TYPE_HEXAGON) {
    // Calculate hex grid coordinates within radius
    size_t capacity = (3 * radius * (radius + 1)) + 1;
    *out_cells = malloc(capacity * sizeof(grid_cell_t));
    if (!*out_cells) {
      *out_count = 0;
      return false;
    }

    size_t count = 0;
    for (int q = -radius; q <= radius; q++) {
      int r1 = fmax(-radius, -q - radius);
      int r2 = fmin(radius, -q + radius);
      for (int r = r1; r <= r2; r++) {
        int s = -q - r;
        (*out_cells)[count] =
          (grid_cell_t){.type = GRID_TYPE_HEXAGON, .coord.hex = {q, r, s}};
        count++;
      }
    }
    *out_count = count;
    return true;
  }

  // Add other geometry types as needed
  *out_cells = NULL;
  *out_count = 0;
  return false;
}

bool grid_pixel_to_cell(grid_type_e geometry_type, const layout_t *layout,
                        int radius, point_t p, grid_cell_t *out_cell) {
  if (!layout || !out_cell)
    return false;

  if (geometry_type == GRID_TYPE_HEXAGON) {
    // Use hex pixel to coordinate conversion
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

    *out_cell =
      (grid_cell_t){.type = GRID_TYPE_HEXAGON, .coord.hex = {qi, ri, si}};

    // Check if within bounds
    return grid_is_valid_cell_with_radius(*out_cell, radius);
  }

  // Add other geometry types as needed
  return false;
}

void grid_get_cell_corners(grid_type_e geometry_type, const layout_t *layout,
                           grid_cell_t cell, point_t corners[]) {
  if (!layout || !corners)
    return;

  if (geometry_type == GRID_TYPE_HEXAGON && cell.type == GRID_TYPE_HEXAGON) {
    // Calculate center position
    const orientation_t *M = &layout->orientation;
    const hex_coord_t *h = &cell.coord.hex;
    double x = (M->f0 * h->q + M->f1 * h->r) * layout->size.x;
    double y = (M->f2 * h->q + M->f3 * h->r) * layout->size.y;
    point_t center = {(x * layout->scale) + layout->origin.x,
                      (y * layout->scale) + layout->origin.y};

    // Calculate corner positions
    double angle_offset = layout->orientation.start_angle;
    for (int i = 0; i < 6; i++) {
      double angle = 2.0 * M_PI * (angle_offset + i) / 6.0;
      corners[i].x = center.x + layout->size.x * layout->scale * cos(angle);
      corners[i].y = center.y + layout->size.y * layout->scale * sin(angle);
    }
  }

  // Add other geometry types as needed
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
                                       .get_cells_in_range = get_cells_in_range,
                                       .rotate_coordinates = rotate_coordinates,
                                       .distance = distance,
                                       .get_corners = get_corners,
                                       .get_all_cells = get_all_cells,
                                       .get_hex_mesh = get_hex_mesh,
                                       .grid_create = grid_create,
                                       .num_neighbors = 6,
                                       .is_valid_cell = is_valid_cell,
                                       .grid_free = grid_free,
                                       .calculate_offset = calculate_offset,
                                       .apply_offset = apply_offset,
                                       .get_center_cell = get_center_cell,
                                       .get_cell_at_pixel = get_cell_at_pixel,
                                       .print_cell = print_cell};
