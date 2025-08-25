#include "game/game.h"
#include "grid/grid_system.h"
#include "raylib.h"
#include "stdio.h"

void game_init(game_t *game) {
  game->board = board_create(GRID_TYPE_HEXAGON, 4);
  game->inventory = inventory_create(3);
  game->hovered_grid_cell = NULL;
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

void update_game(game_t *game, const input_state_t *input) {
  // Transform mouse coordinates from screen space to world space
  Vector2 world_mouse = GetScreenToWorld2D(
    (Vector2){input->mouse.x, input->mouse.y}, game->board->camera);

  game->hovered_grid_cell = grid_get_cell_at_pixel(
    game->board->grid, (point_t){world_mouse.x, world_mouse.y});

  if (game->hovered_grid_cell) {
    print_cell(game->board->grid, *(game->hovered_grid_cell));
  }
}

void game_render(game_t *game, const input_state_t *input) {}
