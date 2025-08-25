#include "../../include/game/board.h"
#include "../../include/game/camera.h"
#include "../../include/third_party/uthash.h"
#include <stdio.h>

#define MAX_POOL_CANDIDATES 10

const orientation_t layout_pointy_t = {.f0 = 1.732050808,
                                       .f1 = 0.866025404,
                                       .f2 = 0.0,
                                       .f3 = 1.5,
                                       .b0 = 0.577350269,
                                       .b1 = -0.333333333,
                                       .b2 = 0.0,
                                       .b3 = 0.666666667,
                                       .start_angle = 0.5};

layout_t default_layout = {
  .orientation = layout_pointy_t,
  .size = {70.0, 70.0}, // Hex size (adjust as needed)
  .origin = {0.0, 0.0}, // Center of the screen (adjust as needed)
  .scale = 1.0f, // scale factor for hex size (adjust as needed)          //
                 // radius of the hexagon (adjust as needed)
};

board_t *board_create(grid_type_e grid_type, int radius) {
  board_t *board = malloc(sizeof(board_t));
  if (!board) {
    fprintf(stderr, "Failed to allocate memory for board\n");
    return NULL;
  }

  board->tiles = tile_map_create();
  board->pools = pool_map_create();
  board->next_pool_id = 1;
  board->grid = grid_create(grid_type, default_layout, radius);
  camera_init(&board->camera);
  return board;
}

void clear_board(board_t *board) {
  tile_map_free(board->tiles);
  pool_map_free(board->pools);
  board->tiles = tile_map_create();
  board->pools = pool_map_create();
  board->next_pool_id = 1;
}

void free_board(board_t *board) {
  tile_map_free(board->tiles);
  pool_map_free(board->pools);
  grid_free(board->grid);
  free(board);
}

bool valid_tile(board_t *board, tile_t *tile) {
  if (!board->grid->vtable->is_valid_cell(board->grid, tile->cell)) {
    fprintf(stderr, "Tile cell not in grid\n");
    return false;
  }
  return true;
}

// Helper function to get tile from cell
tile_t *get_tile_at_cell(board_t *board, grid_cell_t cell) {
  tile_map_entry_t *entry = tile_map_find(board->tiles, cell);
  return entry ? entry->tile : NULL;
}

void get_neighbor_pools(board_t *board, tile_t *tile, pool_t **out_pools,
                        size_t max_neighbors) {
  grid_cell_t neighbor_cells[6];

  board->grid->vtable->get_neighbor_cells(tile->cell, neighbor_cells);

  for (size_t i = 0; i < max_neighbors; ++i) {
    tile_t *neighbor_tile = get_tile_at_cell(board, neighbor_cells[i]);
    if (neighbor_tile) {
      pool_map_entry_t *pool_entry =
        pool_map_find_by_id(board->pools, neighbor_tile->pool_id);
      out_pools[i] = pool_entry ? pool_entry->pool : NULL;

      if (!out_pools[i]) {
        fprintf(stderr,
                "ERROR: Neighbor tile exists but has invalid pool ID!\n");
      }
    } else {
      out_pools[i] = NULL;
    }
  }
}

void add_tile(board_t *board, tile_t *tile) {
  if (!valid_tile(board, tile))
    return;

  pool_t *target_pool = NULL;
  pool_t *candidate_pools[MAX_POOL_CANDIDATES];
  uint32_t compatible_pool_ids[MAX_POOL_CANDIDATES];
  size_t num_compatible_pools = 0;

  // Get neighboring pools that accept this tile type
  get_neighbor_pools(board, tile, candidate_pools, 6);

  for (size_t i = 0; i < 6 && num_compatible_pools < MAX_POOL_CANDIDATES; i++) {
    if (candidate_pools[i] &&
        pool_accepts_tile_type(candidate_pools[i], tile->data.type)) {
      // Check if we already have this pool ID
      bool already_added = false;
      for (size_t j = 0; j < num_compatible_pools; j++) {
        if (compatible_pool_ids[j] == candidate_pools[i]->id) {
          already_added = true;
          break;
        }
      }
      if (!already_added) {
        compatible_pool_ids[num_compatible_pools] = candidate_pools[i]->id;
        num_compatible_pools++;
      }
    }
  }

  if (num_compatible_pools == 0) {
    // Create new pool
    target_pool = pool_map_create_pool(board->pools);
    target_pool->accepted_tile_type = tile->data.type;
    tile->pool_id = target_pool->id;
  } else {
    // Use the first compatible pool (could implement scoring here later)
    pool_map_entry_t *pool_entry =
      pool_map_find_by_id(board->pools, compatible_pool_ids[0]);
    target_pool = pool_entry->pool;
    tile->pool_id = target_pool->id;

    // If multiple pools, merge them into the target pool
    for (size_t i = 1; i < num_compatible_pools; i++) {
      pool_map_entry_t *merge_entry =
        pool_map_find_by_id(board->pools, compatible_pool_ids[i]);
      if (merge_entry && merge_entry->pool) {
        pool_t *merge_pool = merge_entry->pool;

        // Move all tiles from merge_pool to target_pool
        tile_map_entry_t *tile_entry, *tmp;
        HASH_ITER(hh, merge_pool->tiles->root, tile_entry, tmp) {
          tile_entry->tile->pool_id = target_pool->id;
          pool_add_tile_to_pool(target_pool, tile_entry->tile);
        }

        // Remove the merged pool
        pool_map_remove(board->pools, compatible_pool_ids[i]);
      }
    }
  }

  // Add tile to board's tile map and pool
  tile_map_add(board->tiles, tile);
  pool_add_tile_to_pool(target_pool, tile);
  pool_update(target_pool, board->grid);
}

