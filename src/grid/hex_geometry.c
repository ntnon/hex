#include "../../include/grid/grid_geometry.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- Hex-specific definitions (private to this implementation) ---

/**
 * @brief Hexagonal grid edge directions (clockwise from East).
 */
typedef enum {
  HEX_EDGE_E = 0,  // East
  HEX_EDGE_NE = 1, // Northeast
  HEX_EDGE_NW = 2, // Northwest
  HEX_EDGE_W = 3,  // West
  HEX_EDGE_SW = 4, // Southwest
  HEX_EDGE_SE = 5  // Southeast
} hex_edge_direction_t;

/**
 * @brief Hexagonal grid vertex directions (corners, clockwise from Northeast).
 */
typedef enum {
  HEX_VERTEX_NE = 0, // Northeast corner
  HEX_VERTEX_N = 1,  // North corner
  HEX_VERTEX_NW = 2, // Northwest corner
  HEX_VERTEX_SW = 3, // Southwest corner
  HEX_VERTEX_S = 4,  // South corner
  HEX_VERTEX_SE = 5  // Southeast corner
} hex_vertex_direction_t;

// --- Static helper functions ---

static int hex_distance_internal(hex_coord_t a, hex_coord_t b) {
  return (abs(a.q - b.q) + abs(a.q + a.r - b.q - b.r) + abs(a.r - b.r)) / 2;
}

static hex_coord_t hex_round(double fq, double fr, double fs) {
  int q = (int)round(fq);
  int r = (int)round(fr);
  int s = (int)round(fs);

  double q_diff = fabs(q - fq);
  double r_diff = fabs(r - fr);
  double s_diff = fabs(s - fs);

  if (q_diff > r_diff && q_diff > s_diff) {
    q = -r - s;
  } else if (r_diff > s_diff) {
    r = -q - s;
  } else {
    s = -q - r;
  }

  return (hex_coord_t){q, r, s};
}

// --- Implementation of vtable functions ---

static point_t hex_cell_to_pixel(const layout_t *layout, grid_cell_t cell) {
  if (cell.type != GRID_TYPE_HEXAGON) {
    return (point_t){0, 0};
  }

  const hex_coord_t *h = &cell.coord.hex;
  const orientation_t *M = &layout->orientation;

  double x = (M->f0 * h->q + M->f1 * h->r) * layout->size.x;
  double y = (M->f2 * h->q + M->f3 * h->r) * layout->size.y;

  return (point_t){(x * layout->scale) + layout->origin.x,
                   (y * layout->scale) + layout->origin.y};
}

static grid_cell_t hex_pixel_to_cell(const layout_t *layout, point_t p) {
  const orientation_t *M = &layout->orientation;

  point_t pt = {((p.x - layout->origin.x) / layout->scale) / layout->size.x,
                ((p.y - layout->origin.y) / layout->scale) / layout->size.y};

  double q = M->b0 * pt.x + M->b1 * pt.y;
  double r = M->b2 * pt.x + M->b3 * pt.y;
  double s = -q - r;

  hex_coord_t hex = hex_round(q, r, s);

  grid_cell_t cell = {.type = GRID_TYPE_HEXAGON};
  cell.coord.hex = hex;
  return cell;
}

static void hex_get_neighbor(grid_cell_t cell, int direction,
                             grid_cell_t *out_neighbor) {
  static const int dq[6] = {1, 1, 0, -1, -1, 0};
  static const int dr[6] = {0, -1, -1, 0, 1, 1};
  static const int ds[6] = {-1, 0, 1, 1, 0, -1};

  if (!out_neighbor || cell.type != GRID_TYPE_HEXAGON || direction < 0 ||
      direction >= 6) {
    return;
  }

  *out_neighbor = cell;
  out_neighbor->coord.hex.q += dq[direction];
  out_neighbor->coord.hex.r += dr[direction];
  out_neighbor->coord.hex.s += ds[direction];
}

static void hex_get_all_neighbors(grid_cell_t cell,
                                  grid_cell_t *out_neighbors) {
  if (!out_neighbors || cell.type != GRID_TYPE_HEXAGON) {
    return;
  }

  for (int dir = 0; dir < 6; dir++) {
    hex_get_neighbor(cell, dir, &out_neighbors[dir]);
  }
}

