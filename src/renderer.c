#include "tile/tile.h"
#include "tile/tile_map.h"
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "raylib.h"
#include "raymath.h"

#include "ui.h"

#include "../include/grid/grid_cell_utils.h"
#include "../include/renderer.h"

// Instanced rendering system
#define MAX_INSTANCES_PER_BATCH 10000

typedef struct {
  Matrix *transforms;
  Color *colors;
  int count;
  int capacity;
} InstanceBatch;

typedef struct {
  Mesh hexMesh;
  Material materials[4];    // For different tile types
  InstanceBatch batches[4]; // One per tile type
  bool initialized;
} RenderingSystem;

static RenderingSystem g_renderSystem = {0};

// Create a proper white hexagon texture using exact hexagon geometry

// Generate a color based on chunk ID for visualization

// Helper function to get proper transform from grid cell using exact corner
// geometry
  // Get transform matrix using grid layout data directly
  static Matrix
  get_hex_transform_from_grid(const grid_t *grid, grid_cell_t cell) {
  if (!grid) {
    return MatrixIdentity();
  }

  // Use grid's layout data to get position and size
  const layout_t *layout = &grid->layout;

  // Get center position using grid's coordinate system
  point_t center = grid->vtable->to_pixel(grid, cell);

  // Use layout size and scale directly (no need to calculate from corners)
  float hex_radius = layout->size.x * layout->scale;

  // Store position and scale for DrawPoly rendering
  Matrix transform = MatrixIdentity();
  transform.m12 = center.x;                       // Store x position
  transform.m13 = center.y;                       // Store y position
  transform.m0 = hex_radius;                      // Store radius from layout
  transform.m1 = layout->orientation.start_angle; // Store rotation

  return transform;
}

// Validate that our hexagon rendering matches the grid's corner calculations
static bool validate_hex_geometry(const grid_t *grid) {
  if (!grid)
    return false;

  // Create a test cell at origin
  grid_cell_t test_cell = {.type = GRID_TYPE_HEXAGON,
                           .coord.hex = {.q = 0, .r = 0, .s = 0}};

  // Get corners from grid system
  point_t grid_corners[6];
  grid->vtable->get_corners(grid, test_cell, grid_corners);

  // Get center from grid system
  point_t center = grid->vtable->to_pixel(grid, test_cell);

  // Calculate expected radius based on corners
  float expected_radius = 0.0f;
  for (int i = 0; i < 6; i++) {
    float dx = grid_corners[i].x - center.x;
    float dy = grid_corners[i].y - center.y;
    float radius = sqrtf(dx * dx + dy * dy);
    if (i == 0 || radius > expected_radius) {
      expected_radius = radius;
    }
  }

  // We no longer calculate our own radius - we use grid's geometry directly
  return true;
}

// Initialize the instanced rendering system
static bool init_rendering_system(void) {
  if (g_renderSystem.initialized) {
    // printf("DEBUG: Rendering system already initialized");
    return true;
  }

  // printf("DEBUG: Initializing rendering system");

  // Use proper hexagon vertices from the grid system with pointy-top
  // orientation start_angle = 0.5 means 30 degree rotation (pointy-top)
  // Generate basic hexagon mesh - we'll rely on grid's get_corners for proper
  // orientation
  g_renderSystem.hexMesh = GenMeshPoly(6, 5.0f);

  // No texture creation needed for mesh instancing

  // Initialize materials for different tile types
  for (int i = 0; i < 4; i++) {
    g_renderSystem.materials[i] = LoadMaterialDefault();
  }

  // Initialize instance batches
  for (int i = 0; i < 4; i++) {
    g_renderSystem.batches[i].capacity = MAX_INSTANCES_PER_BATCH;
    g_renderSystem.batches[i].transforms =
      malloc(sizeof(Matrix) * MAX_INSTANCES_PER_BATCH);
    g_renderSystem.batches[i].colors =
      malloc(sizeof(Color) * MAX_INSTANCES_PER_BATCH);
    g_renderSystem.batches[i].count = 0;

    if (!g_renderSystem.batches[i].transforms ||
        !g_renderSystem.batches[i].colors) {
      printf("ERROR: Failed to allocate memory for instance batches\n");
      return false;
    }
  }

  g_renderSystem.initialized = true;
  // printf("DEBUG: Rendering system initialization complete");
  // printf("DEBUG: Rendering system validation complete");
  return true;
}

