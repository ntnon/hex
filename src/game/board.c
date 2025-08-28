#include "game/board.h"
#include "game/camera.h"
#include "third_party/uthash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
  .size = {5.0, 5.0},   // Hex size (adjust as needed)
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
  board->hovered_grid_cell = NULL;

  // Initialize preview system
  board_init_preview_system(board,
                            10); // Start with capacity for 10 preview tiles

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
  board_free_preview_system(board);
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

bool board_grow(board_t *board, int growth_amount) {
  if (!board || !board->grid) {
    return false;
  }

  return grid_grow(board->grid, growth_amount);
}

// Function to get neighboring pools that accept a specific tile type
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

  // Generate coordinates using grid system
  grid_cell_t *cells;
  size_t num_cells;
  grid_get_all_cells(board->grid, &cells, &num_cells);
  shuffle_array(cells, num_cells, sizeof(grid_cell_t), swap_grid_cell);

  for (size_t i = 0; i < num_cells; i++) {
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

  free(cells);
}

void cycle_tile_type(board_t *board, tile_t *tile);

void print_board_debug_info(board_t *board) {
  if (!board) {
    printf("Board is NULL\n");
    return;
  }

  printf("=== Board Debug Info ===\n");
  printf("Grid type: %d\n", board->grid->type);
  // Get actual cell count from grid system
  grid_cell_t *temp_cells;
  size_t num_cells;
  grid_get_all_cells(board->grid, &temp_cells, &num_cells);
  free(temp_cells);
  printf("Grid cells: %zu (radius: %d, initial: %d, grown: %d)\n", num_cells,
         board->grid->radius, board->grid->initial_radius,
         board->grid->total_growth);
  printf("Tiles in board: %d\n", board->tiles->num_tiles);
  printf("Pools in board: %zu\n", board->pools->num_pools);
  printf("Next pool ID: %u\n", board->next_pool_id);
  printf("========================\n");
}

// Function to check if a board merge is valid (no tile overlaps)
bool is_merge_valid(board_t *target_board, board_t *source_board,
                    grid_cell_t target_center, grid_cell_t source_center) {
  if (target_board->grid->type != source_board->grid->type) {
    return false;
  }

  // Calculate the offset needed to align source_center with target_center
  grid_cell_t offset =
    target_board->grid->vtable->calculate_offset(target_center, source_center);

  // Generate source coordinates using grid system
  grid_cell_t *source_cells;
  size_t num_source_cells;
  grid_get_all_cells(source_board->grid, &source_cells, &num_source_cells);

  // Check each tile in the source board
  for (size_t i = 0; i < num_source_cells; i++) {
    grid_cell_t source_cell = source_cells[i];
    tile_t *source_tile = get_tile_at_cell(source_board, source_cell);

    if (source_tile) {
      // Apply offset to get the target position
      grid_cell_t target_position =
        target_board->grid->vtable->apply_offset(source_cell, offset);

      // Check if this position is valid in the target grid
      if (!target_board->grid->vtable->is_valid_cell(target_board->grid,
                                                     target_position)) {
        return false; // Target position out of bounds
      }

      // Check if there's already a tile at this position in the target board
      tile_t *existing_tile = get_tile_at_cell(target_board, target_position);
      if (existing_tile) {
        free(source_cells);
        return false; // Overlap detected
      }
    }
  }
  free(source_cells);
  return true; // No overlaps found
}

// Function to merge two boards
bool merge_boards(board_t *target_board, board_t *source_board,
                  grid_cell_t target_center, grid_cell_t source_center) {
  if (!is_merge_valid(target_board, source_board, target_center,
                      source_center)) {
    return false;
  }

  // Calculate the offset needed to align source_center with target_center
  grid_cell_t offset =
    target_board->grid->vtable->calculate_offset(target_center, source_center);

  // Generate source coordinates using grid system
  grid_cell_t *source_cells;
  size_t num_source_cells;
  grid_get_all_cells(source_board->grid, &source_cells, &num_source_cells);

  // Merge each tile from source to target
  int tiles_added = 0;
  for (size_t i = 0; i < num_source_cells; i++) {
    grid_cell_t source_cell = source_cells[i];
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
      tiles_added++;
    }
  }

  free(source_cells);
  return true;
}

