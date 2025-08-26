#include "game/game.h"
#include "grid/grid_system.h"
#include "raylib.h"
#include "stdio.h"
#include "third_party/uthash.h"
#include "tile/tile_map.h"

void game_init(game_t *game) {
  game->board = board_create(GRID_TYPE_HEXAGON, 10);
  game->inventory = inventory_create(3);
  // board_randomize(game->board);

  inventory_fill(game->inventory, 3);
}

void free_game(game_t *game) {
  if (game) {
    if (game->board) {
      free_board(game->board);
    }
    if (game->inventory) {
      free_inventory(game->inventory);
    }
    free(game);
  }
}

void update_board_preview(game_t *game) {
  if (!game || !game->board)
    return;

  // Get the selected inventory item's board
  board_t *selected_board = inventory_get_selected_board(game->inventory);
  if (!selected_board) {
    // No inventory item selected, clear any existing previews
    printf("DEBUG: No selected board, clearing previews\n");
    board_clear_preview_boards(game->board);
    return;
  }

  printf("DEBUG: Selected board found with %d tiles\n",
         selected_board->tiles->num_tiles);

  // Use the board's current hovered position as the target
  if (game->board->hovered_grid_cell) {
    printf("DEBUG: Hovered cell found, updating preview\n");
    board_update_preview(game->board, selected_board,
                         *(game->board->hovered_grid_cell));
  } else {
    // No hover position, clear any existing previews
    printf("DEBUG: No hovered cell, clearing previews\n");
    board_clear_preview_boards(game->board);
  }
}

void update_game(game_t *game, const input_state_t *input) {
  // Transform mouse coordinates from screen space to world space
  Vector2 world_mouse = GetScreenToWorld2D(
    (Vector2){input->mouse.x, input->mouse.y}, game->board->camera);

  game->board->hovered_grid_cell = grid_get_cell_at_pixel(
    game->board->grid, (point_t){world_mouse.x, world_mouse.y});

  // Update board preview based on selected inventory item
  update_board_preview(game);

  if (game->board->hovered_grid_cell) {
    print_cell(game->board->grid, *(game->board->hovered_grid_cell));
  }
}
