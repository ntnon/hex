#include "game_board.h"

typedef struct {
    game_board *main_board;
    game_board *preview_board;
    int generation;
} game_controller;

game_controller *game_controller_create(game_board *board);
void game_controller_free(game_controller *controller);
void progress(game_controller *controller);
void game_controller_draw(game_controller *controller);
