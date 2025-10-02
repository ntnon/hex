#ifndef RENDERER_H
#define RENDERER_H

#include "third_party/clay.h"
#include "game/board.h"
#include "game/game.h"
#include "raylib.h"

/**
 * @brief Rendering system for the game
 */

/* Function declarations */

Color to_raylib_color(Clay_Color color);
Clay_Color color_from_tile(tile_data_t tile_type);
void render_board(const board_t *board);
void render_hex_grid(const board_t *board);
void render_tile(const tile_t *tile, const board_t *board);
void render_hex_cell(const board_t *board, grid_cell_t cell, Clay_Color fill_color,
                     Clay_Color edge_color);
void render_hex_rounded_outline(const board_t *board, grid_cell_t cell,
                                Clay_Color edge_color, float thickness);

/* Optimized rendering functions (require raylib initialization) */
void render_board_optimized(const board_t *board);
void render_hex_grid_optimized(const grid_t *grid);
void render_board_edges(const board_t *board);
void render_game_previews(const game_t *game);
void render_board_in_bounds(const board_t *board, Rectangle bounds);
void render_inventory(const inventory_t *inventory);
void renderer_cleanup(void);

/* Rendering mode controls - only mesh instancing supported */






#endif // RENDERER_H
