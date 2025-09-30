#ifndef GAME_H
#define GAME_H

#include "game/board.h"
#include "game/inventory.h"
#include "controller/input_state.h"
#include "rule_manager.h"
typedef enum {
    //GAME_STATE_MENU,
    GAME_STATE_PLAYING,
    GAME_STATE_COLLECT,
    GAME_STATE_REWARD,

    GAME_STATE_GAME_OVER,
    GAME_STATE_COUNT
} game_state_e;

typedef struct game {
    board_t *board;
    inventory_t *inventory;
    int reward_count;
    game_state_e state;
    //rule_manager_t *rule_manager;
} game_t;

/* Function declarations */

void game_init(game_t *game);
void free_game(game_t *game);
void update_game(game_t *game, const input_state_t *input);
void update_board_preview(game_t *game);
void game_render(game_t *game, const input_state_t *input);
void game_state_cycle(game_t *game);


#endif // GAME_H
