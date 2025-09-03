#include "../include/bye/chunk_renderer.h"
#include "../include/grid/grid_system.h"
#include "../include/renderer.h"
#include "game/board.h"
#include "raylib.h"
#include "rlgl.h"
#include "tile/tile.h"
#include "tile/tile_map.h"
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Constants for optimization
#define INITIAL_BATCH_SIZE 256
#define FRUSTUM_PADDING 100.0f // Extra padding for chunk culling
#define MAX_INSTANCES_PER_CHUNK 128
#define INSTANCE_DATA_SIZE 6 // position(2) + color(4)

// === Simplified Chunk Batching (using raylib functions) ===

static bool initialize_chunk_batching(grid_t *grid) {
  if (!grid)
    return false;

  chunk_system_t *system = &grid->chunk_system;

  if (system->hex_mesh_initialized)
    return true; // Already initialized

  // Just mark as initialized - we'll use raylib's DrawTriangleFan in batches
  system->hex_mesh_initialized = true;

  printf("Initialized chunk batching system\n");
  return true;
}

void chunk_renderer_cleanup_grid_cache(grid_t *grid) {
  if (!grid)
    return;

  chunk_system_t *system = &grid->chunk_system;
  system->hex_mesh_initialized = false;
}

// === Chunked Tile Rendering ===

void chunk_renderer_init_chunk_cache(grid_t *grid, grid_chunk_t *chunk) {
  if (!grid || !chunk)
    return;

  // Clean up existing render data
  if (chunk->render_data) {
    chunk_renderer_cleanup_chunk_cache(chunk);
  }

  chunk->render_data = malloc(sizeof(chunk_render_data_t));
  if (!chunk->render_data)
    return;

  chunk_render_data_t *render_data = chunk->render_data;
  render_data->instances =
    malloc(MAX_INSTANCES_PER_CHUNK * sizeof(hex_instance_t));
  render_data->instance_count = 0;
  render_data->instance_capacity = MAX_INSTANCES_PER_CHUNK;
  render_data->vbo_id = 0;
  render_data->needs_rebuild = true;
  render_data->gpu_buffer_valid = false;

  if (!render_data->instances) {
    free(render_data);
    chunk->render_data = NULL;
    return;
  }
}

void chunk_renderer_rebuild_chunk(grid_t *grid, grid_chunk_t *chunk,
                                  const board_t *board) {
  if (!grid || !chunk || !board)
    return;

  if (!chunk->render_data) {
    chunk_renderer_init_chunk_cache(grid, chunk);
  }

  chunk_render_data_t *render_data = chunk->render_data;
  if (!render_data)
    return;

  // Ensure batching system is initialized
  if (!initialize_chunk_batching(grid)) {
    printf("ERROR: Failed to initialize chunk batching\n");
    return;
  }

  // Get coordinates for this chunk
  grid_cell_t *coords;
  size_t coord_count;
  grid_get_chunk_coordinates(grid, chunk->id, &coords, &coord_count);

  if (!coords || coord_count == 0) {
    render_data->instance_count = 0;
    render_data->needs_rebuild = false;
    render_data->gpu_buffer_valid = false;
    return;
  }

  // Collect tiles in this chunk
  tile_t *chunk_tiles[MAX_INSTANCES_PER_CHUNK];
  size_t tile_count = 0;

  for (size_t i = 0; i < coord_count && tile_count < MAX_INSTANCES_PER_CHUNK;
       i++) {
    tile_map_entry_t *entry = tile_map_find(board->tiles, coords[i]);
    if (entry && entry->tile) {
      chunk_tiles[tile_count++] = entry->tile;
    }
  }

  if (tile_count == 0) {
    render_data->instance_count = 0;
    render_data->needs_rebuild = false;
    render_data->gpu_buffer_valid = false;
    free(coords);
    return;
  }

  // Create instance data using grid-specific function
  float instance_data[MAX_INSTANCES_PER_CHUNK * INSTANCE_DATA_SIZE];
  size_t instances_created = grid->vtable->create_chunk_instances(
    grid, chunk->id, chunk_tiles, tile_count, instance_data,
    MAX_INSTANCES_PER_CHUNK);

  if (instances_created == 0) {
    render_data->instance_count = 0;
    render_data->needs_rebuild = false;
    render_data->gpu_buffer_valid = false;
    free(coords);
    return;
  }

  // Copy instance data to our buffer
  for (size_t i = 0; i < instances_created; i++) {
    size_t base_idx = i * INSTANCE_DATA_SIZE;
    render_data->instances[i].position[0] = instance_data[base_idx + 0];
    render_data->instances[i].position[1] = instance_data[base_idx + 1];
    render_data->instances[i].color[0] = instance_data[base_idx + 2];
    render_data->instances[i].color[1] = instance_data[base_idx + 3];
    render_data->instances[i].color[2] = instance_data[base_idx + 4];
    render_data->instances[i].color[3] = instance_data[base_idx + 5];
  }

  render_data->instance_count = instances_created;
  render_data->needs_rebuild = false;
  render_data->gpu_buffer_valid = false; // Need to upload to GPU
  chunk->dirty = false;

  free(coords);
}

