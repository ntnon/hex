#include "tile/tile.h"
#include "tile/tile_map.h"
#include <math.h>
#include <stdio.h>

#include "../../include/grid/grid_cell_utils.h"
#include "../../include/grid/grid_geometry.h"
#include "../../include/renderer/edge_render_list.h"
#include "../../include/renderer/renderer.h"
#include "game/game.h"
#include "raylib.h"
#include "ui.h"

// Color utilities
Clay_Color color_from_tile(tile_data_t tile_data) {
  switch (tile_data.type) {
  case TILE_MAGENTA:
    return M_MAGENTA;
  case TILE_CYAN:
    return M_SKYBLUE;
  case TILE_YELLOW:
    return M_YELLOW;
  case TILE_GREEN:
    return M_GREEN;
  default:
    return M_GRAY;
  }
}

Color to_raylib_color(Clay_Color color) {
  return (Color){color.r, color.g, color.b, color.a};
}

// Core rendering functions
void render_hex_cell(const board_t *board, grid_cell_t cell,
                     Clay_Color fill_color, Clay_Color edge_color) {
  if (!board)
    return;

  // Get hex corners using geometry-agnostic functions
  point_t corners[6];
  grid_geometry_get_corners(board->geometry_type, &board->layout, cell,
                            corners);

  Vector2 verts[6];
  // Reverse vertex order for counter-clockwise winding
  for (int i = 0; i < 6; i++) {
    int reversed_i = (6 - 1) - i;
    verts[i] =
      (Vector2){(float)corners[reversed_i].x, (float)corners[reversed_i].y};
  }

  Color ray_fill_color = to_raylib_color(fill_color);
  Color ray_edge_color = to_raylib_color(edge_color);

  // Draw filled hexagon
  DrawTriangleFan(verts, 6, ray_fill_color);

  // Draw outline if edge color is not blank
}

void render_tile(const tile_t *tile, const board_t *board) {
  if (!tile || !board)
    return;

  Clay_Color tile_color = color_from_tile(tile->data);
  render_hex_cell(board, tile->cell, tile_color, M_BLANK); // No edges on tiles
}

static void draw_tile_wrapper(tile_t *tile, void *user_data) {
  const board_t *board = (const board_t *)user_data;
  render_tile(tile, board);
}

void render_hex_cells_batched(const board_t *board, grid_cell_t *cells,
                              size_t count, Clay_Color fill_color) {
  if (!board || !cells || count == 0)
    return;

  Color ray_fill_color = to_raylib_color(fill_color);

  // Use batched approach: collect all vertices for this color and draw with
  // minimal calls
  Vector2 *vertices = malloc(count * 6 * sizeof(Vector2));
  if (!vertices)
    return;

  size_t batch_size = 0;
  const size_t MAX_BATCH = 1000; // Process in batches to avoid memory issues

  for (size_t i = 0; i < count; i++) {
    point_t corners[6];
    grid_geometry_get_corners(board->geometry_type, &board->layout, cells[i],
                              corners);

    Vector2 hex_verts[6];
    for (int j = 0; j < 6; j++) {
      int reversed_j = (6 - 1) - j;
      hex_verts[j] =
        (Vector2){(float)corners[reversed_j].x, (float)corners[reversed_j].y};
    }

    // Draw this hex using triangle fan (most efficient for individual hexes)
    DrawTriangleFan(hex_verts, 6, ray_fill_color);

    batch_size++;

    // If we hit batch limit, we could implement more sophisticated batching
    // here
    if (batch_size >= MAX_BATCH) {
      batch_size = 0;
    }
  }

  free(vertices);
}

