#include "../third_party/clay.h"
#include "../game/board.h"
#include "../game/inventory.h"
<<<<<<< HEAD
#include "../screen/game_screen.h"
#include "../screen/menu_screen.h"
=======
#include "raylib.h"

>>>>>>> pre_slop

// void
// render (const board_t *board,
//                const board_input_controller_t *input_ctrl);@
Color to_raylib_color(Clay_Color color);
Clay_Color color_from_tile(tile_data_t tile_type);
void render_board(const board_t *board);
void render_hex_grid(const grid_t *grid);
void render_tile(const tile_t *tile, const grid_t *grid);
void
render_hex_cell (const grid_t *grid, grid_cell_t cell, Clay_Color fill_color,
                 Clay_Color edge_color);

<<<<<<< HEAD
rect_t
=======
//inventory helper functions
Rectangle
>>>>>>> pre_slop
inventory_get_slot_rect (const inventory_t *inv, int i);
void render_inventory(inventory_t *inventory);
<<<<<<< HEAD
void render_inventory_item(const inventory_t *inventory, int index, bool selected);
void render_game_screen(const game_screen_t *game_screen);

void
render_menu_screen (menu_screen_t *menu);
=======
>>>>>>> pre_slop
