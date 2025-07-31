#ifndef game_H
#define game_H

#include "screen_manager.h"
#include "raylib.h"

#include "../board/board.h"
#include "../game/inventory.h"

// Define game actions (if needed)
typedef enum {
    GAME_ACTION_NONE,
    GAME_ACTION_PAUSE,
    GAME_ACTION_QUIT
} game_action_t;

// Define the game screen structure
typedef struct {
    // Game state variables (e.g., score, player position)
    int score;
    // Add other relevant game state variables
    // Example: player_t player;
    board_t *board;
    inventory_t *inventory;

    // Define the bounds for any game UI elements
    Rectangle ui_bounds;
} game_screen_t;

// Function prototypes for game screen management
void game_screen_init(game_screen_t *game);
game_action_t game_screen_update(game_screen_t *game, Vector2 mouse, bool mouse_pressed);
void game_screen_draw(const game_screen_t *game);
void game_screen_unload(game_screen_t *game);

// Input and action handlers for integration with input controller
void game_input_handler(void *screen_data);
void game_action_handler(void *screen_data, screen_manager_t *mgr, bool *running);
void game_render_handler(void *screen_data);

#endif // game_H