void render_board_batched(const board_t *board) {
  if (!board || !board->tiles) {
    printf("ERROR: board or tiles is null\n");
    return;
  }

  // Group tiles by color
  typedef struct {
    grid_cell_t *cells;
    size_t count;
    size_t capacity;
  } tile_group_t;

  tile_group_t groups[TILE_TYPE_COUNT] = {0};

  // Initialize groups
  for (int i = 0; i < TILE_TYPE_COUNT; i++) {
    groups[i].capacity = 64; // Start with reasonable capacity
    groups[i].cells = malloc(groups[i].capacity * sizeof(grid_cell_t));
  }

  // Group tiles by type
  tile_map_entry_t *entry, *tmp;
  HASH_ITER(hh, board->tiles->root, entry, tmp) {
    tile_t *tile = entry->tile;
    if (tile->data.type >= 0 && tile->data.type < TILE_TYPE_COUNT) {
      tile_group_t *group = &groups[tile->data.type];

      // Expand capacity if needed
      if (group->count >= group->capacity) {
        group->capacity *= 2;
        group->cells =
          realloc(group->cells, group->capacity * sizeof(grid_cell_t));
      }

      group->cells[group->count++] = tile->cell;
    }
  }

  // Render each color group
  for (int i = 1; i < TILE_TYPE_COUNT; i++) { // Skip TILE_EMPTY
    if (groups[i].count > 0) {
      Clay_Color color = color_from_tile((tile_data_t){.type = i, .value = 0});
      render_hex_cells_batched(board, groups[i].cells, groups[i].count, color);
    }
  }

  // Cleanup
  for (int i = 0; i < TILE_TYPE_COUNT; i++) {
    free(groups[i].cells);
  }

  // Render pool boundary edges
  // render_board_edges(board);
}

void render_game(game_t *game) {
  if (!game) {
    printf("ERROR: game is null\n");
    return;
  }

  render_board(game->board);
  render_game_previews(game);
}

void render_board(const board_t *board) {
  if (!board) {
    printf("ERROR: board is null\n");
    return;
  }

  if (!board->tiles) {
    printf("ERROR: board->tiles is null\n");
    return;
  }

  // Switch between batched and individual rendering
  // For now, use batched rendering for better performance
  render_board_batched(board);

  // Uncomment below to use individual tile rendering instead:
  // tile_map_foreach_tile(board->tiles, draw_tile_wrapper, (void *)board);
  // render_board_edges(board);
}

void render_board_edges(const board_t *board) {
  if (!board || !board->edge_list)
    return;

  // Get edges and render them
  size_t edge_count;
  const render_edge_t *edges =
    edge_render_list_get_edges(board->edge_list, &edge_count);
  if (edges) {
    for (size_t i = 0; i < edge_count; i++) {
      const render_edge_t *edge = &edges[i];
      DrawLineEx(edge->start, edge->end, edge->thickness, edge->color);
    }
  }

  // Get vertices and render them
  size_t vertex_count;
  const render_vertex_t *vertices =
    edge_render_list_get_vertices(board->edge_list, &vertex_count);
  if (vertices) {
    for (size_t i = 0; i < vertex_count; i++) {
      const render_vertex_t *vertex = &vertices[i];
      DrawCircleV(vertex->position, vertex->radius, vertex->color);
    }
  }
}

