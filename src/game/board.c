#include "game/board.h"
#include "game/camera.h"
#include "third_party/uthash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_POOL_CANDIDATES 10

const orientation_t layout_pointy_t = {.f0 = 1.732050808,
                                       .f1 = 0.866025404,
                                       .f2 = 0.0,
                                       .f3 = 1.5,
                                       .b0 = 0.577350269,
                                       .b1 = -0.333333333,
                                       .b2 = 0.0,
                                       .b3 = 0.666666667,
                                       .start_angle = -0.5};

layout_t default_layout = {
  .orientation = layout_pointy_t,
  .size = {10.0, 10.0}, // Hex size (adjust as needed)
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

  board->edge_list = edge_render_list_create();
  if (!board->edge_list) {
    tile_map_free(board->tiles);
    pool_map_free(board->pools);
    free(board);
    return NULL;
  }

  camera_init(&board->camera);

  if (board_type == BOARD_TYPE_MAIN) {
    create_center_cluster(board);
  }

  return board;
}

void clear_board(board_t *board) {
  tile_map_free(board->tiles);
  pool_map_free(board->pools);
  edge_render_list_clear(board->edge_list);
  board->tiles = tile_map_create();
  board->pools = pool_map_create();
  board->next_pool_id = 1;
}

void free_board(board_t *board) {
  tile_map_free(board->tiles);
  pool_map_free(board->pools);
  edge_render_list_free(board->edge_list);

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
tile_t *get_tile_at_cell(const board_t *board, grid_cell_t cell) {
  tile_map_entry_t *entry = tile_map_find(board->tiles, cell);
  return entry ? entry->tile : NULL;
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

  printf("DEBUG: Adding tile type %d at (%d,%d,%d)\n", tile->data.type,
         tile->cell.coord.hex.q, tile->cell.coord.hex.r,
         tile->cell.coord.hex.s);

  pool_t *target_pool = NULL;
  uint32_t compatible_pool_ids[MAX_POOL_CANDIDATES];
  size_t num_compatible_pools = 0;

  // Look for neighboring tiles of the same color/type
  grid_cell_t neighbor_cells[6];
  board->geometry->get_neighbor_cells(tile->cell, neighbor_cells);

  for (size_t i = 0; i < 6 && num_compatible_pools < MAX_POOL_CANDIDATES; i++) {
    tile_t *neighbor_tile = get_tile_at_cell(board, neighbor_cells[i]);

    // Only consider neighbors of the same color
    if (neighbor_tile && neighbor_tile->data.type == tile->data.type) {
      printf("DEBUG: Found same-color neighbor type %d with pool_id %d\n",
             neighbor_tile->data.type, neighbor_tile->pool_id);
      pool_t *neighbor_pool =
        pool_map_get_pool(board->pools, neighbor_tile->pool_id);

      if (neighbor_pool) {
        // Check if we already have this pool ID
        bool already_added = false;
        for (size_t j = 0; j < num_compatible_pools; j++) {
          if (compatible_pool_ids[j] == neighbor_pool->id) {
            already_added = true;
            break;
          }
        }
        if (!already_added) {
          compatible_pool_ids[num_compatible_pools] = neighbor_pool->id;
          num_compatible_pools++;
          printf("DEBUG: Added pool_id %d to compatible list\n",
                 neighbor_pool->id);
        }
      }
    }
  }

  if (num_compatible_pools == 0) {
    // Create new pool
    printf("DEBUG: No compatible pools found, creating new pool\n");
    target_pool = pool_map_create_pool(board->pools);
    target_pool->accepted_tile_type = tile->data.type;
    tile->pool_id = target_pool->id;
    printf("DEBUG: Created new pool with ID %d for tile type %d\n",
           target_pool->id, tile->data.type);
  } else {
    // Use the first compatible pool (could implement scoring here later)
    printf("DEBUG: Found %zu compatible pools, using pool_id %d\n",
           num_compatible_pools, compatible_pool_ids[0]);
    target_pool = pool_map_get_pool(board->pools, compatible_pool_ids[0]);
    tile->pool_id = target_pool->id;
    printf("DEBUG: Assigned tile to existing pool_id %d\n", target_pool->id);

    // If multiple pools, merge them into the target pool
    for (size_t i = 1; i < num_compatible_pools; i++) {
      pool_t *merge_pool =
        pool_map_get_pool(board->pools, compatible_pool_ids[i]);
      if (merge_pool) {

        // Move all tiles from merge_pool to target_pool
        tile_map_entry_t *tile_entry, *tmp;
        HASH_ITER(hh, merge_pool->tiles->root, tile_entry, tmp) {
          tile_entry->tile->pool_id = target_pool->id;
          pool_add_tile_to_pool(target_pool, tile_entry->tile,
                                board->geometry_type);
        }

        // Remove the merged pool
        pool_map_remove(board->pools, compatible_pool_ids[i]);
      }
    }
  }

  // Add tile to board's tile map and pool
  tile_map_add(board->tiles, tile);
  pool_add_tile_to_pool(target_pool, tile, board->geometry_type);
  // TODO: Fix pool_update to work without grid instance
  // pool_update(target_pool, board->geometry_type, board->layout,
  //             board->radius);

  // Note: Edge list update moved to batch operations for performance
  // board_update_edge_list(board);

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

  // Update edge list after removing tile
  board_update_edge_list(board);

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

  // Update edge list after all tiles added
  board_update_edge_list(board);
}

void board_fill(board_t *board, int radius, board_type_e board_type) {
  // Use batched version for better performance
  board_fill_batch(board, radius, board_type);
}

void board_fill_fast(board_t *board, int radius, board_type_e board_type) {
  clock_t start_time = clock();
  printf("Starting board_fill_fast with radius %d\n", radius);

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
  clock_t coords_start = clock();
  grid_cell_t *all_coords;
  size_t coord_count;
  if (!grid_get_all_coordinates_in_radius(board->geometry_type, radius,
                                          &all_coords, &coord_count)) {
    return;
  }
  printf("Generated %zu coordinates in %.3fs\n", coord_count,
         (double)(clock() - coords_start) / CLOCKS_PER_SEC);

  // Pre-allocate array for tiles
  tile_t **tiles = malloc(coord_count * sizeof(tile_t *));
  size_t tile_count = 0;

  size_t created_tiles = (board_type == BOARD_TYPE_MAIN)
                           ? 1
                           : 0; // Start with 1 for center tile if main board

  // Generate all tile data first (no pool assignment yet)
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
      tile->pool_id = 1; // Assign all tiles to single pool for fast mode
      tiles[tile_count++] = tile;
      created_tiles++;
    } else {
      free(tile); // Free empty tiles
    }
  }

  clock_t batch_start = clock();
  // Add all tiles to board at once (no pool logic)
  add_tiles_batch(board, tiles, tile_count);
  printf("Added %zu tiles in %.3fs\n", tile_count,
         (double)(clock() - batch_start) / CLOCKS_PER_SEC);

  // Create single pool for all tiles (fast mode)
  clock_t pools_start = clock();
  pool_t *single_pool = pool_map_create_pool(board->pools);
  single_pool->accepted_tile_type = TILE_MAGENTA; // Accept any type
  // Don't overwrite ID - pool_map_create_pool already assigns unique ID

  // Add all tiles to single pool
  for (size_t i = 0; i < tile_count; i++) {
    pool_add_tile_to_pool(single_pool, tiles[i], board->geometry_type);
  }
  printf("Assigned single pool in %.3fs\n",
         (double)(clock() - pools_start) / CLOCKS_PER_SEC);

  // Skip edge calculation for fast mode
  printf("Skipping edge calculation for fast mode\n");

  free(tiles);
  free(all_coords);
  printf("Board filled with %zu tiles in %.3fs total (FAST MODE)\n",
         created_tiles, (double)(clock() - start_time) / CLOCKS_PER_SEC);
}