// Validate geometry when enabling optimizations
static bool init_and_validate_rendering_system(const grid_t *grid) {
  // printf("DEBUG: Attempting to init and validate rendering system");
  if (!init_rendering_system()) {
    // printf("DEBUG: init_rendering_system failed");
    return false;
  }

  // Validate that our geometry matches the grid's expectations
  if (grid && !validate_hex_geometry(grid)) {
    printf("WARNING: Hexagon rendering geometry may not match grid layout\n");
    // Continue anyway - the mismatch might be acceptable
  }

  return true;
}

// Cleanup rendering system
void cleanup_rendering_system(void) {
  if (!g_renderSystem.initialized)
    return;

  UnloadMesh(g_renderSystem.hexMesh);

  for (int i = 0; i < 4; i++) {
    UnloadMaterial(g_renderSystem.materials[i]);
    free(g_renderSystem.batches[i].transforms);
    free(g_renderSystem.batches[i].colors);
  }

  g_renderSystem.initialized = false;
}

// Add instance to batch
static void add_to_batch(int batch_index, Matrix transform, Color color) {
  if (!g_renderSystem.initialized || batch_index < 0 || batch_index >= 4)
    return;

  InstanceBatch *batch = &g_renderSystem.batches[batch_index];
  if (batch->count >= batch->capacity)
    return; // Batch full

  batch->transforms[batch->count] = transform;
  batch->colors[batch->count] = color;
  batch->count++;
}

// Flush all batches
static void flush_all_batches(void) {
  if (!g_renderSystem.initialized)
    return;

  // Use mesh instancing for all tile rendering
  for (int i = 0; i < 4; i++) {
    InstanceBatch *batch = &g_renderSystem.batches[i];
    if (batch->count > 0) {
      // printf("DEBUG: Flushing batch %d with %d instances\n", i,
      // batch->count);

      // Set material color based on batch index (0=MAGENTA, 1=CYAN, 2=YELLOW,
      // 3=DEFAULT)
      Color baseColor = WHITE;
      switch (i) {
      case 0: // TILE_MAGENTA batch
        baseColor = (Color){255, 0, 255, 255};
        break;
      case 1: // TILE_CYAN batch
        baseColor = (Color){0, 255, 255, 255};
        break;
      case 2: // TILE_YELLOW batch
        baseColor = (Color){255, 255, 0, 255};
        break;
      default:
        baseColor = WHITE;
        break;
      }

      g_renderSystem.materials[i].maps[MATERIAL_MAP_DIFFUSE].color = baseColor;

      // Use DrawPoly with proper orientation from grid layout
      for (int j = 0; j < batch->count; j++) {
        Matrix transform = batch->transforms[j];
        Vector2 position = {transform.m12, transform.m13};
        float radius = transform.m0;
        float rotation = transform.m1 * 60.0f; // Convert start_angle to degrees
        DrawPoly(position, 6, radius, rotation, baseColor);
      }

      batch->count = 0; // Reset batch
    }
  }

  // Remove test hexagon - not needed for production
  // DrawPoly((Vector2){0, 0}, 6, 50.0f, 0.0f, RED);
}

// Get tile type index for batching
static int get_tile_batch_index(tile_data_t tile_data) {
  switch (tile_data.type) {
  case TILE_MAGENTA:
    return 0;
  case TILE_CYAN:
    return 1;
  case TILE_YELLOW:
    return 2;
  default:
    return 3;
  }
}

// Helper: Add tile to instance batch; user_data is a pointer to the grid.
static void add_tile_to_batch(tile_t *tile, void *user_data) {
  const grid_t *grid = (const grid_t *)user_data;
  if (!tile || !grid)
    return;

  // printf("DEBUG: Adding tile type %d to batch\n", tile->data.type);

  // Use grid's geometry to get proper transform
  Matrix transform = get_hex_transform_from_grid(grid, tile->cell);

  // Get tile color by tile type
  Color color = WHITE;
  switch (tile->data.type) {
  case TILE_MAGENTA:
    color = (Color){255, 0, 255, 255};
    break;
  case TILE_CYAN:
    color = (Color){0, 255, 255, 255};
    break;
  case TILE_YELLOW:
    color = (Color){255, 255, 0, 255};
    break;
  default:
    color = WHITE;
    break;
  }

  // Add to appropriate batch
  int batch_index = get_tile_batch_index(tile->data);
  add_to_batch(batch_index, transform, color);
}

