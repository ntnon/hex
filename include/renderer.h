#include "board/board.h"
// void
// render (const board_t *board,
//                const board_input_controller_t *input_ctrl);
void render_board(const board_t *board);
void render_grid(const grid_t *grid);
void render_tile(const tile_t *tile, const grid_t *grid);
// void
// render_hover_info (const board_t *board,
//                           const board_input_controller_t *ctrl);
void
render_hex_cell (const grid_t *grid, grid_cell_t cell, Color fill_color,
                 Color edge_color);
