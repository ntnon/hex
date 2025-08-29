#include "../../include/grid/grid_system.h"
#include "ui.h"
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

static chunk_id_t get_chunk_id(const grid_t *grid, grid_cell_t cell,
                               int chunk_size) {
  if (cell.type != GRID_TYPE_HEXAGON || chunk_size <= 0) {
    return INVALID_CHUNK_ID;
  }

  // Optimized chunking for hex grids using floor division
  // This ensures consistent chunk boundaries for negative coordinates
  int chunk_x, chunk_y;

  if (cell.coord.hex.q >= 0) {
    chunk_x = cell.coord.hex.q / chunk_size;
  } else {
    chunk_x = (cell.coord.hex.q - chunk_size + 1) / chunk_size;
  }

  if (cell.coord.hex.r >= 0) {
    chunk_y = cell.coord.hex.r / chunk_size;
  } else {
    chunk_y = (cell.coord.hex.r - chunk_size + 1) / chunk_size;
  }

  return (chunk_id_t){chunk_x, chunk_y};
}

static size_t create_chunk_instances(const grid_t *grid, chunk_id_t chunk_id,
                                     tile_t **tiles, size_t tile_count,
                                     float *instance_data,
                                     size_t max_instances) {
  if (!grid || !tiles || !instance_data || tile_count == 0) {
    return 0;
  }

  size_t instances_created = 0;

  for (size_t i = 0; i < tile_count && instances_created < max_instances; i++) {
    tile_t *tile = tiles[i];
    if (!tile)
      continue;

    // Get world position for this hex
    point_t center = grid->vtable->to_pixel(grid, tile->cell);

    // Get color for this tile
    Clay_Color tile_color;
    switch (tile->data.type) {
    case TILE_MAGENTA:
      tile_color = M_MAGENTA;
      break;
    case TILE_CYAN:
      tile_color = M_SKYBLUE;
      break;
    case TILE_YELLOW:
      tile_color = M_YELLOW;
      break;
    default:
      tile_color = (Clay_Color){128, 128, 128, 255}; // Gray fallback
      break;
    }

    // Pack instance data: position (2 floats) + color (4 floats) = 6 floats per
    // instance
    size_t base_idx = instances_created * 6;
    instance_data[base_idx + 0] = (float)center.x;       // position.x
    instance_data[base_idx + 1] = (float)center.y;       // position.y
    instance_data[base_idx + 2] = tile_color.r / 255.0f; // color.r
    instance_data[base_idx + 3] = tile_color.g / 255.0f; // color.g
    instance_data[base_idx + 4] = tile_color.b / 255.0f; // color.b
    instance_data[base_idx + 5] = tile_color.a / 255.0f; // color.a

    instances_created++;
  }

  return instances_created;
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

static void get_chunk_coordinates(const grid_t *grid, chunk_id_t chunk_id,
                                  int chunk_size, grid_cell_t **out_coords,
                                  size_t *out_count) {
  if (!grid || !out_coords || !out_count) {
    if (out_coords)
      *out_coords = NULL;
    if (out_count)
      *out_count = 0;
    return;
  }

  // Calculate coordinate bounds for this chunk
  int min_q = chunk_id.chunk_x * chunk_size;
  int max_q = min_q + chunk_size - 1;
  int min_r = chunk_id.chunk_y * chunk_size;
  int max_r = min_r + chunk_size - 1;

  // Pre-calculate grid radius for faster bounds checking
  int radius = grid->radius;

  // Estimate capacity to reduce allocations (hex chunks are roughly circular)
  size_t estimated_capacity = chunk_size * chunk_size;
  if (estimated_capacity > 256)
    estimated_capacity = 256; // Cap for memory efficiency

  // Use temporary array to build coordinates
  grid_cell_t *temp_coords = malloc(estimated_capacity * sizeof(grid_cell_t));
  if (!temp_coords) {
    *out_coords = NULL;
    *out_count = 0;
    return;
  }

  size_t count = 0;
  size_t capacity = estimated_capacity;

  // Generate coordinates with early termination for performance
  for (int q = min_q; q <= max_q; q++) {
    // Skip entire rows that are clearly out of bounds
    if (abs(q) > radius)
      continue;

    for (int r = min_r; r <= max_r; r++) {
      int s = -q - r;

      // Fast bounds check using hex distance formula
      int hex_distance = (abs(q) + abs(r) + abs(s)) / 2;
      if (hex_distance <= radius) {
        // Resize if needed
        if (count >= capacity) {
          capacity *= 2;
          grid_cell_t *new_coords =
            realloc(temp_coords, capacity * sizeof(grid_cell_t));
          if (!new_coords) {
            free(temp_coords);
            *out_coords = NULL;
            *out_count = 0;
            return;
          }
          temp_coords = new_coords;
        }

        temp_coords[count].type = GRID_TYPE_HEXAGON;
        temp_coords[count].coord.hex.q = q;
        temp_coords[count].coord.hex.r = r;
        temp_coords[count].coord.hex.s = s;
        count++;
      }
    }
  }

  if (count == 0) {
    free(temp_coords);
    *out_coords = NULL;
    *out_count = 0;
    return;
  }

  // Resize to exact size needed
  *out_coords = realloc(temp_coords, count * sizeof(grid_cell_t));
  if (!*out_coords) {
    // Fallback: use the temp array as-is
    *out_coords = temp_coords;
  }

  *out_count = count;
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

  // Initialize chunk system with optimized settings
  grid->chunk_system.chunk_size = 0; // Will be set when chunks are initialized
  grid->chunk_system.hash_table = NULL;
  grid->chunk_system.hash_table_size = 0;
  grid->chunk_system.num_chunks = 0;
  grid->chunk_system.system_dirty = false;
  grid->chunk_system.coord_pool = NULL;

  // Initialize hex mesh data
  grid->chunk_system.hex_vertices = NULL;
  grid->chunk_system.hex_indices = NULL;
  grid->chunk_system.hex_vertex_count = 0;
  grid->chunk_system.hex_index_count = 0;
  grid->chunk_system.hex_vao_id = 0;
  grid->chunk_system.hex_vbo_id = 0;
  grid->chunk_system.hex_ebo_id = 0;
  grid->chunk_system.hex_mesh_initialized = false;

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

  // Clean up chunk system
  grid_cleanup_chunks(grid);

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

  // Mark chunk system for update when chunks are being used
  if (grid->chunk_system.hash_table) {
    grid->chunk_system.system_dirty = true;
  }

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

const grid_vtable_t hex_grid_vtable = {
  .to_pixel = to_pixel,
  .from_pixel = from_pixel,
  .get_neighbor_cell = get_neighbor_cell,
  .get_neighbor_cells = get_neighbor_cells,
  .distance = distance,
  .get_corners = get_corners,
  .get_all_cells = get_all_cells,
  .get_chunk_id = get_chunk_id,
  .get_chunk_coordinates = get_chunk_coordinates,
  .create_chunk_instances = create_chunk_instances,
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
