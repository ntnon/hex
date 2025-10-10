#ifndef GAME_H
#define GAME_H

#include "game/board.h"
#include "game/inventory.h"
#include "controller/input_state.h"
#include "rule_manager.h"
#include "tile/tile.h"

typedef enum {
    GAME_STATE_VIEW,
    GAME_STATE_PLACE,
    GAME_STATE_COLLECT,
    GAME_STATE_REWARD,
    GAME_STATE_GAME_OVER,
    GAME_STATE_COUNT
} game_state_e;

typedef struct simple_preview_t {
    board_t *source_board;           /* Board being previewed for placement */
    grid_cell_t target_position;     /* Where the center would be placed */
    bool is_active;                  /* Whether preview is currently active */
} simple_preview_t;


typedef struct game {
    board_t *board;
    inventory_t *inventory;
    int reward_count;
    rule_manager_t *rule_manager;
    game_state_e state;
    // Hover system
    tile_t *hovered_tile;
    grid_cell_t hovered_cell;
    bool should_show_tile_info;

    // Simplified preview system
    simple_preview_t preview;
} game_t;

/* Function declarations */

void game_init(game_t *game);
void free_game(game_t *game);
void update_game(game_t *game, const input_state_t *input);
void update_board_preview(game_t *game);
void game_render(game_t *game, const input_state_t *input);
void game_state_cycle(game_t *game);

// Simplified preview system functions
void game_set_preview(game_t *game, board_t *source_board, grid_cell_t target_position);
void game_clear_preview(game_t *game);
bool game_get_preview_conflicts(const game_t *game, grid_cell_t **out_conflicts, size_t *out_count);

const char *game_state_to_string(game_state_e *game);
#endif // GAME_H
