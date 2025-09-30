#ifndef GAME_ACTIONS_H
#define GAME_ACTIONS_H

#include "game/game.h"
#include "controller/input_state.h"

typedef struct {
    game_t *game;
} game_actions_t;

/* Function declarations */
void game_actions_init(game_actions_t *actions, game_t *game);

/* Inventory actions */
void game_actions_add_inventory_item(game_actions_t *actions);
void game_actions_rotate_selected_item(game_actions_t *actions, int direction);
void game_actions_select_inventory_item(game_actions_t *actions, int index);

/* Board actions */
bool game_actions_place_tile(game_actions_t *actions, grid_cell_t target_position);
void game_actions_enter_collect_state(game_actions_t *actions);

/* State transitions */
void game_actions_cycle_state(game_actions_t *actions);

#endif // GAME_ACTIONS_H