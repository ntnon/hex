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

/* Optimized rendering functions (require raylib initialization) */
void render_board_optimized(const board_t *board);
void render_hex_grid_optimized(const grid_t *grid);
void render_board_previews(const board_t *board);
void render_board_in_bounds(const board_t *board, Rectangle bounds);
void renderer_cleanup(void);

/* Rendering mode controls - only mesh instancing supported */






#endif // RENDERER_H