void board_fill_batch(board_t *board, int radius, board_type_e board_type) {
  clock_t start_time = clock();
  printf("Starting board_fill_batch with radius %d\n", radius);

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
  clock_t coords_start = clock();
  grid_cell_t *all_coords;
  size_t coord_count;
  if (!grid_get_all_coordinates_in_radius(board->geometry_type, radius,
                                          &all_coords, &coord_count)) {
    return;
  }
  printf("Generated %zu coordinates in %.3fs\n", coord_count,
         (double)(clock() - coords_start) / CLOCKS_PER_SEC);

  // Pre-allocate array for tiles
  tile_t **tiles = malloc(coord_count * sizeof(tile_t *));
  size_t tile_count = 0;

  size_t created_tiles = (board_type == BOARD_TYPE_MAIN)
                           ? 1
                           : 0; // Start with 1 for center tile if main board

  // Generate all tile data first (no pool assignment yet)
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
      tile->pool_id = 0; // Will be assigned later
      tiles[tile_count++] = tile;
      created_tiles++;
    } else {
      free(tile); // Free empty tiles
    }
  }

  clock_t batch_start = clock();
  // Add all tiles to board at once (no pool logic)
  add_tiles_batch(board, tiles, tile_count);
  printf("Added %zu tiles in %.3fs\n", tile_count,
         (double)(clock() - batch_start) / CLOCKS_PER_SEC);

  clock_t pools_start = clock();
  // Assign pools efficiently after all tiles exist
  assign_pools_batch(board);
  printf("Assigned pools in %.3fs\n",
         (double)(clock() - pools_start) / CLOCKS_PER_SEC);

  clock_t edges_start = clock();
  // Update edge list once at the end
  printf("Starting edge calculation for %zu tiles...\n", tile_count);
  board_update_edge_list(board);
  printf("Updated edges in %.3fs\n",
         (double)(clock() - edges_start) / CLOCKS_PER_SEC);

  free(tiles);
  free(all_coords);
  printf("Board filled with %zu tiles in %.3fs total\n", created_tiles,
         (double)(clock() - start_time) / CLOCKS_PER_SEC);
}

