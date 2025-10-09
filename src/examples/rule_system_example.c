/**************************************************************************/ /**
                                                                              * @file rule_system_example.c
                                                                              * @brief Example demonstrating the rule system functionality
                                                                              *****************************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "game/board.h"
#include "game/game.h"
#include "game/rule_manager.h"
#include "game/rule_system.h"
#include "grid/hex_geometry.h"
#include "tile/tile.h"

/**
 * @brief Example 1: Basic neighbor bonus rule
 */
void example_neighbor_bonus() {
  printf("\n=== Example 1: Neighbor Bonus Rule ===\n");

  // Create a simple game setup
  game_t game = {0};
  game.board = board_create(GRID_TYPE_HEXAGON, 3, BOARD_TYPE_MAIN);

  rule_manager_t rule_manager;
  if (!rule_manager_init(&rule_manager, game.board, 1000)) {
    printf("Failed to initialize rule manager\n");
    return;
  }

  // Create a magenta tile at center that gives +1 per same-color neighbor
  grid_cell_t center = {0, 0};
  tile_data_t tile_data = tile_data_create(TILE_MAGENTA, 5);
  tile_t *center_tile = tile_create_ptr(center, tile_data);

  // Add tile to board
  tile_map_set(game.board->tiles, center_tile);

  // Create rule for this tile
  rule_manager_on_tile_placed(&rule_manager, center_tile, game.board);

  printf("Created magenta tile with base production: %.1f\n",
         tile_get_effective_production(center_tile));

  // Add some magenta neighbors
  grid_cell_t neighbor1 = {1, 0};
  grid_cell_t neighbor2 = {0, 1};

  tile_data_t neighbor_data = tile_data_create(TILE_MAGENTA, 3);
  tile_t *neighbor_tile1 = tile_create_ptr(neighbor1, neighbor_data);
  tile_t *neighbor_tile2 = tile_create_ptr(neighbor2, neighbor_data);

  tile_map_set(game.board->tiles, neighbor_tile1);
  tile_map_set(game.board->tiles, neighbor_tile2);

  // Calculate production with neighbor bonus
  float production_with_bonus = rule_manager_calculate_tile_production(
    &rule_manager, center_tile, game.board);

  printf("Production with 2 same-color neighbors: %.1f\n",
         production_with_bonus);
  printf("Expected: %.1f (base) + %.1f (2 neighbors * 1.0 bonus) = %.1f\n",
         tile_get_effective_production(center_tile), 2.0f,
         tile_get_effective_production(center_tile) + 2.0f);

  // Add a different color neighbor (shouldn't contribute to bonus)
  grid_cell_t neighbor3 = {-1, 0};
  tile_data_t cyan_data = tile_data_create(TILE_CYAN, 2);
  tile_t *cyan_tile = tile_create_ptr(neighbor3, cyan_data);
  tile_map_set(game.board->tiles, cyan_tile);

  float production_mixed = rule_manager_calculate_tile_production(
    &rule_manager, center_tile, game.board);

  printf("Production with mixed neighbors: %.1f\n", production_mixed);
  printf("(Should be same as before since cyan doesn't match magenta)\n");

  // Cleanup
  rule_manager_cleanup(&rule_manager);
  free_board(game.board);
  printf("Example 1 completed.\n");
}

/**
 * @brief Example 2: Perception override rule
 */
