#include "tile/tile.h"
#include "tile/tile_map.h"
#include <math.h>
#include <stdio.h>

#include "../include/grid/grid_cell_utils.h"
#include "../include/grid/grid_geometry.h"
#include "../include/renderer.h"
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
  grid_get_cell_corners(board->geometry_type, &board->layout, cell, corners);

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

  for (int i = 0; i < 6; i++) {
    DrawLineEx(verts[i], verts[(i + 1) % 6], 0.5f, ray_edge_color);
    DrawCircleV(verts[i], 0.25f, ray_edge_color);
  }
}

void render_tile(const tile_t *tile, const board_t *board) {
  if (!tile || !board)
    return;

  Clay_Color tile_color = color_from_tile(tile->data);
  render_hex_cell(board, tile->cell, tile_color, M_BLACK);
}

static void draw_tile_wrapper(tile_t *tile, void *user_data) {
  const board_t *board = (const board_t *)user_data;
  render_tile(tile, board);
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

  tile_map_foreach_tile(board->tiles, draw_tile_wrapper, (void *)board);
}

void render_game_previews(const game_t *game) {
  if (!game || !game->preview.is_active || !game->preview.source_board) {
    return;
  }

  // Calculate offset from source center to target position
  grid_cell_t source_center =
    grid_get_center_cell(game->preview.source_board->geometry_type);
  grid_cell_t offset =
    grid_calculate_offset(game->preview.target_position, source_center);

  // Get conflicts using the simplified system
  grid_cell_t *conflicts;
  size_t conflict_count;
  bool has_conflicts =
    game_get_preview_conflicts(game, &conflicts, &conflict_count);

  // Render all source tiles with preview colors
  tile_map_entry_t *entry, *tmp;
  HASH_ITER(hh, game->preview.source_board->tiles->root, entry, tmp) {
    tile_t *source_tile = entry->tile;
    grid_cell_t target_pos = grid_apply_offset(source_tile->cell, offset);

    // Check if this position is valid (within board bounds)
    bool is_valid_cell =
      grid_is_valid_cell_with_radius(target_pos, game->board->radius);

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
  if (!grid_get_all_coordinates_in_radius(board->geometry_type, board->radius,
                                          &all_coords, &coord_count)) {
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
  if (!grid_get_all_coordinates_in_radius(board->geometry_type, board->radius,
                                          &all_coords, &coord_count)) {
    return;
  }

  if (coord_count == 0) {
    free(all_coords);
    return;
  }

  // Calculate board bounding box by sampling a few cells
  float min_x = 1e6f, min_y = 1e6f, max_x = -1e6f, max_y = -1e6f;

  for (size_t i = 0; i < coord_count; i += (coord_count / 10 + 1)) {
    point_t corners[6];
    grid_get_cell_corners(board->geometry_type, &board->layout, all_coords[i],
                          corners);
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
