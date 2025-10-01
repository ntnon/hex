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

static void create_center_cluster(board_t *board) {
  grid_cell_t center = grid_get_center_cell(board->geometry_type);

  // Create three clustered tiles of different colors at center
  tile_data_t magenta_data = tile_data_create(TILE_MAGENTA, 1);
  tile_data_t cyan_data = tile_data_create(TILE_CYAN, 1);
  tile_data_t yellow_data = tile_data_create(TILE_YELLOW, 1);

  tile_t *center_tile = tile_create_ptr(center, magenta_data);
  center_tile->pool_id = 0;
  add_tile(board, center_tile);

  // Get the first two neighbors for the other colors
  grid_cell_t neighbors[6];
  board->geometry->get_neighbor_cells(center, neighbors);

  tile_t *neighbor1_tile = tile_create_ptr(neighbors[0], cyan_data);
  neighbor1_tile->pool_id = 0;
  add_tile(board, neighbor1_tile);

  tile_t *neighbor2_tile = tile_create_ptr(neighbors[1], yellow_data);
  neighbor2_tile->pool_id = 0;
  add_tile(board, neighbor2_tile);
}

board_t *board_create(grid_type_e grid_type, int radius,
                      board_type_e board_type) {
  board_t *board = malloc(sizeof(board_t));
  if (!board) {
    fprintf(stderr, "Failed to allocate memory for board\n");
    return NULL;
  }

  // Configure geometry
  board->geometry_type = grid_type;
  board->radius = radius;
  board->board_type = board_type;
  board->layout = default_layout;

  // Set geometry function pointers based on type
  switch (grid_type) {
  case GRID_TYPE_HEXAGON:
    board->geometry = &hex_grid_vtable;
    break;
  // Add other geometry types as they're implemented
  default:
    fprintf(stderr, "Unsupported grid type: %d\n", grid_type);
    free(board);
    return NULL;
  }

  board->tiles = tile_map_create();
  board->pools = pool_map_create();
  board->next_pool_id = 1;

  camera_init(&board->camera);

  if (board_type == BOARD_TYPE_MAIN) {
    create_center_cluster(board);
  }

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

  free(board);
}

bool valid_tile(board_t *board, tile_t *tile) {
  if (!grid_is_valid_cell_with_radius(tile->cell, board->radius)) {
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
  if (!board || growth_amount <= 0) {
    return false;
  }

  board->radius += growth_amount;
  return true;
}

// Function to get neighboring pools that accept a specific tile type
void get_neighbor_pools(board_t *board, tile_t *tile, pool_t **out_pools,
                        size_t max_neighbors) {
  grid_cell_t neighbor_cells[6];

  board->geometry->get_neighbor_cells(tile->cell, neighbor_cells);

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

  // Debug: Print tile data at start of add_tile
  printf("DEBUG add_tile: entering with tile type=%d, value=%d\n",
         tile->data.type, tile->data.value);

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
  // TODO: Fix pool_update to work without grid instance
  // pool_update(target_pool, board->geometry_type, board->layout,
  // board->radius);

  // Mark chunk dirty for rendering updates - DISABLED
  // chunk_id_t chunk_id = grid_get_chunk_id(board->grid, tile->cell);
  // grid_mark_chunk_dirty(board->grid, chunk_id);
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
    // TODO: Fix pool_update to work without grid instance
    // pool_update(pool_entry->pool, board->geometry_type, board->layout,
    //             board->radius);
  }

  // Remove tile from board's tile map
  tile_map_remove(board->tiles, tile->cell);

  // Mark chunk dirty for rendering updates - DISABLED
  // chunk_id_t chunk_id = grid_get_chunk_id(board->grid, tile->cell);
  // grid_mark_chunk_dirty(board->grid, chunk_id);
}