void remove_tile(board_t *board, tile_t *tile) {
  if (!valid_tile(board, tile))
    return;

  // Get the pool this tile belongs to
  pool_map_entry_t *pool_entry =
    pool_map_find_by_id(board->pools, tile->pool_id);
  if (pool_entry) {
    // Remove tile from pool
    pool_remove_tile(pool_entry->pool, tile);
    pool_update(pool_entry->pool, board->grid);
  }

  // Remove tile from board's tile map
  tile_map_remove(board->tiles, tile->cell);
}

void board_randomize(board_t *board) {
  // Clear existing tiles and pools
  clear_board(board);

  grid_cell_t *cells = board->grid->cells;
  shuffle_array(cells, board->grid->num_cells, sizeof(grid_cell_t),
                swap_grid_cell);

  for (size_t i = 0; i < board->grid->num_cells; i++) {
    if (rand() % 4 >= 0) {
      // In randomize_board()
      tile_t *tile = tile_create_random_ptr(cells[i]);
      if (tile->data.type != TILE_EMPTY) { // Only add non-empty tiles
        tile->pool_id = 0;                 // Will be assigned in add_tile
        add_tile(board, tile);
      } else {
        // Free the empty tile since we're not using it
        free(tile);
      }
    }
  }
}

void cycle_tile_type(board_t *board, tile_t *tile);

void print_board_debug_info(board_t *board) {
  if (!board) {
    printf("Board is NULL\n");
    return;
  }

  printf("=== Board Debug Info ===\n");
  printf("Grid type: %d\n", board->grid->type);
  printf("Grid cells: %zu\n", board->grid->num_cells);
  printf("Tiles in board: %d\n", board->tiles->num_tiles);
  printf("Pools in board: %zu\n", board->pools->num_pools);
  printf("Next pool ID: %u\n", board->next_pool_id);
  printf("========================\n");
}

// Function to check if a board merge is valid (no tile overlaps)
bool is_merge_valid(board_t *target_board, board_t *source_board,
                    grid_cell_t target_center, grid_cell_t source_center) {
  if (target_board->grid->type != source_board->grid->type) {
    fprintf(stderr, "Cannot merge boards of different grid types\n");
    return false;
  }

  // Calculate the offset needed to align source_center with target_center
  grid_cell_t offset =
    target_board->grid->vtable->calculate_offset(target_center, source_center);

  // Check each tile in the source board
  for (size_t i = 0; i < source_board->grid->num_cells; i++) {
    grid_cell_t source_cell = source_board->grid->cells[i];
    tile_t *source_tile = get_tile_at_cell(source_board, source_cell);

    if (source_tile) {
      // Apply offset to get the target position
      grid_cell_t target_position =
        target_board->grid->vtable->apply_offset(source_cell, offset);

      // Check if this position is valid in the target grid
      if (!target_board->grid->vtable->is_valid_cell(target_board->grid,
                                                     target_position)) {
        continue; // Skip tiles that would fall outside the target grid
      }

      // Check if there's already a tile at this position in the target board
      tile_t *existing_tile = get_tile_at_cell(target_board, target_position);
      if (existing_tile) {
        return false; // Overlap detected
      }
    }
  }

  return true; // No overlaps found
}

// Function to merge two boards
bool merge_boards(board_t *target_board, board_t *source_board,
                  grid_cell_t target_center, grid_cell_t source_center) {
  if (!is_merge_valid(target_board, source_board, target_center,
                      source_center)) {
    fprintf(stderr, "Board merge is not valid - tiles would overlap\n");
    return false;
  }

  // Calculate the offset needed to align source_center with target_center
  grid_cell_t offset =
    target_board->grid->vtable->calculate_offset(target_center, source_center);

  // Merge each tile from source to target
  for (size_t i = 0; i < source_board->grid->num_cells; i++) {
    grid_cell_t source_cell = source_board->grid->cells[i];
    tile_t *source_tile = get_tile_at_cell(source_board, source_cell);

    if (source_tile) {
      // Apply offset to get the target position
      grid_cell_t target_position =
        target_board->grid->vtable->apply_offset(source_cell, offset);

      // Check if this position is valid in the target grid
      if (!target_board->grid->vtable->is_valid_cell(target_board->grid,
                                                     target_position)) {
        continue; // Skip tiles that would fall outside the target grid
      }

      // Create a new tile at the target position with the same data
      tile_t *new_tile = malloc(sizeof(tile_t));
      if (!new_tile) {
        fprintf(stderr, "Failed to allocate memory for merged tile\n");
        return false;
      }

      new_tile->cell = target_position;
      new_tile->data = source_tile->data; // Copy tile data
      new_tile->pool_id = 0;              // Will be assigned in add_tile

      // Add the tile to the target board
      add_tile(target_board, new_tile);
    }
  }

  return true;
}
