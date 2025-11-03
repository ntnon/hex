#include "game/game.h"
#include "game/board.h"
#include "grid/grid_geometry.h"
#include "raylib.h"
#include "stdio.h"
#include "ui.h"

void game_init(game_t *game) {
  game->board = board_create(GRID_TYPE_HEXAGON, 30, BOARD_TYPE_MAIN);
  game->inventory = inventory_create();
  game->reward_count = 3;
  game->is_paused = false;
  game->round_count = 0;

  // Initialize rule manager
  game->rule_manager = malloc(sizeof(rule_manager_t));
  if (game->rule_manager) {
    if (!rule_manager_init(game->rule_manager, game->board, 2000)) {
      free(game->rule_manager);
      game->rule_manager = NULL;
      printf("Failed to initialize rule manager\n");
    } else {
      printf("Rule manager initialized successfully\n");
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

  // State is now managed by controller
  // board_randomize(game->board);
  // Use smaller radius for testing to avoid long load times
  printf("Board created with radius: %d\n", game->board->radius);
  // board_fill_batch(game->board, 30, BOARD_TYPE_MAIN); // Small radius for
  //   fast loading
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
    if (game->rule_manager) {
      rule_manager_cleanup(game->rule_manager);
      free(game->rule_manager);
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
  // Transform mouse coordinates from screen space to world space
  Vector2 world_mouse = GetScreenToWorld2D(
    (Vector2){input->mouse.x, input->mouse.y}, game->board->camera);
  // Set the hovered cell
  game->hovered_cell = grid_geometry_pixel_to_cell(
    game->board->geometry_type, &game->board->layout,
    (point_t){world_mouse.x, world_mouse.y});
  game->hovered_tile = get_tile_at_cell(game->board, game->hovered_cell);
  // Check if there's a tile at the hovered cell (only for existing tiles)
  // But first verify the coordinate is within board bounds
  grid_cell_t origin = grid_geometry_get_origin(game->board->geometry_type);
  int distance = grid_geometry_distance(game->board->geometry_type,
                                        game->hovered_cell, origin);
  bool is_within_bounds = (distance <= game->board->radius);

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
  if (in_placement_mode && ui_was_clicked(UI_ID_GAME_AREA)) {
    printf("in placement mode: %d", in_placement_mode);
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
    grid_geometry_get_origin(game->preview.source_board->geometry_type);
  grid_cell_t offset = grid_geometry_calculate_offset(
    game->preview.source_board->geometry_type, source_center,
    game->preview.target_position);

  // Use tile_map functions to find conflicts
  return tile_map_find_merge_conflicts(game->preview.source_board->tiles,
                                       game->board->tiles, offset,
                                       out_conflicts, out_count);
}

/* Game-level business logic implementations */

bool game_try_place_tile(game_t *game, grid_cell_t target_position) {
  // Get the selected board from inventory
  board_t *selected_board = inventory_get_selected_board(game->inventory);
  if (!selected_board) {
    printf("No inventory item selected for placement\n");
    return false;
  }

  // Get the center position of the source board (origin for inventory boards)
  grid_cell_t source_center =
    grid_geometry_get_origin(selected_board->geometry_type);

  // Attempt to merge the selected board onto the main board
  if (merge_boards(game->board, selected_board, target_position,
                   source_center)) {
    printf("Successfully placed tile at (%d, %d)\n",
           target_position.coord.hex.q, target_position.coord.hex.r);

    // Consume the inventory item that was placed
    // inventory_use_selected(game->inventory);

    // TODO: Check for completed patterns, update score, etc.

    return true;
  } else {
    printf("Cannot place tile at (%d, %d) - position blocked or invalid\n",
           target_position.coord.hex.q, target_position.coord.hex.r);
    return false;
  }
}

bool game_try_select_inventory_item(game_t *game, int index) {
  int inventory_size = inventory_get_size(game->inventory);

  // Validate index
  if (index < 0 || index >= inventory_size) {
    printf("Invalid inventory index: %d\n", index);
    return false;
  }

  int currently_selected = game->inventory->selected_index;

  if (index == currently_selected) {
    // Deselecting current item - call set_index with the same index to trigger
    // deselection
    inventory_set_index(game->inventory, index);
    printf("Deselected inventory item %d\n", index);
  } else {
    // Selecting new item
    inventory_set_index(game->inventory, index);
    printf("Selected inventory item %d for placement\n", index);
  }

  return true;
}

void game_add_random_inventory_item(game_t *game) {
  inventory_add_random_item(game->inventory);
  printf("Added random item to inventory (size: %d)\n",
         inventory_get_size(game->inventory));
}

void game_exit_placement_mode(game_t *game) {
  inventory_set_index(game->inventory, -1);
  printf("Exited placement mode\n");
}
