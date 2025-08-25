#ifndef GAME_H
#define GAME_H

#include "game/board.h"
#include "game/inventory.h"
#include "controller/input_state.h"

typedef struct game {
    board_t *board;
    inventory_t *inventory;
    grid_cell_t *hovered_grid_cell;
} game_t;

/* Function declarations */

void game_init(game_t *game);
void free_game(game_t *game);
void update_game(game_t *game, const input_state_t *input);
void game_render(game_t *game, const input_state_t *input);

#endif // GAME_H
