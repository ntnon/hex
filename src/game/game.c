#include "game/game.h"
#include "stdio.h"

void game_init(game_t *game) {
    game->board = board_create(GRID_TYPE_HEXAGON, 30, BOARD_TYPE_MAIN);
    game->inventory = inventory_create(GRID_TYPE_HEXAGON);
    game->reward_count = 3;
    game->is_paused = false;
    game->round_count = 0;

    game->resources = malloc(sizeof(resources_t) * TILE_TYPE_COUNT);
    if (!game->resources) {
        fprintf(stderr, "Failed to allocate memory for resources\n");
    }
    resources_init(game->resources);

    // Hover system moved to game_controller

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
    // inventory_fill_random(game->inventory, 3);
    inventory_fill_single_tiles(game->inventory);
}

void free_game(game_t *game) {
    if (game) {
        if (game->board) {
            free_board(game->board);
        }
        if (game->inventory) {
            free_inventory(game->inventory);
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

    // Preview position should be passed from controller
    // This function is now called by controller with position
    game_clear_preview(game);
}

void game_update_preview_at_position(game_t *game, grid_cell_t position) {
    board_t *selected_board = inventory_get_selected_board(game->inventory);
    if (!selected_board) {
        game_clear_preview(game);
        return;
    }

    if (position.type != GRID_TYPE_UNKNOWN) {
        game_set_preview(game, selected_board, position);
    } else {
        game_clear_preview(game);
    }
}

void update_game(game_t *game, const input_state_t *input) {}

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
        // Deselecting current item - call set_index with the same index to
        // trigger deselection
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