static void hex_get_cells_in_range(grid_cell_t center, int range,
                                   grid_cell_t **out_cells, size_t *out_count) {
  if (!out_cells || !out_count || range < 0) {
    if (out_count)
      *out_count = 0;
    if (out_cells)
      *out_cells = NULL;
    return;
  }

  if (center.type != GRID_TYPE_HEXAGON) {
    *out_count = 0;
    *out_cells = NULL;
    return;
  }

  // Calculate total cells in range
  size_t total = 3 * range * (range + 1) + 1;
  *out_cells = malloc(total * sizeof(grid_cell_t));
  if (!*out_cells) {
    *out_count = 0;
    return;
  }

  size_t count = 0;
  hex_coord_t c = center.coord.hex;

  for (int q = -range; q <= range; q++) {
    for (int r = -range; r <= range; r++) {
      int s = -q - r;
      if (abs(s) <= range) {
        (*out_cells)[count].type = GRID_TYPE_HEXAGON;
        (*out_cells)[count].coord.hex.q = c.q + q;
        (*out_cells)[count].coord.hex.r = c.r + r;
        (*out_cells)[count].coord.hex.s = c.s + s;
        count++;
      }
    }
  }

  *out_count = count;
}

static bool hex_rotate_cell(grid_cell_t cell, int rotations,
                            grid_cell_t *out_cell) {
  if (!out_cell || cell.type != GRID_TYPE_HEXAGON) {
    return false;
  }

  hex_coord_t h = cell.coord.hex;
  rotations = ((rotations % 6) + 6) % 6; // Normalize to 0-5

  for (int i = 0; i < rotations; i++) {
    int new_q = -h.r;
    int new_r = -h.s;
    int new_s = -h.q;
    h.q = new_q;
    h.r = new_r;
    h.s = new_s;
  }

  out_cell->type = GRID_TYPE_HEXAGON;
  out_cell->coord.hex = h;
  return true;
}

static int hex_distance(grid_cell_t a, grid_cell_t b) {
  if (a.type != GRID_TYPE_HEXAGON || b.type != GRID_TYPE_HEXAGON) {
    return -1;
  }
  return hex_distance_internal(a.coord.hex, b.coord.hex);
}

static void hex_get_corners(const layout_t *layout, grid_cell_t cell,
                            point_t *corners) {
  if (!corners || cell.type != GRID_TYPE_HEXAGON) {
    return;
  }

  point_t center = hex_cell_to_pixel(layout, cell);
  const orientation_t *M = &layout->orientation;

  for (int i = 0; i < 6; i++) {
    double angle = 2.0 * M_PI * (M->start_angle + i) / 6.0;
    corners[i].x = center.x + layout->size.x * layout->scale * cos(angle);
    corners[i].y = center.y + layout->size.y * layout->scale * sin(angle);
  }
}

static grid_cell_t hex_calculate_offset(grid_cell_t from, grid_cell_t to) {
  grid_cell_t offset = {.type = GRID_TYPE_HEXAGON};

  if (from.type != GRID_TYPE_HEXAGON || to.type != GRID_TYPE_HEXAGON) {
    offset.type = GRID_TYPE_UNKNOWN;
    return offset;
  }

  offset.coord.hex.q = to.coord.hex.q - from.coord.hex.q;
  offset.coord.hex.r = to.coord.hex.r - from.coord.hex.r;
  offset.coord.hex.s = to.coord.hex.s - from.coord.hex.s;

  return offset;
}

static grid_cell_t hex_apply_offset(grid_cell_t cell, grid_cell_t offset) {
  grid_cell_t result = {.type = GRID_TYPE_HEXAGON};

  if (cell.type != GRID_TYPE_HEXAGON || offset.type != GRID_TYPE_HEXAGON) {
    result.type = GRID_TYPE_UNKNOWN;
    return result;
  }

  result.coord.hex.q = cell.coord.hex.q + offset.coord.hex.q;
  result.coord.hex.r = cell.coord.hex.r + offset.coord.hex.r;
  result.coord.hex.s = cell.coord.hex.s + offset.coord.hex.s;

  return result;
}

static grid_cell_t hex_get_origin(void) {
  grid_cell_t origin = {.type = GRID_TYPE_HEXAGON};
  origin.coord.hex.q = 0;
  origin.coord.hex.r = 0;
  origin.coord.hex.s = 0;
  return origin;
}