void example_perception_override() {
  printf("\n=== Example 2: Perception Override Rule ===\n");

  game_t game = {0};
  game.board = board_create(GRID_TYPE_HEXAGON, 3, BOARD_TYPE_MAIN);

  rule_manager_t rule_manager;
  if (!rule_manager_init(&rule_manager, game.board, 1000)) {
    printf("Failed to initialize rule manager\n");
    return;
  }

  // Create a cyan tile that makes neighbors appear cyan
  grid_cell_t center = {0, 0};
  tile_data_t cyan_data = tile_data_create(TILE_CYAN, 4);
  tile_t *cyan_tile = tile_create_ptr(center, cyan_data);

  tile_map_set(game.board->tiles, cyan_tile);
  rule_manager_on_tile_placed(&rule_manager, cyan_tile, game.board);

  // Create a magenta tile that gives bonus per same-color neighbor
  grid_cell_t neighbor_pos = {1, 0};
  tile_data_t magenta_data = tile_data_create(TILE_MAGENTA, 5);
  tile_t *magenta_tile = tile_create_ptr(neighbor_pos, magenta_data);

  tile_map_set(game.board->tiles, magenta_tile);
  rule_manager_on_tile_placed(&rule_manager, magenta_tile, game.board);

  // Add more neighbors of different colors
  grid_cell_t pos2 = {0, 1};
  grid_cell_t pos3 = {-1, 1};

  tile_data_t yellow_data = tile_data_create(TILE_YELLOW, 3);
  tile_data_t green_data = tile_data_create(TILE_GREEN, 2);

  tile_t *yellow_tile = tile_create_ptr(pos2, yellow_data);
  tile_t *green_tile = tile_create_ptr(pos3, green_data);

  tile_map_set(game.board->tiles, yellow_tile);
  tile_map_set(game.board->tiles, green_tile);

  printf("Created tiles:\n");
  printf("- Cyan tile at (0,0) with perception override\n");
  printf("- Magenta tile at (1,0) with neighbor bonus\n");
  printf("- Yellow tile at (0,1)\n");
  printf("- Green tile at (-1,1)\n");

  // The cyan tile should make neighbors appear cyan to the magenta tile's rule
  float magenta_production = rule_manager_calculate_tile_production(
    &rule_manager, magenta_tile, game.board);

  printf("Magenta tile production: %.1f\n", magenta_production);
  printf("(With perception override, neighbors should appear as cyan,\n");
  printf(" but magenta rule looks for magenta neighbors, so no bonus)\n");

  rule_manager_cleanup(&rule_manager);
  free_board(game.board);
  printf("Example 2 completed.\n");
}

/**
 * @brief Example 3: Pool-based rules
 */
void example_pool_rules() {
  printf("\n=== Example 3: Pool-Based Rules ===\n");

  game_t game = {0};
  game.board = board_create(GRID_TYPE_HEXAGON, 5, BOARD_TYPE_MAIN);

  rule_manager_t rule_manager;
  if (!rule_manager_init(&rule_manager, game.board, 1000)) {
    printf("Failed to initialize rule manager\n");
    return;
  }

  // Create a large pool of magenta tiles
  grid_cell_t positions[] = {{0, 0},  {1, 0},  {0, 1}, {-1, 1},
                             {-1, 0}, {0, -1}, {1, -1}};
  int position_count = sizeof(positions) / sizeof(positions[0]);

  printf("Creating a large magenta pool (%d tiles):\n", position_count);

  for (int i = 0; i < position_count; i++) {
    tile_data_t data = tile_data_create(TILE_MAGENTA, 3 + i);
    tile_t *tile = tile_create_ptr(positions[i], data);

    tile_map_set(game.board->tiles, tile);
    rule_manager_on_tile_placed(&rule_manager, tile, game.board);

    printf("- Tile at (%d, %d) with base production: %.1f\n", positions[i].q,
           positions[i].r, tile_get_effective_production(tile));
  }

  // Simulate pool creation (normally done by board logic)
  pool_t test_pool = {0};
  test_pool.id = 1;
  test_pool.compactness_score = 0.9f; // High compactness

  // Create pool rules
  rule_manager_on_pool_changed(&rule_manager, &test_pool, game.board);

  printf("\nPool created with high compactness score: %.2f\n",
         test_pool.compactness_score);
  printf("Pool size: %d tiles (should trigger size bonus)\n", position_count);

  // Show rule statistics
  rule_manager_print_stats(&rule_manager);

  rule_manager_cleanup(&rule_manager);
  free_board(game.board);
  printf("Example 3 completed.\n");
}

/**
 * @brief Example 4: Instant rules
 */
