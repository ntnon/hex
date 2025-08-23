#include "game/game.h"
#include "render/renderer.h"
#include "stdio.h"

void game_init(game_t *game) {
  game->board = board_create(GRID_TYPE_HEXAGON, 4);
  game->inventory = inventory_create(3);

  board_randomize(game->board);
  inventory_fill(game->inventory, 3);
}

void free_game(game_t *game) {
  if (game) {
    if (game->board) {
      free_board(game->board);
    }
    if (game->inventory) {
      free_inventory(game->inventory);
    }
    free(game);
  }
}

void game_render(game_t *game, const input_state_t *input) {}