void add_tiles_batch(board_t *board, tile_t **tiles, size_t count) {
  if (!board || !tiles || count == 0)
    return;

  // Add all tiles to the board's tile map without pool assignment
  for (size_t i = 0; i < count; i++) {
    if (tiles[i] && valid_tile(board, tiles[i])) {
      tile_map_add_unchecked(board->tiles, tiles[i]);
    }
  }
}

void assign_pools_batch(board_t *board) {
  if (!board || !board->tiles)
    return;

  // Use flood-fill algorithm to find connected components of same-colored tiles
  // This is much more efficient than checking neighbors for each tile
  // individually

  tile_map_entry_t *entry, *tmp;

  // First pass: mark all tiles as unassigned
  HASH_ITER(hh, board->tiles->root, entry, tmp) {
    entry->tile->pool_id = 0; // 0 means unassigned
  }

  // next_pool_id removed - pool_map manages IDs automatically

  // Second pass: flood-fill to assign pools
  printf("Starting pool assignment for %zu tiles...\n",
         (size_t)tile_map_size(board->tiles));
  size_t pools_created = 0;
  HASH_ITER(hh, board->tiles->root, entry, tmp) {
    tile_t *tile = entry->tile;

    if (tile->pool_id == 0) { // Unassigned
      // Create new pool and flood-fill
      pool_t *new_pool = pool_map_create_pool(board->pools);
      new_pool->accepted_tile_type = tile->data.type;
      // Don't overwrite ID - pool_map_create_pool already assigns unique ID

      // Flood-fill all connected tiles of same type
      flood_fill_assign_pool(board, tile, new_pool);
      pools_created++;

      if (pools_created % 10 == 0) {
        printf("Created %zu pools so far...\n", pools_created);
      }
    }
  }
  printf("Created %zu pools total\n", pools_created);
}