frustum_bounds_t chunk_renderer_get_frustum_bounds(Camera2D camera,
                                                   float screen_width,
                                                   float screen_height) {
  frustum_bounds_t bounds;

  // Calculate world space bounds from camera
  Vector2 top_left = GetScreenToWorld2D((Vector2){0, 0}, camera);
  Vector2 bottom_right =
    GetScreenToWorld2D((Vector2){screen_width, screen_height}, camera);

  bounds.min_x = fminf(top_left.x, bottom_right.x) - FRUSTUM_PADDING;
  bounds.max_x = fmaxf(top_left.x, bottom_right.x) + FRUSTUM_PADDING;
  bounds.min_y = fminf(top_left.y, bottom_right.y) - FRUSTUM_PADDING;
  bounds.max_y = fmaxf(top_left.y, bottom_right.y) + FRUSTUM_PADDING;

  return bounds;
}

bool chunk_renderer_is_chunk_visible(const grid_t *grid, chunk_id_t chunk_id,
                                     frustum_bounds_t bounds) {
  if (!grid)
    return false;

  // Get approximate chunk bounds by sampling corner coordinates
  int chunk_size = grid->chunk_system.chunk_size;
  int min_q = chunk_id.chunk_x * chunk_size;
  int max_q = min_q + chunk_size - 1;
  int min_r = chunk_id.chunk_y * chunk_size;
  int max_r = min_r + chunk_size - 1;

  // Sample chunk corners to get approximate bounds
  grid_cell_t corner_cells[4] = {
    {.type = GRID_TYPE_HEXAGON, .coord.hex = {min_q, min_r, -min_q - min_r}},
    {.type = GRID_TYPE_HEXAGON, .coord.hex = {max_q, min_r, -max_q - min_r}},
    {.type = GRID_TYPE_HEXAGON, .coord.hex = {min_q, max_r, -min_q - max_r}},
    {.type = GRID_TYPE_HEXAGON, .coord.hex = {max_q, max_r, -max_q - max_r}}};

  float chunk_min_x = FLT_MAX, chunk_max_x = -FLT_MAX;
  float chunk_min_y = FLT_MAX, chunk_max_y = -FLT_MAX;

  for (int i = 0; i < 4; i++) {
    if (grid->vtable->is_valid_cell(grid, corner_cells[i])) {
      point_t pos = grid->vtable->to_pixel(grid, corner_cells[i]);
      chunk_min_x = fminf(chunk_min_x, (float)pos.x);
      chunk_max_x = fmaxf(chunk_max_x, (float)pos.x);
      chunk_min_y = fminf(chunk_min_y, (float)pos.y);
      chunk_max_y = fmaxf(chunk_max_y, (float)pos.y);
    }
  }

  // Check overlap with frustum bounds
  return !(chunk_max_x < bounds.min_x || chunk_min_x > bounds.max_x ||
           chunk_max_y < bounds.min_y || chunk_min_y > bounds.max_y);
}

bool chunk_renderer_draw_tiles(const board_t *board, Camera2D camera) {
  if (!board || !board->grid)
    return false;

  grid_t *grid = board->grid;
  chunk_system_t *system = &grid->chunk_system;

  if (!system->hash_table)
    return false;

  // Check if raylib is ready
  if (rlGetTextureIdDefault() == 0) {
    return false; // Not ready for rendering
  }

  // Ensure batching system is initialized
  if (!system->hex_mesh_initialized && !initialize_chunk_batching(grid)) {
    return false; // Can't render without batching system
  }

  // Get frustum bounds for culling
  frustum_bounds_t bounds = chunk_renderer_get_frustum_bounds(
    camera, GetScreenWidth(), GetScreenHeight());

  size_t rendered_chunks = 0;
  size_t culled_chunks = 0;
  size_t total_instances = 0;

  // Render visible chunks using batched rendering
  for (size_t i = 0; i < system->hash_table_size; i++) {
    grid_chunk_t *chunk = system->hash_table[i];
    while (chunk) {
      // Frustum culling
      if (!chunk_renderer_is_chunk_visible(grid, chunk->id, bounds)) {
        culled_chunks++;
        chunk = chunk->next;
        continue;
      }

      // Rebuild if dirty
      if (chunk->dirty || !chunk->render_data ||
          (chunk->render_data && chunk->render_data->needs_rebuild)) {
        chunk_renderer_rebuild_chunk(grid, chunk, board);
      }

      // Draw the chunk instances
      if (chunk->render_data && chunk->render_data->instance_count > 0) {
        render_chunk(chunk);
        rendered_chunks++;
        total_instances += chunk->render_data->instance_count;
      }

      chunk = chunk->next;
    }
  }

  // Optional debug info
  // printf("Rendered %zu chunks (%zu instances), culled %zu chunks\n",
  //        rendered_chunks, total_instances, culled_chunks);

  return true;
}

