#include "tile.h"
#include "hex_grid.h"
#include "piece.h"
#include "highlight_manager.h"

typedef struct game_board {
    int id;
    int radius;
    layout layout;
    hex center;
    hex_array hex_array;
    tile_array *tile_array;
    bool is_dirty;
    highlight_manager *highlight_manager;
    int ghost_tiles;
    int magentas;
    int cyans;
    int yellows;
    int magenta_production;
    int cyan_production;
    int yellow_production;
} game_board;

game_board* game_board_new(int radius);

void game_board_free(game_board* board);

void game_board_randomize(game_board* board);

game_board *game_board_clone(const game_board* board);

void game_board_add_piece(game_board* board, piece* piece);

void game_board_highlight_hex(game_board* board, hex* hex);

void game_board_hover_piece(game_board* board, piece* piece);

void game_board_unhover_piece(game_board* board, piece* piece);

void game_board_place_piece(game_board* board, piece* piece);

void game_board_remove_piece(game_board* board, piece* piece);

void update_tile_groups(game_board* board);

void validate_tile_groups(game_board* board);

piece* get_piece(game_board* board, int index);

void game_board_draw(game_board* board);

void game_board_print(game_board* board);

void game_board_update_tile(game_board* board, tile *target_tile, tile *new_tile);

void
tile_array_cycle_tile (tile_array *tile_array, int index);