void flood_fill_assign_pool(board_t *board, tile_t *start_tile, pool_t *pool) {
  if (!board || !start_tile || !pool || start_tile->pool_id != 0)
    return;

  // Use iterative flood-fill with stack to prevent stack overflow on large
  // boards
  tile_t **stack =
    malloc(100000 * sizeof(tile_t *)); // Start with larger size for big boards
  size_t stack_size = 0;
  size_t stack_capacity = 100000;

  // Add start tile to stack
  stack[stack_size++] = start_tile;

  while (stack_size > 0) {
    // Pop tile from stack
    tile_t *current_tile = stack[--stack_size];

    // Skip if already processed
    if (current_tile->pool_id != 0)
      continue;

    // Assign to pool
    current_tile->pool_id = pool->id;
    pool_add_tile_to_pool(pool, current_tile, board->geometry_type);

    // Check all 6 neighbors
    for (int dir = 0; dir < 6; dir++) {
      grid_cell_t neighbor_cell;
      board->geometry->get_neighbor_cell(current_tile->cell, dir,
                                         &neighbor_cell);
      tile_t *neighbor = get_tile_at_cell(board, neighbor_cell);

      if (neighbor && neighbor->pool_id == 0 &&             // Unassigned
          neighbor->data.type == current_tile->data.type) { // Same type

        // Expand stack if needed (use larger increments)
        if (stack_size >= stack_capacity) {
          size_t new_capacity = stack_capacity + 50000; // Add 50k at a time
          tile_t **new_stack = realloc(stack, new_capacity * sizeof(tile_t *));
          if (!new_stack) {
            printf("ERROR: Failed to expand flood-fill stack\n");
            free(stack);
            return;
          }
          stack = new_stack;
          stack_capacity = new_capacity;
        }

        // Add neighbor to stack
        stack[stack_size++] = neighbor;
      }
    }
  }

  free(stack);
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

// Get neighbor cell in a specific hex direction

// Check if this tile should draw the edge in the given direction
// Uses deterministic ownership to prevent duplicate edge drawing
static bool should_draw_edge(const board_t *board, grid_cell_t cell,
                             hex_edge_direction_t edge_dir) {
  if (!board)
    return false;

  // Get the tile at current cell
  tile_t *current_tile = get_tile_at_cell(board, cell);
  if (!current_tile)
    return false; // No tile here, no edges to draw

  // Get neighbor cell using geometry-native function
  grid_cell_t neighbor = hex_get_edge_neighbor(cell, edge_dir);

  // Check if neighbor is out of bounds
  if (!grid_is_valid_cell_with_radius(neighbor, board->radius)) {
    return true; // Always draw boundary edges
  }

  // Get tile at neighbor
  tile_t *neighbor_tile = get_tile_at_cell(board, neighbor);

  // Only draw edge if there's a pool boundary
  if (!neighbor_tile) {
    return true; // Edge to empty space (pool boundary)
  }

  if (current_tile->pool_id == neighbor_tile->pool_id) {
    return false; // Same pool, no edge needed
  }

  // Different pools - use deterministic ownership to prevent duplicates
  // Only draw the edge if this tile "owns" it based on consistent criteria

  // Compare coordinates to determine ownership
  hex_coord_t curr_hex = cell.coord.hex;
  hex_coord_t neighbor_hex = neighbor.coord.hex;

  // Primary criterion: tile with smaller q coordinate owns the edge
  if (curr_hex.q < neighbor_hex.q) {
    return true;
  } else if (curr_hex.q > neighbor_hex.q) {
    return false;
  }

  // If q is equal, use r coordinate as tiebreaker
  if (curr_hex.r < neighbor_hex.r) {
    return true;
  } else if (curr_hex.r > neighbor_hex.r) {
    return false;
  }

  // If both q and r are equal, use s coordinate (shouldn't happen for valid
  // neighbors)
  return curr_hex.s < neighbor_hex.s;
}

void board_update_edge_list(board_t *board) {
  if (!board || !board->edge_list)
    return;

  // Clear existing edges
  edge_render_list_clear(board->edge_list);

  // Default edge appearance
  Color edge_color = (Color){0, 0, 0, 55}; // Black
  float edge_thickness = 0.5f;
  float vertex_radius = 0.25f;

  // Progress tracking for large boards
  size_t total_tiles = tile_map_size(board->tiles);
  size_t processed = 0;
  size_t progress_interval = total_tiles / 20; // Report every 5%
  if (progress_interval == 0)
    progress_interval = 1000;

  // Iterate through all tiles
  tile_map_entry_t *entry, *tmp;
  HASH_ITER(hh, board->tiles->root, entry, tmp) {
    tile_t *tile = entry->tile;

    // Check each hex edge direction using geometry-native approach
    for (int dir = 0; dir < 6; dir++) {
      hex_edge_direction_t edge_dir = (hex_edge_direction_t)dir;
      if (should_draw_edge(board, tile->cell, edge_dir)) {
        // Get edge endpoints directly from geometry
        point_t start_pt =
          hex_get_edge_start(&board->layout, tile->cell, edge_dir);
        point_t end_pt = hex_get_edge_end(&board->layout, tile->cell, edge_dir);

        Vector2 start = {start_pt.x, start_pt.y};
        Vector2 end = {end_pt.x, end_pt.y};

        edge_render_list_add_edge(board->edge_list, start, end, edge_color,
                                  edge_thickness);

        // Add corner circles (vertices) using geometry-native vertex positions
        edge_render_list_add_vertex(board->edge_list, start, edge_color,
                                    vertex_radius);
        edge_render_list_add_vertex(board->edge_list, end, edge_color,
                                    vertex_radius);
      }
    }

    processed++;
    if (processed % progress_interval == 0) {
      printf("Edge calculation progress: %zu/%zu tiles (%.1f%%)\n", processed,
             total_tiles, (100.0 * processed) / total_tiles);
    }
  }
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