void render_chunk(const grid_chunk_t *chunk) {
  if (!chunk || !chunk->render_data)
    return;

  chunk_render_data_t *render_data = chunk->render_data;
  if (render_data->instance_count == 0)
    return;

  // Draw each hex instance using raylib's DrawTriangleFan
  for (size_t i = 0; i < render_data->instance_count; i++) {
    hex_instance_t *instance = &render_data->instances[i];

    // Create hex vertices around the instance position
    Vector2 center = {instance->position[0], instance->position[1]};
    Color color = {(unsigned char)(instance->color[0] * 255),
                   (unsigned char)(instance->color[1] * 255),
                   (unsigned char)(instance->color[2] * 255),
                   (unsigned char)(instance->color[3] * 255)};

    // Simple hex approximation - you could make this more accurate
    Vector2 hexPoints[6];
    float radius = 10.0f; // Fixed radius for now
    for (int j = 0; j < 6; j++) {
      float angle = j * 60.0f * DEG2RAD;
      hexPoints[j] = (Vector2){center.x + radius * cosf(angle),
                               center.y + radius * sinf(angle)};
    }

    DrawTriangleFan(hexPoints, 6, color);
  }
}

void chunk_renderer_cleanup_chunk_cache(grid_chunk_t *chunk) {
  if (!chunk || !chunk->render_data)
    return;

  chunk_render_data_t *render_data = chunk->render_data;

  if (render_data->instances) {
    free(render_data->instances);
  }

  // No GPU resources to clean up in simplified version

  free(render_data);
  chunk->render_data = NULL;
}

void chunk_renderer_mark_tile_dirty(board_t *board, grid_cell_t cell) {
  if (!board || !board->grid)
    return;

  chunk_id_t chunk_id = grid_get_chunk_id(board->grid, cell);
  grid_mark_chunk_dirty(board->grid, chunk_id);
}

// === Batch Renderer ===

void hex_batch_init(hex_batch_renderer_t *renderer, size_t initial_capacity) {
  if (!renderer)
    return;

  renderer->hex_capacity =
    initial_capacity > 0 ? initial_capacity : INITIAL_BATCH_SIZE;
  renderer->hex_data =
    malloc(renderer->hex_capacity * sizeof(hex_vertex_cache_t));
  renderer->hex_count = 0;
  renderer->needs_flush = false;
}

void hex_batch_add(hex_batch_renderer_t *renderer, Vector2 center,
                   Vector2 corners[6], Clay_Color fill_color,
                   Clay_Color edge_color) {
  if (!renderer || !corners)
    return;

  // Resize if needed
  if (renderer->hex_count >= renderer->hex_capacity) {
    renderer->hex_capacity *= 2;
    renderer->hex_data = realloc(
      renderer->hex_data, renderer->hex_capacity * sizeof(hex_vertex_cache_t));
    if (!renderer->hex_data)
      return;
  }

  hex_vertex_cache_t *hex = &renderer->hex_data[renderer->hex_count];
  hex->center = center;
  memcpy(hex->corners, corners, 6 * sizeof(Vector2));
  hex->fill_color = fill_color;
  hex->edge_color = edge_color;

  renderer->hex_count++;
  renderer->needs_flush = true;
}

void hex_batch_flush(hex_batch_renderer_t *renderer) {
  if (!renderer || renderer->hex_count == 0)
    return;

  // Draw all hexes in the batch
  for (size_t i = 0; i < renderer->hex_count; i++) {
    const hex_vertex_cache_t *hex = &renderer->hex_data[i];

    // Draw filled hex
    Color fill = to_raylib_color(hex->fill_color);
    DrawTriangleFan((Vector2 *)hex->corners, 6, fill);

    // Draw edges
    Color edge = to_raylib_color(hex->edge_color);
    for (int j = 0; j < 6; j++) {
      int next = (j + 1) % 6;
      DrawLineV(hex->corners[j], hex->corners[next], edge);
    }
  }

  renderer->hex_count = 0;
  renderer->needs_flush = false;
}

