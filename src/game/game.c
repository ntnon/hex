#include "../../include/game/game.h"
#include <stdlib.h>

void
game_init (game_t *game)
{
  // Initialize inventory with a fixed size (e.g., 5)
  game->inventory = *create_inventory (5);

  // Initialize board (assuming you have a board_init function)
  game->board = *board_create (GRID_TYPE_HEXAGON, 20);

  board_input_controller_t ctrl;
  board_input_controller_init (&ctrl);
  game->input_ctrl = &ctrl;
}

void
game_update (game_t *game)
{
  // Update board logic (assuming you have a board_update function)

  // Inventory update logic can go here if needed
}

void
game_render (game_t *game)
{
  // Render the board first
  renderer_draw_board (&game->board);

  // Render the inventory overlay
  inventory_render (&game->inventory);
}

void
free_game (game_t *game)
{
  // Free inventory resources (assuming you have inventory_destroy)
  free_inventory (&game->inventory);

  // Free board resources (assuming you have board_destroy)
  free_board (&game->board);
}
