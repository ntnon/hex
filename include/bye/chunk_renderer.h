#ifndef CHUNK_RENDERER_H
#define CHUNK_RENDERER_H

#include "grid/grid_system.h"
#include "game/board.h"
#include "raylib.h"
#include "ui.h"

/**
 * @brief Frustum culling bounds for determining visible chunks
 */
typedef struct {
    float min_x, max_x;
    float min_y, max_y;
} frustum_bounds_t;

/**
 * @brief Cached vertex data for efficient hex rendering
 */
typedef struct {
    Vector2 center;
    Vector2 corners[6];
    Clay_Color fill_color;
    Clay_Color edge_color;
} hex_vertex_cache_t;

/**
 * @brief Batch renderer for multiple hexes
 */
typedef struct {
    hex_vertex_cache_t *hex_data;
    size_t hex_count;
    size_t hex_capacity;
    bool needs_flush;
} hex_batch_renderer_t;

// === Instanced Tile Rendering ===

/**
 * @brief Initializes the instanced hex mesh for tile rendering.
 * Creates the canonical hex geometry and OpenGL buffers.
 * @param grid The grid system instance.
 * @return True if initialization succeeded, false otherwise.
 */
bool chunk_renderer_init_hex_mesh(grid_t *grid);

/**
 * @brief Cleans up instanced rendering resources.
 * @param grid The grid system instance.
 */
void chunk_renderer_cleanup_grid_cache(grid_t *grid);

// === Chunk Management ===

/**
 * @brief Initializes render data for a specific chunk.
 * @param grid The grid system instance.
 * @param chunk The chunk to initialize rendering for.
 */
void chunk_renderer_init_chunk_cache(grid_t *grid, grid_chunk_t *chunk);

/**
 * @brief Rebuilds render cache for a dirty chunk.
 * @param grid The grid system instance.
 * @param chunk The chunk to rebuild.
 * @param board The board containing tile data.
 */
void chunk_renderer_rebuild_chunk(grid_t *grid, grid_chunk_t *chunk, const board_t *board);

/**
 * @brief Renders all visible chunks with tiles using instanced rendering.
 * Uses frustum culling and GPU instancing for optimal performance.
 * Only works after raylib initialization.
 * @param board The board to render.
 * @param camera The current camera for frustum culling.
 * @return True if rendered successfully, false if not ready.
 */
bool chunk_renderer_draw_tiles(const board_t *board, Camera2D camera);

/**
 * @brief Draws a single chunk using instanced rendering.
 * @param chunk The chunk to render.
 */
void render_chunk(const grid_chunk_t *chunk);

/**
 * @brief Cleans up render data for a chunk.
 * @param chunk The chunk to clean up.
 */
void chunk_renderer_cleanup_chunk_cache(grid_chunk_t *chunk);

// === Utility Functions ===

/**
 * @brief Calculates frustum bounds from camera for culling.
 * @param camera The camera to calculate bounds for.
 * @param screen_width Screen width in pixels.
 * @param screen_height Screen height in pixels.
 * @return Frustum bounds in world space.
 */
frustum_bounds_t chunk_renderer_get_frustum_bounds(Camera2D camera,
                                                   float screen_width,
                                                   float screen_height);

/**
 * @brief Checks if a chunk overlaps with frustum bounds.
 * @param grid The grid system instance.
 * @param chunk_id The chunk to test.
 * @param bounds The frustum bounds.
 * @return True if chunk is potentially visible.
 */
bool chunk_renderer_is_chunk_visible(const grid_t *grid,
                                     chunk_id_t chunk_id,
                                     frustum_bounds_t bounds);

/**
 * @brief Marks chunks dirty when tiles are added/removed.
 * Call this whenever tiles change in the board.
 * @param board The board that changed.
 * @param cell The cell that was modified.
 */
void chunk_renderer_mark_tile_dirty(board_t *board, grid_cell_t cell);

// === Batch Renderer ===

/**
 * @brief Initializes hex batch renderer.
 * @param renderer The renderer to initialize.
 * @param initial_capacity Initial number of hexes to allocate for.
 */
void hex_batch_init(hex_batch_renderer_t *renderer, size_t initial_capacity);

/**
 * @brief Adds a hex to the batch for rendering.
 * @param renderer The batch renderer.
 * @param center Center point of the hex.
 * @param corners Array of 6 corner points.
 * @param fill_color Fill color.
 * @param edge_color Edge color.
 */
void hex_batch_add(hex_batch_renderer_t *renderer,
                   Vector2 center,
                   Vector2 corners[6],
                   Clay_Color fill_color,
                   Clay_Color edge_color);

/**
 * @brief Flushes the batch renderer, drawing all hexes.
 * @param renderer The batch renderer to flush.
 */
void hex_batch_flush(hex_batch_renderer_t *renderer);

/**
 * @brief Cleans up batch renderer resources.
 * @param renderer The renderer to clean up.
 */
void hex_batch_cleanup(hex_batch_renderer_t *renderer);

// === Performance Testing ===

/**
 * @brief Tests chunk rendering performance.
 * @param board The board to test with.
 * @param camera Test camera setup.
 */
void chunk_renderer_performance_test(const board_t *board, Camera2D camera);

// === Optional Initialization ===

/**
 * @brief Enables instanced chunk rendering after raylib initialization.
 * Call this once after raylib is initialized to enable performance optimizations.
 * @param board The board to enable optimized rendering for.
 * @return True if optimization enabled successfully.
 */
bool chunk_renderer_enable_optimizations(board_t *board);

/**
 * @brief Checks if instanced chunk rendering is available.
 * @param board The board to check.
 * @return True if instanced rendering is available.
 */
bool chunk_renderer_is_optimized(const board_t *board);

#endif // CHUNK_RENDERER_H
