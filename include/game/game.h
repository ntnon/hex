#ifndef GAME_H
#define GAME_H

#include "game/board.h"
#include "game/inventory.h"
#include "controller/input_state.h"
//#include "rule_manager.h"
#include "tile/tile.h"


typedef struct simple_preview_t {
    board_t *source_board;           /* Board being previewed for placement */
    grid_cell_t target_position;     /* Where the center would be placed */
    bool is_active;                  /* Whether preview is currently active */
} simple_preview_t;


typedef struct game {
    board_t *board;
    inventory_t *inventory;
    int reward_count;

    bool round_count;
    bool is_paused;
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

/* Game-level business logic */
bool game_try_place_tile(game_t *game, grid_cell_t target_position);
bool game_try_select_inventory_item(game_t *game, int index);
void game_add_random_inventory_item(game_t *game);
void game_exit_placement_mode(game_t *game);

// Simplified preview system functions
void game_set_preview(game_t *game, board_t *source_board, grid_cell_t target_position);
void game_clear_preview(game_t *game);
bool game_get_preview_conflicts(const game_t *game, grid_cell_t **out_conflicts, size_t *out_count);

#endif // GAME_H
