#include "../game/board.h"
#include "../game/inventory.h"
#include "../screen/game_screen.h"

// void
// render (const board_t *board,
//                const board_input_controller_t *input_ctrl);
void render_board(const board_t *board);
void render_hex_grid(const grid_t *grid);
void render_tile(const tile_t *tile, const grid_t *grid);
void
render_hex_cell (const grid_t *grid, grid_cell_t cell, color_t fill_color,
                 color_t edge_color);

rect_t
inventory_get_slot_rect (const inventory_t *inv, int i);
void render_inventory(inventory_t *inventory);
void render_inventory_item(const inventory_t *inventory, int index, bool selected);
void render_game_screen(const game_screen_t *game_screen);
