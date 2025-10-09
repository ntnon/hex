/**************************************************************************/ /**
                                                                              * @file demo_rewards.c
                                                                              * @brief Simple demonstration of the reward system
                                                                              *
                                                                              * This demonstrates the basic reward system functionality including:
                                                                              * - Reward generation based on game state
                                                                              * - Rule creation from reward templates
                                                                              * - Selection and confirmation workflow
                                                                              * - Integration with the game's rule system
                                                                              *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Core game includes
#include "game/board.h"
#include "game/reward_state.h"
#include "game/reward_system.h"
#include "game/rule_manager.h"
#include "game/rule_system.h"
#include "tile/tile.h"

void print_separator(const char *title) {
  printf("\n");
  printf("================================================\n");
  printf(" %s\n", title);
  printf("================================================\n");
}

void print_rule_summary(const rule_t *rule) {
  printf("Rule Details:\n");
  printf("  ID: %u\n", rule->id);
  printf("  Priority: %u\n", rule->priority);
  printf("  Scope: %s\n", rule->scope == RULE_SCOPE_SELF           ? "Self"
                          : rule->scope == RULE_SCOPE_NEIGHBORS    ? "Neighbors"
                          : rule->scope == RULE_SCOPE_RANGE        ? "Range"
                          : rule->scope == RULE_SCOPE_POOL         ? "Pool"
                          : rule->scope == RULE_SCOPE_BOARD_GLOBAL ? "Global"
                                                                   : "Unknown");

  printf("  Effect: %s\n",
         rule->effect_type == RULE_EFFECT_ADD_FLAT        ? "Add Flat Value"
         : rule->effect_type == RULE_EFFECT_ADD_SCALED    ? "Add Scaled Value"
         : rule->effect_type == RULE_EFFECT_MULTIPLY      ? "Multiply"
         : rule->effect_type == RULE_EFFECT_SET_VALUE     ? "Set Value"
         : rule->effect_type == RULE_EFFECT_OVERRIDE_TYPE ? "Override Type"
         : rule->effect_type == RULE_EFFECT_MODIFY_RANGE  ? "Modify Range"
                                                          : "Unknown");

  printf("  Active: %s\n", rule->is_active ? "Yes" : "No");
  printf("  Range: %u\n", rule->affected_range);
}

void demonstrate_reward_generation() {
  print_separator("REWARD GENERATION DEMO");

  // Initialize reward system
  reward_system_t reward_system;
  if (!reward_system_init(&reward_system, (uint32_t)time(NULL))) {
    printf("❌ Failed to initialize reward system\n");
    return;
  }

  // Create a test board
  board_t *test_board = board_create(GRID_TYPE_HEXAGON, 8, BOARD_TYPE_MAIN);
  if (!test_board) {
    printf("❌ Failed to create test board\n");
    reward_system_cleanup(&reward_system);
    return;
  }

  // Add some tiles to make it interesting
  printf("Setting up test board with sample tiles...\n");
  for (int q = -2; q <= 2; q++) {
    for (int r = -2; r <= 2; r++) {
      if (abs(q + r) <= 2) {
        grid_cell_t cell = {.type = GRID_TYPE_HEXAGON,
                            .coord.hex = {.q = q, .r = r, .s = -(q + r)}};

        tile_t *tile = tile_create_random_ptr(cell);
        if (tile) {
          add_tile_to_board(test_board, tile);
        }
      }
    }
  }

  printf("Created board with %d tiles\n", test_board->tiles->num_tiles);

  // Generate rewards for different player levels
  for (uint32_t level = 1; level <= 5; level++) {
    printf("\n--- Player Level %u Rewards ---\n", level);

    reward_generation_params_t params =
      reward_system_create_default_params(test_board, NULL, level, level * 3);

    if (reward_system_generate_options(&reward_system, &params)) {
      reward_option_t *options;
      uint8_t count;

      if (reward_system_get_options(&reward_system, &options, &count)) {
        printf("Generated %u reward options:\n", count);

        for (uint8_t i = 0; i < count; i++) {
          const char *rarity_name;
          const char *category_name;
          const char *category_icon;
          uint32_t rarity_color;

          reward_system_get_rarity_info(options[i].rarity, &rarity_name,
                                        &rarity_color);
          reward_system_get_category_info(options[i].category, &category_name,
                                          &category_icon);

          printf("  [%u] %s %s - %s\n", i, category_icon, rarity_name,
                 options[i].title);
          printf("      %s\n", options[i].description);
          printf("      Power: %.1f\n", options[i].power_score);
        }
      }
    }
  }

  // Cleanup
  free_board(test_board);
  reward_system_cleanup(&reward_system);
  printf("✅ Reward generation demo completed\n");
}

void demonstrate_reward_selection() {
  print_separator("REWARD SELECTION DEMO");

  // Initialize systems
  reward_system_t reward_system;
  reward_state_t reward_state;

  if (!reward_system_init(&reward_system, 42) ||
      !reward_state_init(&reward_state, 84)) {
    printf("❌ Failed to initialize systems\n");
    return;
  }

  // Create test board
  board_t *test_board = board_create(GRID_TYPE_HEXAGON, 5, BOARD_TYPE_MAIN);

  // Generate rewards
  reward_generation_params_t params =
    reward_system_create_default_params(test_board, NULL, 3, 10);

  if (!reward_system_generate_options(&reward_system, &params)) {
    printf("❌ Failed to generate rewards\n");
    goto cleanup;
  }

  // Get and display options
  reward_option_t *options;
  uint8_t count;
  if (!reward_system_get_options(&reward_system, &options, &count) ||
      count == 0) {
    printf("❌ No reward options available\n");
    goto cleanup;
  }

  printf("Available reward options:\n");
  for (uint8_t i = 0; i < count; i++) {
    printf("  [%u] %s\n", i, options[i].title);
    printf("      %s\n", options[i].description);
  }

  // Simulate player selection (select first option)
  printf("\nSimulating player selecting option 0...\n");
  if (!reward_system_select_option(&reward_system, 0)) {
    printf("❌ Failed to select option\n");
    goto cleanup;
  }

  const reward_option_t *selected =
    reward_system_get_selected_option(&reward_system);
  if (selected) {
    printf("✅ Selected: %s\n", selected->title);
  }

  // Confirm selection
  printf("Confirming selection...\n");
  rule_t selected_rule;
  if (reward_system_confirm_selection(&reward_system, &selected_rule)) {
    printf("✅ Selection confirmed!\n");
    print_rule_summary(&selected_rule);
  } else {
    printf("❌ Failed to confirm selection\n");
  }

cleanup:
  if (test_board)
    free_board(test_board);
  reward_state_cleanup(&reward_state);
  reward_system_cleanup(&reward_system);
  printf("✅ Selection demo completed\n");
}

void demonstrate_rule_integration() {
  print_separator("RULE INTEGRATION DEMO");

  // Create a complete game setup
  board_t *game_board = board_create(GRID_TYPE_HEXAGON, 6, BOARD_TYPE_MAIN);
  rule_manager_t rule_manager;
  reward_system_t reward_system;

  if (!game_board || !rule_manager_init(&rule_manager, game_board, 100) ||
      !reward_system_init(&reward_system, 123)) {
    printf("❌ Failed to initialize systems\n");
    goto cleanup;
  }

  printf("Initialized game systems:\n");
  printf("  Board: %d radius\n", game_board->radius);
  printf("  Rule manager: Ready\n");
  printf("  Reward system: Ready\n");

  // Add some initial tiles
  printf("\nAdding initial tiles to board...\n");
  for (int i = 0; i < 5; i++) {
    grid_cell_t cell = {.type = GRID_TYPE_HEXAGON,
                        .coord.hex = {.q = i - 2, .r = 0, .s = 2 - i}};

    tile_t *tile = tile_create_random_ptr(cell);
    if (tile && add_tile_to_board(game_board, tile)) {
      printf("  Added tile at (%d, %d)\n", cell.coord.hex.q, cell.coord.hex.r);
    }
  }

  // Generate and select a reward
  printf("\nGenerating reward for current game state...\n");
  reward_generation_params_t params =
    reward_system_create_default_params(game_board, &rule_manager, 2, 8);

  if (reward_system_generate_options(&reward_system, &params)) {
    reward_option_t *options;
    uint8_t count;

    if (reward_system_get_options(&reward_system, &options, &count) &&
        count > 0) {
      printf("Generated %u rewards, selecting first one...\n", count);

      // Select and confirm
      reward_system_select_option(&reward_system, 0);
      rule_t new_rule;

      if (reward_system_confirm_selection(&reward_system, &new_rule)) {
        printf("✅ Reward confirmed: %s\n", options[0].title);

        // Add rule to rule manager
        uint32_t rule_id = rule_manager_add_rule(&rule_manager, &new_rule);
        if (rule_id > 0) {
          printf("✅ Rule added to game with ID: %u\n", rule_id);

          // Test the rule on some tiles
          printf("\nTesting rule effects:\n");

          // Create a test tile
          grid_cell_t test_cell = {.type = GRID_TYPE_HEXAGON,
                                   .coord.hex = {.q = 0, .r = 0, .s = 0}};
          tile_t *test_tile = tile_create_random_ptr(test_cell);

          if (test_tile) {
            float base_production = tile_get_effective_production(test_tile);
            float rule_production =
              rule_manager_calculate_tile_production(&rule_manager, test_tile);

            printf("  Base production: %.2f\n", base_production);
            printf("  With rules: %.2f\n", rule_production);
            printf("  Rule effect: %+.2f\n", rule_production - base_production);

            tile_destroy(test_tile);
          }
        }
      }
    }
  }

cleanup:
  if (game_board)
    free_board(game_board);
  rule_manager_cleanup(&rule_manager);
  reward_system_cleanup(&reward_system);
  printf("✅ Integration demo completed\n");
}

void print_usage_instructions() {
  print_separator("HOW TO USE THE REWARD SYSTEM");

  printf("The reward system is now integrated into the game! Here's how to use "
         "it:\n\n");

  printf("🎮 IN-GAME CONTROLS:\n");
  printf("  • Press SPACEBAR to trigger reward selection (manual)\n");
  printf("  • Click on reward cards to select them\n");
  printf("  • Click 'Confirm Selection' to apply the reward\n");
  printf("  • Click 'Skip Reward' to dismiss (if available)\n\n");

  printf("🔧 INTEGRATION POINTS:\n");
  printf("  • Rewards are automatically triggered at turn end\n");
  printf("  • Reward rules are added to the game's rule manager\n");
  printf("  • Rules affect tile production, range, and interactions\n");
  printf("  • Rules persist for the entire game session\n\n");

  printf("📊 REWARD TYPES:\n");
  printf("  • Production: Increase tile output\n");
  printf("  • Range: Extend tile influence\n");
  printf("  • Synergy: Create tile type interactions\n");
  printf("  • Global: Affect entire board\n");
  printf("  • Special: Unique game-changing effects\n\n");

  printf("🏆 RARITY SYSTEM:\n");
  printf("  • Common: Basic improvements (gray)\n");
  printf("  • Uncommon: Notable bonuses (green)\n");
  printf("  • Rare: Significant effects (blue)\n");
  printf("  • Legendary: Game-changing powers (orange)\n\n");

  printf(
    "The system scales rewards based on player level and game progress.\n");
  printf("Higher level players get access to more powerful reward types!\n");
}

int main() {
  printf("🎯 HEXHEX REWARD SYSTEM DEMONSTRATION\n");
  printf("=====================================\n");
  printf("This demo shows the complete reward system functionality.\n");

  // Run all demonstrations
  demonstrate_reward_generation();
  demonstrate_reward_selection();
  demonstrate_rule_integration();
  print_usage_instructions();

  printf("\n🎉 DEMO COMPLETE!\n");
  printf("The reward system is ready for use in the game.\n");
  printf("Compile and run the main game to see it in action!\n\n");

  return 0;
}
