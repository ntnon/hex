#include "controller/input_state.h"
#include "render/renderer.h"

#define UI_HEIGHT 50

void
game_screen_init (game_screen_t *game, int w, int h)
{

  // Initialize game state variables
  game->score = 0;
  // Initialize other game state variables as needed
  // Example: game->player = initialize_player();

  game->board = board_create (GRID_TYPE_HEXAGON, 5);
  board_randomize (game->board);

  game->inventory = inventory_create (5);
  inventory_fill (game->inventory, 5);
  // Set up UI bounds (if needed)
  game->ui_bounds = (rect_t){ 0, h - UI_HEIGHT, w, UI_HEIGHT };
}

game_action_t
game_screen_update (game_screen_t *game, input_state_t *input)
{
  // Update game logic here (e.g., player movement, collision detection)
  if (!game || !input)
    return GAME_ACTION_NONE;
  return GAME_ACTION_NONE;
}

void
game_screen_draw (const game_screen_t *game)
{
  // Draw game elements (e.g., player, enemies, etc.)
  // Example: DrawPlayer(game->player);

  render_board (game->board);
  render_inventory (game->inventory);
}

void
game_screen_unload (void *game_screen)
{
  if (!game_screen)
    return;

  game_screen_t *game = (game_screen_t *)game_screen;

  if (game->board)
    {
      free_board (game->board);
      game->board = NULL;
    }

  if (game->inventory)
    {
      free_inventory (game->inventory);
      game->inventory = NULL;
    }
}
void
game_input_handler (void *screen_data, input_state_t *input)
{
  game_screen_t *game = (game_screen_t *)screen_data;
  if (!game)
    return;
}

void
game_action_handler (void *screen_data, screen_manager_t *mgr, bool *running)
{
  game_screen_t *game = (game_screen_t *)screen_data;
  screen_manager_t *screen_mgr = (screen_manager_t *)mgr;

  if (!game || !screen_mgr || !running)
    return;

  // Handle game actions based on input
  if (game->score < 0)
    {                   // Example condition to quit
      *running = false; // End the game
    }
}

void
game_render_handler (void *screen_data)
{
  game_screen_t *game = (game_screen_t *)screen_data;
  if (!game)
    return;

  game_screen_draw (game);
}
