#include "game/board.h"
#include "game/inventory.h"
#include "input_state.h"

typedef struct game {
    board_t *board;
    inventory_t *inventory;
} game_t;

void game_init(game_t *game);

void free_game(game_t *game);

void game_render(game_t *game, const input_state_t *input);
