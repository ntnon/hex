#include "game/game.h"
#include "game/board.h"
#include "grid/grid_geometry.h"
#include "raylib.h"
#include "stdio.h"

void game_init(game_t *game) {
  game->board = board_create(GRID_TYPE_HEXAGON, 200, BOARD_TYPE_MAIN);
  game->inventory = inventory_create(2);
  game->reward_count = 3;
  game->state = GAME_STATE_PLAYING;
  // board_randomize(game->board);
  // board_fill(game->board, 1, BOARD_TYPE_MAIN);
  // inventory_fill(game->inventory, 1);
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
    board_clear_preview_boards(game->board);
    return;
  }

  // Use the board's current hovered position as the target
  if (game->board->hovered_grid_cell) {
    board_update_preview(game->board, selected_board,
                         *(game->board->hovered_grid_cell));
  } else {
    // No hover position, clear any existing previews
    board_clear_preview_boards(game->board);
  }
}

void update_game(game_t *game, const input_state_t *input) {
  // Transform mouse coordinates from screen space to world space
  Vector2 world_mouse = GetScreenToWorld2D(
    (Vector2){input->mouse.x, input->mouse.y}, game->board->camera);

  // Convert pixel to cell using geometry-agnostic function
  static grid_cell_t hovered_cell;
  if (grid_pixel_to_cell(
        game->board->geometry_type, &game->board->layout, game->board->radius,
        (point_t){world_mouse.x, world_mouse.y}, &hovered_cell)) {
    game->board->hovered_grid_cell = &hovered_cell;
  } else {
    game->board->hovered_grid_cell = NULL;
  }

  // Update board preview based on selected inventory item
  update_board_preview(game);

  // Removed excessive coordinate printing that was happening every frame
}

void game_state_cycle(game_t *game) {
  game->state = (game->state + 1) % GAME_STATE_COUNT;
}