static void hex_get_ring(grid_cell_t center, int radius,
                         grid_cell_t **out_cells, size_t *out_count) {
  if (!out_cells || !out_count || radius < 0) {
    if (out_count)
      *out_count = 0;
    if (out_cells)
      *out_cells = NULL;
    return;
  }

  if (center.type != GRID_TYPE_HEXAGON) {
    *out_count = 0;
    *out_cells = NULL;
    return;
  }

  if (radius == 0) {
    *out_cells = malloc(sizeof(grid_cell_t));
    if (*out_cells) {
      (*out_cells)[0] = center;
      *out_count = 1;
    } else {
      *out_count = 0;
    }
    return;
  }

  size_t total = 6 * radius;
  *out_cells = malloc(total * sizeof(grid_cell_t));
  if (!*out_cells) {
    *out_count = 0;
    return;
  }

  size_t count = 0;

  // Start at a corner and walk around the ring
  grid_cell_t current = center;
  for (int i = 0; i < radius; i++) {
    hex_get_neighbor(current, 4, &current); // Move SW to get to starting corner
  }

  // Walk around the ring
  for (int side = 0; side < 6; side++) {
    for (int step = 0; step < radius; step++) {
      (*out_cells)[count++] = current;
      hex_get_neighbor(current, side, &current);
    }
  }

  *out_count = count;
}

static void hex_get_line(grid_cell_t start, grid_cell_t end,
                         grid_cell_t **out_cells, size_t *out_count) {
  if (!out_cells || !out_count) {
    if (out_count)
      *out_count = 0;
    if (out_cells)
      *out_cells = NULL;
    return;
  }

  if (start.type != GRID_TYPE_HEXAGON || end.type != GRID_TYPE_HEXAGON) {
    *out_count = 0;
    *out_cells = NULL;
    return;
  }

  int distance = hex_distance(start, end);
  size_t total = distance + 1;

  *out_cells = malloc(total * sizeof(grid_cell_t));
  if (!*out_cells) {
    *out_count = 0;
    return;
  }

  for (int i = 0; i <= distance; i++) {
    double t = distance == 0 ? 0 : (double)i / distance;
    double q = start.coord.hex.q + (end.coord.hex.q - start.coord.hex.q) * t;
    double r = start.coord.hex.r + (end.coord.hex.r - start.coord.hex.r) * t;
    double s = start.coord.hex.s + (end.coord.hex.s - start.coord.hex.s) * t;

    (*out_cells)[i].type = GRID_TYPE_HEXAGON;
    (*out_cells)[i].coord.hex = hex_round(q, r, s);
  }

  *out_count = total;
}

static bool hex_cells_equal(grid_cell_t a, grid_cell_t b) {
  if (a.type != GRID_TYPE_HEXAGON || b.type != GRID_TYPE_HEXAGON) {
    return false;
  }

  return a.coord.hex.q == b.coord.hex.q && a.coord.hex.r == b.coord.hex.r &&
         a.coord.hex.s == b.coord.hex.s;
}

static bool hex_get_cell_mesh(const layout_t *layout, float **vertices,
                              unsigned int **indices, size_t *vertex_count,
                              size_t *index_count) {
  if (!vertices || !indices || !vertex_count || !index_count) {
    return false;
  }

  *vertex_count = 7; // Center + 6 corners
  *index_count = 18; // 6 triangles * 3 indices

  *vertices = malloc(*vertex_count * 2 * sizeof(float));
  *indices = malloc(*index_count * sizeof(unsigned int));

  if (!*vertices || !*indices) {
    free(*vertices);
    free(*indices);
    *vertices = NULL;
    *indices = NULL;
    return false;
  }

  // Center vertex
  (*vertices)[0] = 0.0f;
  (*vertices)[1] = 0.0f;

  // Corner vertices
  for (int i = 0; i < 6; i++) {
    double angle = 2.0 * M_PI * (layout->orientation.start_angle + i) / 6.0;
    (*vertices)[(i + 1) * 2] = layout->size.x * cos(angle);
    (*vertices)[(i + 1) * 2 + 1] = layout->size.y * sin(angle);
  }

  // Triangles (fan from center)
  for (int i = 0; i < 6; i++) {
    (*indices)[i * 3] = 0; // Center
    (*indices)[i * 3 + 1] = i + 1;
    (*indices)[i * 3 + 2] = ((i + 1) % 6) + 1;
  }

  return true;
}

