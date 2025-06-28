#include "game_board.h"

typedef struct {
    game_board *main_board;
    game_board *preview_board;
} game_board_controller;

game_board_controller *game_board_controller_create(game_board *board);
void game_board_controller_free(game_board_controller *controller);
