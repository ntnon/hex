#include "../../include/screen/game_screen.h"
#include "../../include/renderer.h"
#include "raylib.h"

#define UI_HEIGHT 50

void
game_screen_init (game_screen_t *game)
{
  // Initialize game state variables
  game->score = 0;
  // Initialize other game state variables as needed
  // Example: game->player = initialize_player();

  game->board = board_create (GRID_TYPE_HEXAGON, 5);

  game->inventory = inventory_create (5);
  // Set up UI bounds (if needed)
  game->ui_bounds = (Rectangle){ 0, GetScreenHeight () - UI_HEIGHT,
                                 GetScreenWidth (), UI_HEIGHT };
}

game_action_t
game_screen_update (game_screen_t *game, Vector2 mouse, bool mouse_pressed)
{
  // Update game logic here (e.g., player movement, collision detection)
  if (!game || !mouse.x || !mouse_pressed)
    return GAME_ACTION_NONE;
  // Example: Check for quit action
  if (IsKeyPressed (KEY_ESCAPE))
    {
      return GAME_ACTION_QUIT;
    }

  // Handle other game interactions
  // Example: Update score based on game events

  return GAME_ACTION_NONE;
}

void
game_screen_draw (const game_screen_t *game)
{
  // Draw game elements (e.g., player, enemies, etc.)
  // Example: DrawPlayer(game->player);

  render_board (game->board);
}

void
game_screen_unload (game_screen_t *game)
{
  if (!game)
    return;
  if (game->board)
    {

      free_board (game->board);
    }
  if (game->inventory)
    {

      free_inventory (game->inventory);
    }
  free (game);
}

void
game_input_handler (void *screen_data)
{
  game_screen_t *game = (game_screen_t *)screen_data;
  if (!game)
    return;

  Vector2 mouse = GetMousePosition ();
  bool mouse_pressed = IsMouseButtonPressed (MOUSE_LEFT_BUTTON);

  // Update game logic based on input
  game_action_t action = game_screen_update (game, mouse, mouse_pressed);

  // Handle any specific actions (e.g., pause, quit)
  if (action == GAME_ACTION_QUIT)
    {
      // Handle quit action
    }
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