// Board rotation function - rotates all tiles around a center point
// rotation_steps: number of 60-degree steps (1-5, positive = clockwise)
bool board_rotate(board_t *board, grid_cell_t center, int rotation_steps) {
  if (!board || !board->tiles) {
    return false;
  }

  // Normalize rotation steps to 0-5 range
  rotation_steps = ((rotation_steps % 6) + 6) % 6;
  if (rotation_steps == 0) {
    return true; // No rotation needed
  }

  // Create array to store new tile positions
  tile_t **tiles_to_update = malloc(board->tiles->num_tiles * sizeof(tile_t *));
  grid_cell_t *new_positions =
    malloc(board->tiles->num_tiles * sizeof(grid_cell_t));

  if (!tiles_to_update || !new_positions) {
    free(tiles_to_update);
    free(new_positions);
    return false;
  }

  // Collect tiles and calculate new positions
  int tile_count = 0;
  tile_map_entry_t *entry, *tmp;
  HASH_ITER(hh, board->tiles->root, entry, tmp) {
    tile_t *tile = entry->tile;

    // Convert to coordinates relative to center
    int rel_q = tile->cell.coord.hex.q - center.coord.hex.q;
    int rel_r = tile->cell.coord.hex.r - center.coord.hex.r;
    int rel_s = tile->cell.coord.hex.s - center.coord.hex.s;

    // Apply rotation transformation (repeated rotation_steps times)
    for (int step = 0; step < rotation_steps; step++) {
      // 60-degree clockwise rotation: (q,r,s) -> (-r,-s,-q)
      int temp_q = -rel_r;
      int temp_r = -rel_s;
      int temp_s = -rel_q;

      rel_q = temp_q;
      rel_r = temp_r;
      rel_s = temp_s;
    }

    // Convert back to absolute coordinates
    grid_cell_t new_pos = {.type = GRID_TYPE_HEXAGON,
                           .coord.hex = {.q = rel_q + center.coord.hex.q,
                                         .r = rel_r + center.coord.hex.r,
                                         .s = rel_s + center.coord.hex.s}};

    // Check if new position is valid in the grid
    if (!board->grid->vtable->is_valid_cell(board->grid, new_pos)) {

      free(tiles_to_update);
      free(new_positions);
      return false;
    }

    // Check for conflicts with other tiles at new positions
    for (int i = 0; i < tile_count; i++) {
      if (new_positions[i].coord.hex.q == new_pos.coord.hex.q &&
          new_positions[i].coord.hex.r == new_pos.coord.hex.r) {

        free(tiles_to_update);
        free(new_positions);
        return false;
      }
    }

    tiles_to_update[tile_count] = tile;
    new_positions[tile_count] = new_pos;
    tile_count++;
  }

  // Remove all tiles from their current positions
  for (int i = 0; i < tile_count; i++) {
    tile_map_remove(board->tiles, tiles_to_update[i]->cell);
  }

  // Update tile positions and re-add them
  for (int i = 0; i < tile_count; i++) {
    tiles_to_update[i]->cell = new_positions[i];
    tile_map_add(board->tiles, tiles_to_update[i]);
  }

  free(tiles_to_update);
  free(new_positions);

  return true;
}

// Board cloning function
board_t *board_clone(board_t *original) {
  if (!original)
    return NULL;

  board_t *clone = malloc(sizeof(board_t));
  if (!clone)
    return NULL;

  // Create new grid with same parameters - need to calculate radius from
  // original For now, copy the grid structure manually
  clone->grid = malloc(sizeof(grid_t));
  if (!clone->grid) {
    free(clone);
    return NULL;
  }

  // Copy grid structure
  clone->grid->type = original->grid->type;
  clone->grid->layout = original->grid->layout;
  clone->grid->vtable = original->grid->vtable;
  clone->grid->radius = original->grid->radius;
  clone->grid->initial_radius = original->grid->initial_radius;
  clone->grid->total_growth = original->grid->total_growth;

  // Create new tile and pool maps
  clone->tiles = tile_map_create();
  clone->pools = pool_map_create();
  clone->next_pool_id = original->next_pool_id;

  // Copy camera
  clone->camera = original->camera;

  // Initialize hovered_grid_cell
  clone->hovered_grid_cell = NULL;

  // Initialize empty preview system
  clone->preview_boards = NULL;
  clone->num_preview_boards = 0;
  clone->preview_capacity = 0;

  // Clone all tiles
  tile_map_entry_t *entry, *tmp;
  HASH_ITER(hh, original->tiles->root, entry, tmp) {
    tile_t *original_tile = entry->tile;
    tile_t *cloned_tile = malloc(sizeof(tile_t));
    if (!cloned_tile) {
      free_board(clone);
      return NULL;
    }

    *cloned_tile = *original_tile; // Copy tile data
    tile_map_add(clone->tiles, cloned_tile);
  }

  return clone;
}