void render_game_previews(const game_t *game) {
  if (!game || !game->preview.is_active || !game->preview.source_board) {
    return;
  }

  // Calculate offset from source center to target position
  grid_cell_t source_center =
    grid_geometry_get_origin(game->preview.source_board->geometry_type);
  grid_cell_t offset = grid_geometry_calculate_offset(
    game->preview.source_board->geometry_type, source_center,
    game->preview.target_position);

  // Get conflicts using the simplified system
  grid_cell_t *conflicts;
  size_t conflict_count;
  bool has_conflicts =
    game_get_preview_conflicts(game, &conflicts, &conflict_count);

  // Render all source tiles with preview colors
  tile_map_entry_t *entry, *tmp;
  HASH_ITER(hh, game->preview.source_board->tiles->root, entry, tmp) {
    tile_t *source_tile = entry->tile;
    grid_cell_t target_pos = grid_geometry_apply_offset(
      game->preview.source_board->geometry_type, source_tile->cell, offset);

    // Check if this position is valid (within board bounds)
    grid_cell_t origin = grid_geometry_get_origin(game->board->geometry_type);
    int distance =
      grid_geometry_distance(game->board->geometry_type, target_pos, origin);
    bool is_valid_cell = (distance <= game->board->radius);

    if (is_valid_cell) {
      // Check if this position conflicts
      bool is_conflict = false;
      if (has_conflicts && conflicts) {
        for (size_t i = 0; i < conflict_count; i++) {
          if (grid_cells_equal(&target_pos, &conflicts[i])) {
            is_conflict = true;
            break;
          }
        }
      }

      if (is_conflict) {
        // Render conflict in red
        render_hex_cell(game->board, target_pos,
                        (Clay_Color){255, 0, 0, 180},  // Semi-transparent red
                        (Clay_Color){255, 0, 0, 255}); // Red border
      } else {
        // Render valid preview tile with transparency
        Clay_Color tile_color = color_from_tile(source_tile->data);
        Clay_Color preview_color =
          (Clay_Color){tile_color.r, tile_color.g, tile_color.b, 180};
        render_hex_cell(game->board, target_pos, preview_color,
                        (Clay_Color){0, 255, 0, 255}); // Green border for valid
      }
    }
  }

  // Free conflicts array if allocated
  if (conflicts) {
    free(conflicts);
  }
}

void render_hex_grid(const board_t *board) {
  if (!board)
    return;

  // Get all coordinates for this geometry type and radius
  grid_cell_t *all_coords;
  size_t coord_count;
  grid_cell_t origin = grid_geometry_get_origin(board->geometry_type);
  grid_geometry_get_cells_in_range(board->geometry_type, origin, board->radius,
                                   &all_coords, &coord_count);
  if (!all_coords || coord_count == 0) {
    return;
  }

  // Render grid outline
  for (size_t i = 0; i < coord_count; i++) {
    render_hex_cell(board, all_coords[i], M_BLANK, M_GRAY);
  }

  free(all_coords);
}

void render_board_in_bounds(const board_t *board, Rectangle bounds) {
  if (!board || bounds.width <= 0 || bounds.height <= 0) {
    return;
  }

  // Get all coordinates to calculate board bounding box
  grid_cell_t *all_coords;
  size_t coord_count;
  grid_cell_t origin = grid_geometry_get_origin(board->geometry_type);
  grid_geometry_get_cells_in_range(board->geometry_type, origin, board->radius,
                                   &all_coords, &coord_count);
  if (!all_coords || coord_count == 0) {
    return;
  }

  // Calculate board bounding box using the optimized function
  float min_x, min_y, max_x, max_y;
  if (!grid_geometry_calculate_bounds(board->geometry_type, &board->layout,
                                      all_coords, coord_count, &min_x, &min_y,
                                      &max_x, &max_y)) {
    free(all_coords);
    return;
  }

  free(all_coords);

  float board_width = max_x - min_x;
  float board_height = max_y - min_y;

  if (board_width <= 0 || board_height <= 0) {
    return;
  }

  // Calculate scale to fit with padding
  float padding = 10.0f;
  float scale_x = (bounds.width - padding * 2) / board_width;
  float scale_y = (bounds.height - padding * 2) / board_height;
  float scale = fminf(scale_x, scale_y);

  // Calculate center positions
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
  render_board(board);
  EndMode2D();
}

void render_inventory(const inventory_t *inventory) {
  if (!inventory) {
    return;
  }
  for (size_t i = 0; i < kv_size(inventory->items); i++) {

    inventory_item_t *item = &kv_A(inventory->items, i);

    if (item && item->board && is_id_valid(item->id)) {
      Clay_BoundingBox boundingBox = Clay_GetElementData(item->id).boundingBox;
      if (boundingBox.width > 0 && boundingBox.height > 0) {
        Rectangle bounds = {.x = boundingBox.x,
                            .y = boundingBox.y,
                            .width = boundingBox.width,
                            .height = boundingBox.height};
        render_board_in_bounds(item->board, bounds);
      }

      // Highlight selected item
      if ((int)i == inventory->selected_index) {
        // Add selection highlighting here if needed
      }
    }
  }
}