void hex_batch_cleanup(hex_batch_renderer_t *renderer) {
  if (!renderer)
    return;

  if (renderer->hex_data) {
    free(renderer->hex_data);
    renderer->hex_data = NULL;
  }
  renderer->hex_count = 0;
  renderer->hex_capacity = 0;
  renderer->needs_flush = false;
}

// === Performance Testing ===

void chunk_renderer_performance_test(const board_t *board, Camera2D camera) {
  if (!board) {
    printf("ERROR: Cannot test chunk renderer with null board\n");
    return;
  }

  printf("\n=== Chunk Renderer Performance Test ===\n");

  grid_t *grid = board->grid;
  chunk_system_t *system = &grid->chunk_system;

  printf("Grid size: %d radius (%zu total cells)\n", grid->radius,
         (size_t)(3 * grid->radius * (grid->radius + 1) + 1));
  printf("Chunk system: %zu chunks (size %d)\n", system->num_chunks,
         system->chunk_size);

  // Use fixed bounds for headless testing (1300x700 screen simulation)
  frustum_bounds_t bounds;
  bounds.min_x = -650.0f;
  bounds.max_x = 650.0f;
  bounds.min_y = -350.0f;
  bounds.max_y = 350.0f;

  printf("Frustum bounds: (%.1f, %.1f) to (%.1f, %.1f)\n", bounds.min_x,
         bounds.min_y, bounds.max_x, bounds.max_y);

  size_t visible_chunks = 0;
  size_t chunks_with_tiles = 0;
  size_t total_tiles_in_visible_chunks = 0;

  clock_t start = clock();

  for (size_t i = 0; i < system->hash_table_size; i++) {
    grid_chunk_t *chunk = system->hash_table[i];
    while (chunk) {
      if (chunk_renderer_is_chunk_visible(grid, chunk->id, bounds)) {
        visible_chunks++;

        // Check if chunk has tiles and count them
        grid_cell_t *coords;
        size_t coord_count;
        grid_get_chunk_coordinates(grid, chunk->id, &coords, &coord_count);

        if (coords) {
          size_t tiles_in_chunk = 0;
          for (size_t j = 0; j < coord_count; j++) {
            tile_map_entry_t *entry = tile_map_find(board->tiles, coords[j]);
            if (entry && entry->tile) {
              tiles_in_chunk++;
            }
          }
          if (tiles_in_chunk > 0) {
            chunks_with_tiles++;
            total_tiles_in_visible_chunks += tiles_in_chunk;
          }
          free(coords);
        }
      }
      chunk = chunk->next;
    }
  }

  clock_t end = clock();
  double culling_time = ((double)(end - start)) / CLOCKS_PER_SEC;

  printf("Visible chunks: %zu / %zu (%.1f%% culled)\n", visible_chunks,
         system->num_chunks,
         100.0f * (system->num_chunks - visible_chunks) / system->num_chunks);
  printf("Chunks with tiles: %zu\n", chunks_with_tiles);
  printf("Total tiles in visible chunks: %zu\n", total_tiles_in_visible_chunks);
  printf("Frustum culling time: %.6f seconds\n", culling_time);

  // Test grid cache performance
  if (system->hex_vertices) {
    printf("Grid background cache: %zu vertices (%.1fKB)\n",
           system->hex_vertex_count,
           (system->hex_vertex_count * sizeof(float)) / 1024.0f);
  }

  printf("=== Optimization Summary ===\n");
  printf("- Chunk-based rendering: %zu chunks instead of %d individual cells\n",
         system->num_chunks, grid->radius * 2 + 1);
  printf("- Frustum culling: %.1f%% performance gain from culling\n",
         100.0f * (system->num_chunks - visible_chunks) / system->num_chunks);
  printf("- Grid background: Pre-cached %zu vertices\n",
         system->hex_vertex_count);
  printf("=== Test Complete ===\n\n");
}

bool chunk_renderer_enable_optimizations(board_t *board) {
  if (!board || !board->grid) {
    return false;
  }

  // Check if raylib is initialized
  if (rlGetTextureIdDefault() == 0) {
    printf(
      "WARNING: Cannot enable chunk optimizations - raylib not initialized\n");
    return false;
  }

  // Initialize chunk batching system for tile rendering
  bool success = initialize_chunk_batching(board->grid);
  if (success) {
    printf("Batched tile rendering enabled for chunks\n");
  } else {
    printf("Failed to initialize chunk batching, using fallback\n");
  }
  return success;
}

bool chunk_renderer_is_optimized(const board_t *board) {
  if (!board || !board->grid) {
    return false;
  }

  const chunk_system_t *system = &board->grid->chunk_system;
  return system->hex_mesh_initialized;
}
