#include "game/game.h"
#include "game/board.h"
#include "game/reward_state.h"
#include "grid/grid_geometry.h"
#include "raylib.h"
#include "stdio.h"
#include <time.h>

void game_init(game_t *game) {
  game->board = board_create(GRID_TYPE_HEXAGON, 50, BOARD_TYPE_MAIN);
  game->inventory = inventory_create(2);
  game->reward_count = 3;

  // Initialize rule manager
  game->rule_manager = malloc(sizeof(rule_manager_t));
  if (game->rule_manager) {
    if (!rule_manager_init(game->rule_manager, game->board, 1000)) {
      free(game->rule_manager);
      game->rule_manager = NULL;
      printf("Failed to initialize rule manager\n");
    } else {
      printf("Rule manager initialized successfully\n");
    }
  }

  // Initialize reward state
  game->reward_state = malloc(sizeof(reward_state_t));
  if (game->reward_state) {
    if (!reward_state_init(game->reward_state, (uint32_t)time(NULL))) {
      free(game->reward_state);
      game->reward_state = NULL;
      printf("Failed to initialize reward state\n");
    } else {
      printf("Reward state initialized successfully\n");
    }
  }

  // Initialize hover system
  game->hovered_tile = NULL;
  game->hovered_cell = (grid_cell_t){0};
  game->should_show_tile_info = false;

  // Initialize simplified preview system
  game->preview.source_board = NULL;
  game->preview.target_position = (grid_cell_t){0};
  game->preview.is_active = false;

  game->state = GAME_STATE_PLAYING;
  // board_randomize(game->board);
  // Use smaller radius for testing to avoid long load times
  printf("Board created with radius: %d\n", game->board->radius);
  // board_fill_batch(game->board, 30, BOARD_TYPE_MAIN); // Small radius for
  //  fast loading
  inventory_fill(game->inventory, 1);
}

void free_game(game_t *game) {
  if (game) {
    if (game->board) {
      free_board(game->board);
    }
    if (game->inventory) {
      free_inventory(game->inventory);
    }
    if (game->rule_manager) {
      rule_manager_cleanup(game->rule_manager);
      free(game->rule_manager);
    }
    if (game->reward_state) {
      reward_state_cleanup(game->reward_state);
      free(game->reward_state);
    }
    // Clear preview (no memory to free in simplified system)
    game_clear_preview(game);
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
    game_clear_preview(game);
    return;
  }

  // Use the board's current hovered position as the target (if we have a valid
  // hover)
  if (game->hovered_cell.type != GRID_TYPE_UNKNOWN) {
    // Update preview at hovered position
    game_set_preview(game, selected_board, game->hovered_cell);
  } else {
    // No hover position, clear any existing previews
    game_clear_preview(game);
  }
}

void update_game(game_t *game, const input_state_t *input) {
  // Update reward state if active
  if (game->reward_state && reward_state_is_active(game->reward_state)) {
    reward_state_update(game->reward_state, game, GetFrameTime());
    return; // Don't update game logic while in reward state
  }

  // Transform mouse coordinates from screen space to world space
  Vector2 world_mouse = GetScreenToWorld2D(
    (Vector2){input->mouse.x, input->mouse.y}, game->board->camera);

  // Convert pixel to cell using pure geometry conversion
  if (grid_pixel_to_cell(game->board->geometry_type, &game->board->layout,
                         (point_t){world_mouse.x, world_mouse.y},
                         &game->hovered_cell)) {

    // Check if there's a tile at the hovered cell (only for existing tiles)
    // But first verify the coordinate is within board bounds
    bool is_within_bounds =
      grid_is_valid_cell_with_radius(game->hovered_cell, game->board->radius);

    if (is_within_bounds) {
      game->hovered_tile = get_tile_at_cell(game->board, game->hovered_cell);
    } else {
      game->hovered_tile = NULL;
    }

    // Determine if we should show tile info card
    // Show info only if: hovering a tile AND not in placement mode
    bool in_placement_mode =
      (inventory_get_selected_board(game->inventory) != NULL);
    game->should_show_tile_info =
      (game->hovered_tile != NULL && !in_placement_mode);

  } else {
    // Clear hover state if pixel-to-cell conversion failed
    game->hovered_tile = NULL;
    game->hovered_cell = (grid_cell_t){.type = GRID_TYPE_UNKNOWN};
    game->should_show_tile_info = false;
  }

  // Update board preview based on selected inventory item
  update_board_preview(game);

  // Removed excessive coordinate printing that was happening every frame
}

// Simplified preview system functions
void game_set_preview(game_t *game, board_t *source_board,
                      grid_cell_t target_position) {
  if (!game)
    return;

  game->preview.source_board = source_board;
  game->preview.target_position = target_position;
  game->preview.is_active = (source_board != NULL);
}

void game_clear_preview(game_t *game) {
  if (!game)
    return;

  game->preview.source_board = NULL;
  game->preview.target_position = (grid_cell_t){0};
  game->preview.is_active = false;
}

bool game_get_preview_conflicts(const game_t *game, grid_cell_t **out_conflicts,
                                size_t *out_count) {
  if (!game || !game->preview.is_active || !game->preview.source_board) {
    if (out_conflicts)
      *out_conflicts = NULL;
    if (out_count)
      *out_count = 0;
    return true;
  }

  // Calculate offset from source center to target position
  grid_cell_t source_center =
    grid_get_center_cell(game->preview.source_board->geometry_type);
  grid_cell_t offset =
    grid_calculate_offset(game->preview.target_position, source_center);

  // Use tile_map functions to find conflicts
  return tile_map_find_merge_conflicts(game->preview.source_board->tiles,
                                       game->board->tiles, offset,
                                       out_conflicts, out_count);
}

void game_state_cycle(game_t *game) {
  game->state = (game->state + 1) % GAME_STATE_COUNT;
}

// Reward system functions
bool game_trigger_reward_selection(game_t *game, int trigger) {
  if (!game || !game->reward_state) {
    return false;
  }

  // Don't trigger if already in reward state
  if (reward_state_is_active(game->reward_state)) {
    return false;
  }

  // Check if this trigger should actually offer rewards
  if (!reward_state_should_trigger_reward(game, (reward_trigger_t)trigger)) {
    return false;
  }

  // Enter reward state
  return reward_state_enter(game->reward_state, game,
                            (reward_trigger_t)trigger);
}

bool game_handle_reward_input(game_t *game, const input_state_t *input) {
  if (!game || !game->reward_state || !input) {
    return false;
  }

  if (!reward_state_is_active(game->reward_state)) {
    return false;
  }

  // Handle mouse input
  bool handled =
    reward_state_handle_mouse_input(game->reward_state, input->mouse.x,
                                    input->mouse.y, input->mouse_left_clicked);

  // Handle keyboard input (simplified - would need proper key event handling)
  // This would need to be integrated with the actual input system

  return handled;
}

bool game_is_in_reward_state(const game_t *game) {
  if (!game || !game->reward_state) {
    return false;
  }

  return reward_state_is_active(game->reward_state);
}
