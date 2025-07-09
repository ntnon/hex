#include "inventory.h"
#include "../../include/board/renderer.h"

typedef struct {
inv_t   inventory;
board_t board;
board_input_controller_t *input_ctrl;
} game_t;

void game_init(game_t *game);
void game_update(game_t *game);
void game_render(game_t *game);

void
free_game (game_t *game);
