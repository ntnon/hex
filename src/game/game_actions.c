#include "game/game_actions.h"
#include "game/board.h"
#include "game/game.h"
#include "game/inventory.h"
#include "grid/grid_geometry.h"
#include <stdio.h>

void game_actions_init(game_actions_t *actions, game_t *game) {
  actions->game = game;
}

/* Inventory actions */
void game_actions_add_inventory_item(game_actions_t *actions) {
  inventory_add_random_item(actions->game->inventory);
  printf("Added random item to inventory\n");
}

void game_actions_rotate_selected_item(game_actions_t *actions, int direction) {
  if (inventory_rotate_selected(actions->game->inventory, direction)) {
    printf("Rotated inventory item %s\n",
           direction > 0 ? "clockwise" : "counterclockwise");
  }
}

void game_actions_select_inventory_item(game_actions_t *actions, int index) {
  int inventory_size = inventory_get_size(actions->game->inventory);
  if (index >= 0 && index < inventory_size) {
    inventory_set_index(actions->game->inventory, index);
    printf("Selected inventory item at index %d\n", index);
  } else {
    printf("Invalid inventory index: %d\n", index);
  }
}

/* Board actions */
bool game_actions_place_tile(game_actions_t *actions,
                             grid_cell_t target_position) {
  board_t *selected_board =
    inventory_get_selected_board(actions->game->inventory);

  if (!selected_board) {
    printf("No inventory item selected for placement\n");
    return false;
  }

  printf("Attempting to place tile at position (%d, %d)\n",
         target_position.coord.hex.q, target_position.coord.hex.r);

  grid_cell_t source_center =
    grid_get_center_cell(selected_board->geometry_type);

  if (merge_boards(actions->game->board, selected_board, target_position,
                   source_center)) {
    printf("Successfully placed tile on main board\n");
    game_actions_enter_collect_state(actions);
    return true;
  } else {
    printf("Failed to place tile - invalid placement\n");
    return false;
  }
}

void game_actions_enter_collect_state(game_actions_t *actions) {
  printf("Entering collect state\n");
  actions->game->state = GAME_STATE_COLLECT;
  actions->game->inventory->selected_index = -1;
}

/* State transitions */
void game_actions_cycle_state(game_actions_t *actions) {
  game_state_cycle(actions->game);
}
