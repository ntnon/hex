#include "board_input_controller.h"
void
renderer_draw (const board_t *board,
               const board_input_controller_t *input_ctrl);
void renderer_draw_board(const board_t *board);
void renderer_draw_grid(const grid_t *grid);
void renderer_draw_tile(const tile_t *tile, const grid_t *grid);
void
renderer_draw_hover_info (const board_t *board,
                          const board_input_controller_t *ctrl);
