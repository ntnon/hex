#include "../../include/game/game.h"
#include "stdio.h"
#include <stdlib.h>

game_t *
game_create ()
{
  game_t *game = malloc (sizeof (game_t));
  game->board = board_create (GRID_TYPE_HEXAGON, 5);
  game->inventory = inventory_create (3);
  return game;
}

void
free_game (game_t *game)
{
  if (!game)
    return;

  free_board (game->board);
  free_inventory (game->inventory);
  free (game);
}
