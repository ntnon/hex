#include "third_party/uthash.h"
#include "tile/tile.h"
#include "tile/tile_map.h"
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "raylib.h"

#include "ui.h"

#include "../include/grid/grid_cell_utils.h"
#include "../include/renderer.h"

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
        render_hex_cell(grid, cell, M_LIGHTGRAY, M_GRAY);
      }
    }
  }
}

Color to_raylib_color(Clay_Color color) {
  return (Color){color.r, color.g, color.b, color.a};
}

void render_hex_cell(const grid_t *grid, grid_cell_t cell,
                     Clay_Color fill_color, Clay_Color edge_color) {
  int corners_count = 6;
  point_t corners[6];
  grid->vtable->get_corners(grid, cell, corners);

  Vector2 verts[6];
  for (int j = 0; j < corners_count; ++j) {
    verts[j].x = (float)corners[j].x;
    verts[j].y = (float)corners[j].y;
  }

  Color ray_fill_color = to_raylib_color(fill_color);
  Color ray_edge_color = to_raylib_color(edge_color);
  DrawTriangleFan(verts, corners_count, ray_fill_color);

  for (int j = 0; j < corners_count; ++j) {
    int next = (j + 1) % corners_count;
    DrawLineV(verts[j], verts[next], ray_edge_color);
  }
}

void render_board_previews(const board_t *board) {
  if (!board || !board->preview_boards || board->num_preview_boards == 0) {
    return;
  }

  for (size_t i = 0; i < board->num_preview_boards; i++) {
    board_preview_t *preview = &board->preview_boards[i];

    // Always render valid tiles if merged_board exists
    if (preview->merged_board) {
      tile_map_entry_t *entry, *tmp;
      HASH_ITER(hh, preview->merged_board->tiles->root, entry, tmp) {
        tile_t *merged_tile = entry->tile;

        // Check if this tile exists in the original board
        tile_t *original_tile =
          get_tile_at_cell((board_t *)board, merged_tile->cell);

        if (!original_tile) {
          // This is a valid tile - render with actual color and green border
          Clay_Color tile_color = color_from_tile(merged_tile->data);
          Clay_Color preview_color =
            (Clay_Color){tile_color.r, tile_color.g, tile_color.b, 180};
          render_hex_cell(
            preview->merged_board->grid, merged_tile->cell, preview_color,
            (Clay_Color){0, 255, 0, 255}); // Green border for valid
        }
      }
    }

    // Always render conflicts if they exist
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

  free(cells);

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
  render_board(board);
  EndMode2D();
}