// Simplified preview system implementation
void board_init_preview_system(board_t *board, size_t initial_capacity) {
  if (!board)
    return;

  board->preview_boards = malloc(initial_capacity * sizeof(board_preview_t));
  board->num_preview_boards = 0;
  board->preview_capacity = initial_capacity;
}

void board_free_preview_system(board_t *board) {
  if (!board || !board->preview_boards)
    return;

  // Free any existing previews
  for (size_t i = 0; i < board->num_preview_boards; i++) {
    board_preview_t *preview = &board->preview_boards[i];

    // Free the contents but not the struct itself (it's part of the array)
    if (preview->merged_board) {
      free_board(preview->merged_board);
      preview->merged_board = NULL;
    }

    if (preview->conflict_positions) {
      free(preview->conflict_positions);
      preview->conflict_positions = NULL;
    }
  }

  free(board->preview_boards);
  board->preview_boards = NULL;
  board->num_preview_boards = 0;
  board->preview_capacity = 0;
}

board_preview_t *board_create_merge_preview(board_t *target_board,
                                            board_t *source_board,
                                            grid_cell_t target_position,
                                            grid_cell_t source_center) {
  if (!target_board || !source_board)
    return NULL;

  board_preview_t *preview = malloc(sizeof(board_preview_t));
  if (!preview)
    return NULL;

  // Clone the target board
  preview->merged_board = board_clone(target_board);
  if (!preview->merged_board) {
    free(preview);
    return NULL;
  }

  // Initialize conflict tracking
  preview->conflict_positions = NULL;
  preview->num_conflicts = 0;
  preview->is_valid_merge = false;

  // Calculate the offset needed for positioning
  grid_cell_t offset = target_board->grid->vtable->calculate_offset(
    target_position, source_center);

  // Check if merge is valid and collect conflicts
  if (is_merge_valid(target_board, source_board, target_position,
                     source_center)) {
    // Valid merge - apply it to the preview board
    merge_boards(preview->merged_board, source_board, target_position,
                 source_center);
    preview->is_valid_merge = true;
  } else {
    // Invalid merge - collect conflict positions
    // Count conflicts first
    size_t conflict_count = 0;
    tile_map_entry_t *entry, *tmp;
    HASH_ITER(hh, source_board->tiles->root, entry, tmp) {
      tile_t *source_tile = entry->tile;
      grid_cell_t target_pos =
        target_board->grid->vtable->apply_offset(source_tile->cell, offset);

      // Check if position would conflict
      if (!target_board->grid->vtable->is_valid_cell(target_board->grid,
                                                     target_pos) ||
          get_tile_at_cell(target_board, target_pos) != NULL) {
        conflict_count++;
      }
    }

    // Allocate and fill conflict positions
    if (conflict_count > 0) {
      preview->conflict_positions =
        malloc(conflict_count * sizeof(grid_cell_t));
      if (preview->conflict_positions) {
        preview->num_conflicts = conflict_count;
        size_t conflict_index = 0;

        HASH_ITER(hh, source_board->tiles->root, entry, tmp) {
          tile_t *source_tile = entry->tile;
          grid_cell_t target_pos =
            target_board->grid->vtable->apply_offset(source_tile->cell, offset);

          if (!target_board->grid->vtable->is_valid_cell(target_board->grid,
                                                         target_pos) ||
              get_tile_at_cell(target_board, target_pos) != NULL) {
            preview->conflict_positions[conflict_index++] = target_pos;
          }
        }
      }
    }
  }

  return preview;
}

void board_free_preview(board_preview_t *preview) {
  if (!preview)
    return;

  if (preview->merged_board) {
    free_board(preview->merged_board);
  }

  if (preview->conflict_positions) {
    free(preview->conflict_positions);
  }

  free(preview);
}

void board_clear_preview_boards(board_t *board) {
  if (!board)
    return;

  // Free existing previews
  for (size_t i = 0; i < board->num_preview_boards; i++) {
    board_preview_t *preview = &board->preview_boards[i];

    // Free the contents but not the struct itself (it's part of the array)
    if (preview->merged_board) {
      free_board(preview->merged_board);
      preview->merged_board = NULL;
    }

    if (preview->conflict_positions) {
      free(preview->conflict_positions);
      preview->conflict_positions = NULL;
    }
  }

  board->num_preview_boards = 0;
}

