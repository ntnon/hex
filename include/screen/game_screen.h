#ifndef GAME_H
#define GAME_H

#include "screen_manager.h"

#include "../game/board.h"
#include "../game/inventory.h"
#include "../controller/game_controller.h"
#include "../types.h"
#include "../game/input_state.h"

// Define game actions (if needed)
typedef enum {
    GAME_ACTION_NONE,
    GAME_ACTION_PAUSE,
    GAME_ACTION_QUIT
} game_action_t;

// Define the game screen structure
typedef struct {
        rect_t ui_bounds;
    // Game state variables (e.g., score, player position)
    int score;
    // Add other relevant game state variables
    // Example: player_t player;
    board_t *board;
    inventory_t *inventory;
    //game_controller_t controller;
    ui_area_t active_area;
    // Define the bounds for any game UI elements

} game_screen_t;

// Function prototypes for game screen management
game_screen_t *game_screen_create(void);
void game_screen_init(game_screen_t *game, int width, int height);
game_action_t game_screen_update(game_screen_t *game, input_state_t *input);
void game_screen_unload(void *game);

// Input and action handlers for integration with input controller
void game_input_handler(void *screen_data, input_state_t *input);
void game_action_handler(void *screen_data, screen_manager_t *mgr, bool *running);
void game_render_handler(void *screen_data);

#endif // game_H