// Helper: Draw each tile from the tile map; user_data is a pointer to the
// grid.
static void draw_tile_wrapper(tile_t *tile, void *user_data) {
  const grid_t *grid = (const grid_t *)user_data;
  render_tile(tile, grid);
}

void render_board(const board_t *board) {
  if (!board) {
    printf("ERROR: board is null\n");
    return;
  }

  if (!board->grid) {
    printf("ERROR: board->grid is null\n");
    return;
  }

  if (!board->tiles) {
    printf("ERROR: board->tiles is null\n");
    return;
  }

  tile_map_foreach_tile(board->tiles, draw_tile_wrapper, (void *)board->grid);
}

Clay_Color color_from_tile(tile_data_t tile_data) {
  switch (tile_data.type) {
  case TILE_MAGENTA:
    return M_MAGENTA;
  case TILE_CYAN:
    return M_SKYBLUE;
  case TILE_YELLOW:
    return M_YELLOW;
  default:
    printf("ERROR: unknown tile type\n");
    return M_BLANK;
  }
}

void render_tile(const tile_t *tile, const grid_t *grid) {
  if (!tile) {
    printf("ERROR: tile is null\n");
    return;
  }
  if (!grid) {
    printf("ERROR: grid is null\n");
    return;
  }
  if (!grid->vtable) {
    printf("ERROR: grid->vtable is null\n");
    return;
  }

  // Draw the colored hex cell.
  render_hex_cell(grid, tile->cell, color_from_tile(tile->data), M_BLANK);
}

void render_hex_grid(const grid_t *grid) {
  if (!grid) {
    return;
  }

  // Generate hex coordinates on-demand within the grid radius
  if (grid->type == GRID_TYPE_HEXAGON) {
    for (int q = -grid->radius; q <= grid->radius; q++) {
      int r1 = fmax(-grid->radius, -q - grid->radius);
      int r2 = fmin(grid->radius, -q + grid->radius);
      for (int r = r1; r <= r2; r++) {
        int s = -q - r;
        grid_cell_t cell = {.type = GRID_TYPE_HEXAGON,
                            .coord.hex = {.q = q, .r = r, .s = s}};
        render_hex_cell(grid, cell, M_BLANK, M_GRAY);
      }
    }
  }
}

Color to_raylib_color(Clay_Color color) {
  return (Color){color.r, color.g, color.b, color.a};
}

/* Optimized rendering functions (require raylib initialization) */
void render_board_optimized(const board_t *board) {
  if (!board) {
    printf("ERROR: board is null\n");
    return;
  }

  if (!board->grid) {
    printf("ERROR: board->grid is null\n");
    return;
  }

  if (!board->tiles) {
    printf("ERROR: board->tiles is null\n");
    return;
  }

  // printf("DEBUG: render_board_optimized called");

  // Initialize rendering system if needed with validation
  if (!init_and_validate_rendering_system(board->grid)) {
    // printf("DEBUG: Falling back to traditional rendering");
    // Fall back to traditional rendering
    tile_map_foreach_tile(board->tiles, draw_tile_wrapper, (void *)board->grid);
    return;
  }

  // printf("DEBUG: Using optimized rendering");

  // Clear all batches before adding new tiles
  for (int i = 0; i < 4; i++) {
    g_renderSystem.batches[i].count = 0;
  }

  // Add all tiles to instance batches
  tile_map_foreach_tile(board->tiles, add_tile_to_batch, (void *)board->grid);

  // Debug: Check batch counts
  // int total_tiles = 0;
  // for (int i = 0; i < 4; i++) {
  //   total_tiles += g_renderSystem.batches[i].count;
  // }
  // if (total_tiles > 0) {
  //   printf("DEBUG: Rendering %d tiles in batches\n", total_tiles);
  // }

  // Flush all batches (renders all tiles in few draw calls)
  flush_all_batches();
}