void board_randomize(board_t *board, int radius, board_type_e board_type) {
  // Clamp radius to board's maximum radius
  if (radius > board->radius) {
    radius = board->radius;
  }

  // Clear existing tiles and pools
  clear_board(board);

  // Place three clustered tiles for main boards
  if (board_type == BOARD_TYPE_MAIN) {
    create_center_cluster(board);
  }

  // Get all valid coordinates for this geometry type within the specified
  // radius
  grid_cell_t *all_coords;
  size_t coord_count;
  if (!grid_get_all_coordinates_in_radius(board->geometry_type, radius,
                                          &all_coords, &coord_count)) {
    return;
  }

  // Remove center coordinate from the list if we placed a center tile
  if (board_type == BOARD_TYPE_MAIN) {
    grid_cell_t center = grid_get_center_cell(board->geometry_type);
    size_t filtered_count = 0;
    for (size_t i = 0; i < coord_count; i++) {
      bool is_center = (all_coords[i].coord.hex.q == center.coord.hex.q &&
                        all_coords[i].coord.hex.r == center.coord.hex.r &&
                        all_coords[i].coord.hex.s == center.coord.hex.s);
      if (!is_center) {
        all_coords[filtered_count++] = all_coords[i];
      }
    }
    coord_count = filtered_count;
  }

  // Create a reasonable number of tiles instead of filling entire grid
  size_t target_tiles = radius * 10; // Scale with grid size but keep reasonable
  if (target_tiles > coord_count) {
    target_tiles = coord_count / 1.5; // Don't exceed half the available cells
  }

  // Shuffle the coordinates for random placement
  for (size_t i = coord_count - 1; i > 0; i--) {
    size_t j = rand() % (i + 1);
    grid_cell_t temp = all_coords[i];
    all_coords[i] = all_coords[j];
    all_coords[j] = temp;
  }

  // Place tiles at random coordinates
  size_t created_tiles = 0;
  for (size_t i = 0; i < coord_count && created_tiles < target_tiles; i++) {
    grid_cell_t cell = all_coords[i];
    tile_t *tile = tile_create_random_ptr(cell);
    if (tile->data.type != TILE_EMPTY) {
      tile->pool_id = 0; // Will be assigned in add_tile
      add_tile(board, tile);
      created_tiles++;
    } else {
      free(tile); // Free empty tiles
    }
  }

  free(all_coords);
}

void board_fill(board_t *board, int radius, board_type_e board_type) {
  // Clamp radius to board's maximum radius
  if (radius > board->radius) {
    radius = board->radius;
  }

  // Clear existing tiles and pools
  clear_board(board);

  // Place three clustered tiles for main boards
  if (board_type == BOARD_TYPE_MAIN) {
    create_center_cluster(board);
  }

  // Get all valid coordinates for this geometry type within the specified
  // radius
  grid_cell_t *all_coords;
  size_t coord_count;
  if (!grid_get_all_coordinates_in_radius(board->geometry_type, radius,
                                          &all_coords, &coord_count)) {
    return;
  }

  size_t created_tiles = (board_type == BOARD_TYPE_MAIN)
                           ? 1
                           : 0; // Start with 1 for center tile if main board

  // Fill all valid cells in the grid (except center which we already placed)
  for (size_t i = 0; i < coord_count; i++) {
    grid_cell_t cell = all_coords[i];

    // Skip center coordinate if we placed a center tile
    if (board_type == BOARD_TYPE_MAIN) {
      grid_cell_t center = grid_get_center_cell(board->geometry_type);
      bool is_center = (cell.coord.hex.q == center.coord.hex.q &&
                        cell.coord.hex.r == center.coord.hex.r &&
                        cell.coord.hex.s == center.coord.hex.s);
      if (is_center) {
        continue;
      }
    }

    tile_t *tile = tile_create_random_ptr(cell);
    if (tile->data.type != TILE_EMPTY) {
      tile->pool_id = 0; // Will be assigned in add_tile
      add_tile(board, tile);
      created_tiles++;
    } else {
      free(tile); // Free empty tiles
    }
  }

  free(all_coords);
  printf("Board filled with %zu tiles\n", created_tiles);
}

void cycle_tile_type(board_t *board, tile_t *tile);