void example_instant_rules() {
  printf("\n=== Example 4: Instant Rules ===\n");

  game_t game = {0};
  game.board = board_create(GRID_TYPE_HEXAGON, 3, BOARD_TYPE_MAIN);

  rule_manager_t rule_manager;
  if (!rule_manager_init(&rule_manager, game.board, 1000)) {
    printf("Failed to initialize rule manager\n");
    return;
  }

  // Create a tile
  grid_cell_t pos = {0, 0};
  tile_data_t data = tile_data_create(TILE_GREEN, 10);
  tile_t *tile = tile_create_ptr(pos, data);

  tile_map_set(game.board->tiles, tile);

  printf("Created green tile with base production: %.1f\n",
         tile_get_effective_production(tile));

  // Add instant modifier rules
  rule_manager_add_instant_modifier(&rule_manager, pos, RULE_TARGET_MODIFIER,
                                    5.0f);
  rule_manager_add_instant_modifier(&rule_manager, pos, RULE_TARGET_MODIFIER,
                                    -2.0f);

  printf("Added instant modifiers: +5.0 and -2.0\n");

  size_t rule_count_before = rule_manager_get_rule_count(&rule_manager);
  printf("Rules before execution: %zu\n", rule_count_before);

  // Execute instant rules
  rule_manager_update_production(&rule_manager, game.board);

  size_t rule_count_after = rule_manager_get_rule_count(&rule_manager);
  printf("Rules after execution: %zu (instant rules should be removed)\n",
         rule_count_after);

  // NOTE: In this example, the instant rules don't actually modify the tile
  // because the rule application logic is simplified. In a full implementation,
  // instant rules would permanently modify tile->modifier.

  rule_manager_cleanup(&rule_manager);
  free_board(game.board);
  printf("Example 4 completed.\n");
}

/**
 * @brief Example 5: Rule debugging and inspection
 */
void example_rule_debugging() {
  printf("\n=== Example 5: Rule Debugging ===\n");

  game_t game = {0};
  game.board = board_create(GRID_TYPE_HEXAGON, 3, BOARD_TYPE_MAIN);

  rule_manager_t rule_manager;
  if (!rule_manager_init(&rule_manager, game.board, 1000)) {
    printf("Failed to initialize rule manager\n");
    return;
  }

  // Create tiles with different types
  grid_cell_t positions[] = {{0, 0}, {1, 0}, {0, 1}};
  tile_type_t types[] = {TILE_MAGENTA, TILE_CYAN, TILE_YELLOW};

  for (int i = 0; i < 3; i++) {
    tile_data_t data = tile_data_create(types[i], 5);
    tile_t *tile = tile_create_ptr(positions[i], data);

    tile_map_set(game.board->tiles, tile);
    rule_manager_on_tile_placed(&rule_manager, tile, game.board);
  }

  // Print all rules
  rule_manager_print_all_rules(&rule_manager);

  // Print rules for a specific cell
  rule_manager_print_rules_for_cell(&rule_manager, positions[0]);

  // Print manager statistics
  rule_manager_print_stats(&rule_manager);

  rule_manager_cleanup(&rule_manager);
  free_board(game.board);
  printf("Example 5 completed.\n");
}

int main() {
  printf("=== Rule System Examples ===\n");
  printf("Demonstrating the hexhex rule system functionality.\n");

  example_neighbor_bonus();
  example_perception_override();
  example_pool_rules();
  example_instant_rules();
  example_rule_debugging();

  printf("\n=== All Examples Completed ===\n");
  printf("The rule system provides:\n");
  printf("1. Neighbor-based bonuses with type checking\n");
  printf("2. Perception overrides for complex interactions\n");
  printf("3. Pool-based rules for large formations\n");
  printf("4. Instant rules for permanent modifications\n");
  printf("5. Comprehensive debugging and inspection tools\n");
  printf("\nNext steps:\n");
  printf("- Integrate rule evaluation into production calculations\n");
  printf("- Add more complex rule conditions and effects\n");
  printf("- Implement rule priority and ordering systems\n");
  printf("- Add rule persistence and loading/saving\n");

  return 0;
}
