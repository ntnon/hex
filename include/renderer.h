#ifndef RENDERER_H
#define RENDERER_H

#include "third_party/clay.h"
#include "game/board.h"
#include "raylib.h"

/**
 * @brief Rendering system for the game
 */

/* Function declarations */

Color to_raylib_color(Clay_Color color);
Clay_Color color_from_tile(tile_data_t tile_type);
void render_board(const board_t *board);
void render_hex_grid(const grid_t *grid);
void render_tile(const tile_t *tile, const grid_t *grid);
void render_hex_cell(const grid_t *grid, grid_cell_t cell, Clay_Color fill_color,
                     Clay_Color edge_color);

#endif // RENDERER_H