void board_update_preview(board_t *board, board_t *source_board,
                          grid_cell_t mouse_position) {
  if (!board || !source_board)
    return;

  // Clear existing previews
  board_clear_preview_boards(board);

  // Get the center of the source board
  grid_cell_t source_center = grid_get_center_cell(source_board->grid);

  // Resize preview array if needed
  if (board->num_preview_boards >= board->preview_capacity) {
    board->preview_capacity =
      board->preview_capacity > 0 ? board->preview_capacity * 2 : 1;
    board->preview_boards = realloc(
      board->preview_boards, board->preview_capacity * sizeof(board_preview_t));
    if (!board->preview_boards) {
      fprintf(stderr, "Failed to reallocate preview boards array\n");
      return;
    }
  }

  // Get pointer to the preview slot in the array
  board_preview_t *preview = &board->preview_boards[board->num_preview_boards];

  // Clone the target board
  preview->merged_board = board_clone(board);
  if (!preview->merged_board) {
    return;
  }

  // Initialize conflict tracking
  preview->conflict_positions = NULL;
  preview->num_conflicts = 0;
  preview->is_valid_merge = false;

  // Calculate offset for the preview
  grid_cell_t offset =
    board->grid->vtable->calculate_offset(mouse_position, source_center);

  // Always create the merged board for preview, adding only valid tiles
  size_t valid_tiles = 0;
  size_t conflict_count = 0;
  tile_map_entry_t *entry, *tmp;

  // First pass: count valid tiles and conflicts
  HASH_ITER(hh, source_board->tiles->root, entry, tmp) {
    tile_t *source_tile = entry->tile;
    grid_cell_t target_pos =
      board->grid->vtable->apply_offset(source_tile->cell, offset);

    bool is_valid_cell =
      board->grid->vtable->is_valid_cell(board->grid, target_pos);
    tile_t *existing_tile = get_tile_at_cell(board, target_pos);

    if (is_valid_cell && existing_tile == NULL) {
      valid_tiles++;
    } else {
      conflict_count++;
    }
  }

  printf("DEBUG: Found %zu valid tiles, %zu conflicts\n", valid_tiles,
         conflict_count);

  // Second pass: add valid tiles to merged board and collect conflicts
  HASH_ITER(hh, source_board->tiles->root, entry, tmp) {
    tile_t *source_tile = entry->tile;
    grid_cell_t target_pos =
      board->grid->vtable->apply_offset(source_tile->cell, offset);

    bool is_valid_cell =
      board->grid->vtable->is_valid_cell(board->grid, target_pos);
    tile_t *existing_tile = get_tile_at_cell(board, target_pos);

    if (is_valid_cell && existing_tile == NULL) {
      // Valid tile - add to merged board
      tile_t *new_tile = malloc(sizeof(tile_t));
      if (new_tile) {
        *new_tile = *source_tile;    // Copy tile data
        new_tile->cell = target_pos; // Update position
        tile_map_add(preview->merged_board->tiles, new_tile);
      }
    }
  }

  // Allocate and fill conflict positions
  if (conflict_count > 0) {
    preview->conflict_positions = malloc(conflict_count * sizeof(grid_cell_t));
    if (preview->conflict_positions) {
      preview->num_conflicts = conflict_count;
      size_t conflict_index = 0;

      HASH_ITER(hh, source_board->tiles->root, entry, tmp) {
        tile_t *source_tile = entry->tile;
        grid_cell_t target_pos =
          board->grid->vtable->apply_offset(source_tile->cell, offset);

        bool is_valid_cell =
          board->grid->vtable->is_valid_cell(board->grid, target_pos);
        tile_t *existing_tile = get_tile_at_cell(board, target_pos);

        if (!is_valid_cell || existing_tile != NULL) {
          preview->conflict_positions[conflict_index++] = target_pos;
          if (conflict_index <= 3) { // Only show first 3 conflicts
            printf("DEBUG: CONFLICT #%zu: (%d,%d) -> (%d,%d) %s%s\n",
                   conflict_index, source_tile->cell.coord.hex.q,
                   source_tile->cell.coord.hex.r, target_pos.coord.hex.q,
                   target_pos.coord.hex.r,
                   !is_valid_cell ? "OUT_OF_BOUNDS " : "",
                   existing_tile ? "OCCUPIED" : "");
          }
        }
      }
    }
  }

  // Set validity based on whether we have any valid tiles to show
  preview->is_valid_merge = (valid_tiles > 0);

  board->num_preview_boards++;
}
