#include "../include/game/board.h"
#include "../include/grid/grid_types.h"
#include "../include/tile/tile.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

void test_singleton_tile() {
  printf("Testing singleton tile logic...\n");

  board_t *board = board_create(GRID_TYPE_HEXAGON, 5, BOARD_TYPE_MAIN);

  // Create a single isolated tile
  grid_cell_t center = grid_get_center_cell(GRID_TYPE_HEXAGON);
  tile_data_t magenta_data = tile_data_create(TILE_MAGENTA, 1);
  tile_t *single_tile = tile_create_ptr(center, magenta_data);

  add_tile(board, single_tile);

  // Singleton tile should have pool_id = 0 and no pools created
  assert(single_tile->pool_id == 0);
  assert(board->pools->num_pools == 0);

  printf("✓ Singleton tile test passed\n");
  free(board);
}

void test_adjacent_tiles_create_pool() {
  printf("Testing adjacent tiles pool creation...\n");

  board_t *board = board_create(GRID_TYPE_HEXAGON, 5, BOARD_TYPE_MAIN);

  // Create first tile
  grid_cell_t center = grid_get_center_cell(GRID_TYPE_HEXAGON);
  tile_data_t cyan_data = tile_data_create(TILE_CYAN, 1);
  tile_t *first_tile = tile_create_ptr(center, cyan_data);
  add_tile(board, first_tile);

  // Should be singleton initially
  assert(first_tile->pool_id == 0);
  assert(board->pools->num_pools == 0);

  // Add adjacent tile of same color
  grid_cell_t neighbor_cells[6];
  board->geometry->get_neighbor_cells(center, neighbor_cells);
  tile_t *second_tile = tile_create_ptr(neighbor_cells[0], cyan_data);
  add_tile(board, second_tile);

  // Both tiles should now be in the same pool
  assert(first_tile->pool_id != 0);
  assert(second_tile->pool_id != 0);
  assert(first_tile->pool_id == second_tile->pool_id);
  assert(board->pools->num_pools == 1);

  printf("✓ Adjacent tiles pool creation test passed\n");
  free(board);
}

void test_pool_merging() {
  printf("Testing pool merging logic...\n");

  board_t *board = board_create(GRID_TYPE_HEXAGON, 5, BOARD_TYPE_MAIN);

  grid_cell_t center = grid_get_center_cell(GRID_TYPE_HEXAGON);
  grid_cell_t neighbor_cells[6];
  board->geometry->get_neighbor_cells(center, neighbor_cells);

  tile_data_t yellow_data = tile_data_create(TILE_YELLOW, 1);

  // Create first pair of adjacent tiles
  tile_t *tile1 = tile_create_ptr(center, yellow_data);
  tile_t *tile2 = tile_create_ptr(neighbor_cells[0], yellow_data);
  add_tile(board, tile1);
  add_tile(board, tile2);

  int pool_id_1 = tile1->pool_id;
  assert(pool_id_1 != 0);
  assert(tile2->pool_id == pool_id_1);
  assert(board->pools->num_pools == 1);

  // Create second pair of adjacent tiles (not connected to first pair yet)
  grid_cell_t far_neighbor_cells[6];
  board->geometry->get_neighbor_cells(neighbor_cells[2], far_neighbor_cells);
  tile_t *tile3 = tile_create_ptr(neighbor_cells[2], yellow_data);
  tile_t *tile4 = tile_create_ptr(far_neighbor_cells[0], yellow_data);
  add_tile(board, tile3);
  add_tile(board, tile4);

  int pool_id_2 = tile3->pool_id;
  assert(pool_id_2 != 0);
  assert(tile4->pool_id == pool_id_2);
  assert(pool_id_1 != pool_id_2); // Different pools initially
  assert(board->pools->num_pools == 2);

  // Now connect the two pools by placing a tile between them
  tile_t *bridge_tile = tile_create_ptr(neighbor_cells[1], yellow_data);
  add_tile(board, bridge_tile);

  // All tiles should now be in the same pool (pools merged)
  int merged_pool_id = tile1->pool_id;
  assert(tile1->pool_id == merged_pool_id);
  assert(tile2->pool_id == merged_pool_id);
  assert(tile3->pool_id == merged_pool_id);
  assert(tile4->pool_id == merged_pool_id);
  assert(bridge_tile->pool_id == merged_pool_id);
  assert(board->pools->num_pools == 1);

  printf("✓ Pool merging test passed\n");
  free(board);
}

void test_pool_dissolution() {
  printf("Testing pool dissolution when tiles removed...\n");

  board_t *board = board_create(GRID_TYPE_HEXAGON, 5, BOARD_TYPE_MAIN);

  // Create a small pool with 2 tiles
  grid_cell_t center = grid_get_center_cell(GRID_TYPE_HEXAGON);
  grid_cell_t neighbor_cells[6];
  board->geometry->get_neighbor_cells(center, neighbor_cells);

  tile_data_t green_data = tile_data_create(TILE_GREEN, 1);
  tile_t *tile1 = tile_create_ptr(center, green_data);
  tile_t *tile2 = tile_create_ptr(neighbor_cells[0], green_data);

  add_tile(board, tile1);
  add_tile(board, tile2);

  assert(tile1->pool_id != 0);
  assert(tile2->pool_id != 0);
  assert(tile1->pool_id == tile2->pool_id);
  assert(board->pools->num_pools == 1);

  // Remove one tile - should dissolve pool and convert remaining to singleton
  remove_tile(board, tile2);

  assert(tile1->pool_id == 0);          // Should become singleton
  assert(board->pools->num_pools == 0); // No pools left

  printf("✓ Pool dissolution test passed\n");
  free(board);
}

void test_different_colors_no_pool() {
  printf("Testing different colored adjacent tiles don't form pools...\n");

  board_t *board = board_create(GRID_TYPE_HEXAGON, 5, BOARD_TYPE_MAIN);

  grid_cell_t center = grid_get_center_cell(GRID_TYPE_HEXAGON);
  grid_cell_t neighbor_cells[6];
  board->geometry->get_neighbor_cells(center, neighbor_cells);

  // Place different colored tiles adjacent to each other
  tile_data_t magenta_data = tile_data_create(TILE_MAGENTA, 1);
  tile_data_t cyan_data = tile_data_create(TILE_CYAN, 1);

  tile_t *magenta_tile = tile_create_ptr(center, magenta_data);
  tile_t *cyan_tile = tile_create_ptr(neighbor_cells[0], cyan_data);

  add_tile(board, magenta_tile);
  add_tile(board, cyan_tile);

  // Both should remain singletons
  assert(magenta_tile->pool_id == 0);
  assert(cyan_tile->pool_id == 0);
  assert(board->pools->num_pools == 0);

  printf("✓ Different colors no pool test passed\n");
  free(board);
}

int main(void) {
  printf("=== Pool Logic Tests ===\n\n");

  test_singleton_tile();
  test_adjacent_tiles_create_pool();
  test_pool_merging();
  test_pool_dissolution();
  test_different_colors_no_pool();

  printf("\n✅ All pool logic tests passed!\n");
  return 0;
}