void print_board_debug_info(board_t *board) {
  if (!board) {
    printf("Board is NULL\n");
    return;
  }

  printf("=== Board Debug Info ===\n");
  printf("Grid type: %d\n", board->geometry_type);
  // Calculate cell count based on radius (hex grid formula)
  size_t num_cells = 3 * board->radius * (board->radius + 1) + 1;
  printf("Grid cells: %zu (radius: %d)\n", num_cells, board->radius);

  // Display chunk system status
  printf("Chunk system: disabled\n");
  // printf("Chunk system: enabled with %zu active chunks (size %d)\n",
  //        board->grid->chunk_system.num_chunks,
  //        board->grid->chunk_system.chunk_size);

  printf("Tiles in board: %d\n", board->tiles->num_tiles);
  printf("Pools in board: %zu\n", board->pools->num_pools);
  printf("Next pool ID: %u\n", board->next_pool_id);
  printf("========================\n");
}

// Function to check if a board merge is valid (no tile overlaps)
bool is_merge_valid(board_t *target_board, board_t *source_board,
                    grid_cell_t target_center, grid_cell_t source_center) {
  if (target_board->geometry_type != source_board->geometry_type) {
    return false;
  }

  // Calculate the offset needed to align source_center with target_center
  grid_cell_t offset = grid_calculate_offset(target_center, source_center);

  // Check each tile in the source board directly

  tile_map_entry_t *entry, *tmp;
  HASH_ITER(hh, source_board->tiles->root, entry, tmp) {
    grid_cell_t source_cell = entry->tile->cell;
    grid_cell_t target_position = grid_apply_offset(source_cell, offset);

    // Check if this position is valid in the target grid
    if (!grid_is_valid_cell_with_radius(target_position,
                                        target_board->radius)) {
      return false; // Target position out of bounds
    }

    // Check if this position is occupied in the target board
    tile_t *target_tile = get_tile_at_cell(target_board, target_position);
    if (target_tile) {
      return false; // Position already occupied
    }
  }
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
  grid_cell_t offset = grid_calculate_offset(target_center, source_center);

  // Merge each tile from source to target
  int tiles_added = 0;
  tile_map_entry_t *entry, *tmp;
  HASH_ITER(hh, source_board->tiles->root, entry, tmp) {
    tile_t *source_tile = entry->tile;

    // Apply offset to get the target position
    grid_cell_t target_position = grid_apply_offset(source_tile->cell, offset);

    // Check if this position is valid in the target grid
    if (!grid_is_valid_cell_with_radius(target_position,
                                        target_board->radius)) {
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

  // Create a temporary clone to validate rotation before modifying original
  tile_map_t *temp_map = tile_map_clone(board->tiles);
  if (!temp_map) {
    return false;
  }

  // Perform rotation on the clone
  if (!tile_map_rotate(temp_map, center, rotation_steps)) {
    tile_map_free(temp_map);
    return false;
  }

  // Validate all rotated positions are within grid bounds
  if (!board_validate_tile_map_bounds(board, temp_map)) {
    tile_map_free(temp_map);
    return false;
  }

  // Validation passed - apply rotation to original tile map
  bool success = tile_map_rotate(board->tiles, center, rotation_steps);
  tile_map_free(temp_map);

  return success;
}

// Board cloning function
board_t *board_clone(board_t *original) {
  if (!original)
    return NULL;

  board_t *clone = malloc(sizeof(board_t));
  if (!clone)
    return NULL;

  // Copy geometry configuration
  clone->geometry_type = original->geometry_type;
  clone->geometry = original->geometry;
  clone->layout = original->layout;
  clone->radius = original->radius;
  clone->board_type = original->board_type;

  // Create new tile and pool maps
  clone->tiles = tile_map_create();
  clone->pools = pool_map_create();
  clone->next_pool_id = original->next_pool_id;

  // Copy camera
  clone->camera = original->camera;

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

bool board_validate_tile_map_bounds(const board_t *board,
                                    const tile_map_t *tile_map) {
  if (!board || !tile_map)
    return false;

  if (tile_map->num_tiles == 0)
    return true; // Empty map is valid

  // Validate each tile is within board's spatial bounds
  tile_map_entry_t *entry, *tmp;
  HASH_ITER(hh, tile_map->root, entry, tmp) {
    if (!grid_is_valid_cell_with_radius(entry->tile->cell, board->radius)) {
      return false;
    }
  }

  return true;
}