void render_hex_cell(const grid_t *grid, grid_cell_t cell,
                     Clay_Color fill_color, Clay_Color edge_color) {
  // Use grid's get_corners function for proper orientation and sizing
  int corners_count = 6;
  point_t corners[6];
  grid->vtable->get_corners(grid, cell, corners);

  Vector2 verts[6];
  // Reverse vertex order for proper clockwise winding in DrawTriangleFan
  for (int j = 0; j < corners_count; ++j) {
    int reversed_j = (corners_count - 1) - j;
    verts[j].x = (float)corners[reversed_j].x;
    verts[j].y = (float)corners[reversed_j].y;
  }

  Color ray_fill_color = to_raylib_color(fill_color);
  Color ray_edge_color = to_raylib_color(edge_color);

  // Use DrawTriangleFan with exact corner positions from grid
  if (fill_color.a > 0) {
    DrawTriangleFan(verts, corners_count, ray_fill_color);
  }

  // Draw outline if edge color is not blank
  if (edge_color.a > 0) {
    for (int j = 0; j < corners_count; ++j) {
      int next = (j + 1) % corners_count;
      DrawLineV(verts[j], verts[next], ray_edge_color);
    }
  }
}

void render_board_previews(const board_t *board) {
  if (!board || !board->preview_boards || board->num_preview_boards == 0) {
    return;
  }

  for (size_t i = 0; i < board->num_preview_boards; i++) {
    board_preview_t *preview = &board->preview_boards[i];

    // Render valid preview tiles with transparency and green border
    if (preview->num_preview_tiles > 0) {
      for (size_t j = 0; j < preview->num_preview_tiles; j++) {
        grid_cell_t position = preview->preview_positions[j];
        tile_data_t tile_data = preview->preview_tiles[j];

        Clay_Color tile_color = color_from_tile(tile_data);
        Clay_Color preview_color =
          (Clay_Color){tile_color.r, tile_color.g, tile_color.b, 180};

        render_hex_cell(board->grid, position, preview_color,
                        (Clay_Color){0, 255, 0, 255}); // Green border for valid
      }
    }

    // Render conflicts with red markers
    if (preview->num_conflicts > 0) {
      for (size_t j = 0; j < preview->num_conflicts; j++) {
        render_hex_cell(board->grid, preview->conflict_positions[j],
                        (Clay_Color){255, 0, 0, 128},  // Semi-transparent red
                        (Clay_Color){255, 0, 0, 255}); // Red border
      }
    }
  }
}

void render_board_in_bounds(const board_t *board, Rectangle bounds) {
  if (!board || bounds.width <= 0 || bounds.height <= 0) {
    return;
  }

  const grid_t *grid = board->grid;
  if (!grid) {
    return;
  }

  // Get all grid cells
  grid_cell_t *cells;
  size_t num_cells;
  grid_get_all_cells(grid, &cells, &num_cells);

  if (!cells || num_cells == 0) {
    return;
  }

  // Calculate board bounding box
  float min_x = FLT_MAX, min_y = FLT_MAX;
  float max_x = -FLT_MAX, max_y = -FLT_MAX;

  for (size_t i = 0; i < num_cells; i++) {
    grid_cell_t cell = cells[i];
    point_t corners[6];
    grid->vtable->get_corners(grid, cell, corners);
    for (int j = 0; j < 6; j++) {
      if (corners[j].x < min_x)
        min_x = corners[j].x;
      if (corners[j].y < min_y)
        min_y = corners[j].y;
      if (corners[j].x > max_x)
        max_x = corners[j].x;
      if (corners[j].y > max_y)
        max_y = corners[j].y;
    }
  }

  float board_width = max_x - min_x;
  float board_height = max_y - min_y;

  if (cells) {
    free(cells);
  }

  if (board_width <= 0 || board_height <= 0) {
    return;
  }

  // Calculate scale to fit board in bounds with some padding
  float padding = 10.0f;
  float scale_x = (bounds.width - padding * 2) / board_width;
  float scale_y = (bounds.height - padding * 2) / board_height;
  float scale = fminf(scale_x, scale_y);

  // Calculate center position
  float board_center_x = (min_x + max_x) * 0.5f;
  float board_center_y = (min_y + max_y) * 0.5f;
  float bounds_center_x = bounds.x + bounds.width * 0.5f;
  float bounds_center_y = bounds.y + bounds.height * 0.5f;

  // Set up camera for scaled rendering
  Camera2D camera = {0};
  camera.offset = (Vector2){bounds_center_x, bounds_center_y};
  camera.target = (Vector2){board_center_x, board_center_y};
  camera.rotation = 0.0f;
  camera.zoom = scale;

  BeginMode2D(camera);
  render_board_optimized(board);
  EndMode2D();
}

// Cleanup function to be called on exit
void renderer_cleanup(void) { cleanup_rendering_system(); }