// --- Additional hex-specific helper functions ---

static point_t hex_get_edge_start(const layout_t *layout, grid_cell_t cell,
                                  hex_edge_direction_t edge) {
  if (cell.type != GRID_TYPE_HEXAGON || edge < 0 || edge >= 6) {
    return (point_t){0, 0};
  }

  point_t corners[6];
  hex_get_corners(layout, cell, corners);
  return corners[edge];
}

static point_t hex_get_edge_end(const layout_t *layout, grid_cell_t cell,
                                hex_edge_direction_t edge) {
  if (cell.type != GRID_TYPE_HEXAGON || edge < 0 || edge >= 6) {
    return (point_t){0, 0};
  }

  point_t corners[6];
  hex_get_corners(layout, cell, corners);
  return corners[(edge + 1) % 6];
}

static point_t hex_get_vertex_position(const layout_t *layout, grid_cell_t cell,
                                       hex_vertex_direction_t vertex) {
  if (cell.type != GRID_TYPE_HEXAGON || vertex < 0 || vertex >= 6) {
    return (point_t){0, 0};
  }

  point_t corners[6];
  hex_get_corners(layout, cell, corners);
  return corners[vertex];
}

static grid_cell_t hex_get_edge_neighbor(grid_cell_t cell,
                                         hex_edge_direction_t edge) {
  grid_cell_t neighbor;
  hex_get_neighbor(cell, edge, &neighbor);
  return neighbor;
}

static void hex_get_spiral(grid_cell_t center, int max_radius,
                           grid_cell_t **out_cells, size_t *out_count) {
  hex_get_cells_in_range(center, max_radius, out_cells, out_count);
  // Could implement actual spiral ordering if needed
}

static grid_cell_t hex_rotate_around(grid_cell_t cell, grid_cell_t center,
                                     int rotations) {
  if (cell.type != GRID_TYPE_HEXAGON || center.type != GRID_TYPE_HEXAGON) {
    grid_cell_t invalid = {.type = GRID_TYPE_UNKNOWN};
    return invalid;
  }

  // Translate to origin
  grid_cell_t offset = hex_calculate_offset(center, cell);

  // Rotate around origin
  grid_cell_t rotated;
  hex_rotate_cell(offset, rotations, &rotated);

  // Translate back
  return hex_apply_offset(center, rotated);
}

static int hex_get_direction_to(grid_cell_t from, grid_cell_t to) {
  if (from.type != GRID_TYPE_HEXAGON || to.type != GRID_TYPE_HEXAGON) {
    return -1;
  }

  grid_cell_t offset = hex_calculate_offset(from, to);

  // Check if they're neighbors
  if (hex_distance(from, to) != 1) {
    return -1;
  }

  // Match offset to direction
  static const int dq[6] = {1, 1, 0, -1, -1, 0};
  static const int dr[6] = {0, -1, -1, 0, 1, 1};

  for (int dir = 0; dir < 6; dir++) {
    if (offset.coord.hex.q == dq[dir] && offset.coord.hex.r == dr[dir]) {
      return dir;
    }
  }

  return -1;
}

// --- vtable instance ---

const grid_vtable_t hex_grid_vtable = {
  .cell_to_pixel = hex_cell_to_pixel,
  .pixel_to_cell = hex_pixel_to_cell,
  .get_neighbor = hex_get_neighbor,
  .get_all_neighbors = hex_get_all_neighbors,
  .get_cells_in_range = hex_get_cells_in_range,
  .rotate_cell = hex_rotate_cell,
  .distance = hex_distance,
  .get_corners = hex_get_corners,
  .calculate_offset = hex_calculate_offset,
  .apply_offset = hex_apply_offset,
  .get_origin = hex_get_origin,
  .get_ring = hex_get_ring,
  .get_line = hex_get_line,
  .cells_equal = hex_cells_equal,
  .get_cell_mesh = hex_get_cell_mesh,
  .neighbor_count = 6,
  .corner_count = 6,
  .type = GRID_TYPE_HEXAGON};
