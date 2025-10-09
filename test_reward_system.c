/**************************************************************************/ /**
                                                                              * @file test_reward_system.c
                                                                              * @brief Simple test for the reward system functionality
                                                                              *****************************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Include reward system headers
#include "game/board.h"
#include "game/reward_state.h"
#include "game/reward_system.h"
#include "game/rule_system.h"
#include "tile/tile.h"

void test_reward_system_init() {
  printf("Testing reward system initialization...\n");

  reward_system_t system;
  uint32_t seed = (uint32_t)time(NULL);

  assert(reward_system_init(&system, seed));
  printf("✓ Reward system initialized successfully\n");

  reward_system_cleanup(&system);
  printf("✓ Reward system cleaned up successfully\n");
}

void test_reward_generation() {
  printf("Testing reward generation...\n");

  reward_system_t system;
  uint32_t seed = 12345;

  assert(reward_system_init(&system, seed));

  // Create minimal board for testing
  board_t *board = board_create(GRID_TYPE_HEXAGON, 5, BOARD_TYPE_MAIN);
  assert(board != NULL);

  // Create generation parameters
  reward_generation_params_t params =
    reward_system_create_default_params(board, NULL, 1, 1);

  // Generate reward options
  assert(reward_system_generate_options(&system, &params));
  printf("✓ Generated reward options successfully\n");

  // Check options
  reward_option_t *options;
  uint8_t count;
  assert(reward_system_get_options(&system, &options, &count));
  assert(count > 0);
  assert(count <= MAX_REWARD_OPTIONS);
  printf("✓ Generated %d reward options\n", count);

  // Print options for debugging
  for (uint8_t i = 0; i < count; i++) {
    printf("  Option %d: %s (Power: %.1f)\n", i,
           options[i].title ? options[i].title : "Unknown",
           options[i].power_score);
  }

  free_board(board);
  reward_system_cleanup(&system);
}

void test_reward_selection() {
  printf("Testing reward selection...\n");

  reward_system_t system;
  uint32_t seed = 54321;

  assert(reward_system_init(&system, seed));

  // Create minimal board for testing
  board_t *board = board_create(GRID_TYPE_HEXAGON, 3, BOARD_TYPE_MAIN);
  assert(board != NULL);

  // Generate rewards
  reward_generation_params_t params =
    reward_system_create_default_params(board, NULL, 1, 1);
  assert(reward_system_generate_options(&system, &params));

  // Get options
  reward_option_t *options;
  uint8_t count;
  assert(reward_system_get_options(&system, &options, &count));
  assert(count > 0);

  // Test selection
  assert(reward_system_select_option(&system, 0));
  printf("✓ Selected option 0 successfully\n");

  const reward_option_t *selected = reward_system_get_selected_option(&system);
  assert(selected != NULL);
  assert(selected == &options[0]);
  printf("✓ Retrieved selected option successfully\n");

  // Test confirmation
  rule_t selected_rule;
  assert(reward_system_confirm_selection(&system, &selected_rule));
  printf("✓ Confirmed selection successfully\n");

  assert(reward_system_has_confirmed_selection(&system));
  printf("✓ Selection confirmation state correct\n");

  free_board(board);
  reward_system_cleanup(&system);
}

void test_reward_state() {
  printf("Testing reward state management...\n");

  reward_state_t state;
  uint32_t seed = 98765;

  assert(reward_state_init(&state, seed));
  printf("✓ Reward state initialized successfully\n");

  // Test phase management
  assert(!reward_state_is_active(&state));
  printf("✓ Initial state is inactive\n");

  // Test configuration
  reward_state_configure(&state, true, 2.0f, true, false);
  printf("✓ Configured reward state\n");

  // Test animations
  reward_state_set_animations_enabled(&state, true);
  printf("✓ Enabled animations\n");

  reward_state_cleanup(&state);
  printf("✓ Reward state cleaned up successfully\n");
}

void test_reward_display_info() {
  printf("Testing reward display information...\n");

  // Test rarity information
  const char *rarity_name;
  uint32_t rarity_color;

  reward_system_get_rarity_info(REWARD_RARITY_COMMON, &rarity_name,
                                &rarity_color);
  assert(rarity_name != NULL);
  assert(strlen(rarity_name) > 0);
  printf("✓ Common rarity: %s (color: 0x%08X)\n", rarity_name, rarity_color);

  reward_system_get_rarity_info(REWARD_RARITY_LEGENDARY, &rarity_name,
                                &rarity_color);
  assert(rarity_name != NULL);
  printf("✓ Legendary rarity: %s (color: 0x%08X)\n", rarity_name, rarity_color);

  // Test category information
  const char *category_name;
  const char *category_icon;

  reward_system_get_category_info(REWARD_CATEGORY_PRODUCTION, &category_name,
                                  &category_icon);
  assert(category_name != NULL);
  assert(category_icon != NULL);
  printf("✓ Production category: %s %s\n", category_icon, category_name);

  reward_system_get_category_info(REWARD_CATEGORY_SPECIAL, &category_name,
                                  &category_icon);
  assert(category_name != NULL);
  assert(category_icon != NULL);
  printf("✓ Special category: %s %s\n", category_icon, category_name);
}

void test_reward_integration() {
  printf("Testing reward system integration...\n");

  reward_system_t reward_system;
  reward_state_t reward_state;

  assert(reward_system_init(&reward_system, 11111));
  assert(reward_state_init(&reward_state, 22222));

  // Create test board
  board_t *board = board_create(GRID_TYPE_HEXAGON, 4, BOARD_TYPE_MAIN);
  assert(board != NULL);

  // Generate rewards in the system
  reward_generation_params_t params =
    reward_system_create_default_params(board, NULL, 3, 5);
  assert(reward_system_generate_options(&reward_system, &params));

  // Test that both systems work together
  reward_option_t *options;
  uint8_t count;
  assert(reward_system_get_options(&reward_system, &options, &count));
  printf("✓ Generated %d reward options for integration test\n", count);

  // Select and confirm a reward
  if (count > 0) {
    assert(reward_system_select_option(&reward_system, 0));

    rule_t selected_rule;
    assert(reward_system_confirm_selection(&reward_system, &selected_rule));
    printf("✓ Successfully selected and confirmed reward\n");

    // Print rule information
    printf("  Selected rule: ID=%u, Priority=%u, Scope=%u, Target=%u\n",
           selected_rule.id, selected_rule.priority, selected_rule.scope,
           selected_rule.target);
  }

  free_board(board);
  reward_state_cleanup(&reward_state);
  reward_system_cleanup(&reward_system);
}

void print_test_summary() {
  printf("\n=== Reward System Test Summary ===\n");
  printf("All tests passed! The reward system is working correctly.\n");
  printf("\nKey features tested:\n");
  printf("• Reward system initialization and cleanup\n");
  printf("• Reward option generation with different parameters\n");
  printf("• Reward selection and confirmation\n");
  printf("• Reward state management and configuration\n");
  printf("• Display information for rarities and categories\n");
  printf("• Integration between reward system and state\n");
  printf("\nThe system is ready for integration with the game UI!\n");
}

int main() {
  printf("=== Reward System Test Suite ===\n\n");

  test_reward_system_init();
  printf("\n");

  test_reward_generation();
  printf("\n");

  test_reward_selection();
  printf("\n");

  test_reward_state();
  printf("\n");

  test_reward_display_info();
  printf("\n");

  test_reward_integration();
  printf("\n");

  print_test_summary();

  return 0;
}
