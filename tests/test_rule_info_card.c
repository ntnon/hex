#include "game/rule_system.h"
#include "ui/rule_info_card.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* ============================================================================
 * Rule Info Card Test Program
 *
 * This test program verifies the rule info card system functionality without
 * requiring a full UI context.
 * ============================================================================
 */

/**
 * @brief Test rule display info generation
 */
void test_rule_display_info(void) {
  printf("Testing rule display info generation...\n");

  // Create a neighbor bonus rule
  rule_t neighbor_rule = {0};
  neighbor_rule.id = 1;
  neighbor_rule.scope = RULE_SCOPE_NEIGHBORS;
  neighbor_rule.target = RULE_TARGET_PRODUCTION;
  neighbor_rule.condition_type = RULE_CONDITION_NEIGHBOR_COUNT;
  neighbor_rule.condition_params.neighbor_count.neighbor_type = TILE_YELLOW;
  neighbor_rule.condition_params.neighbor_count.min_count = 1;
  neighbor_rule.condition_params.neighbor_count.max_count = 3;
  neighbor_rule.effect_type = RULE_EFFECT_ADD_FLAT;
  neighbor_rule.effect_params.value = 2.0f;
  neighbor_rule.is_active = true;

  rule_display_info_t info = rule_get_display_info(&neighbor_rule);

  assert(info.type_icon != NULL);
  assert(info.scope_icon != NULL);
  assert(info.target_icon != NULL);
  assert(info.name != NULL);
  assert(info.description != NULL);
  assert(info.display_priority > 0);

  printf("  ✓ Neighbor rule info generated successfully\n");
  printf("    - Type: %s %s\n", info.type_icon, info.name);
  printf("    - Description: %s\n", info.description);
  printf("    - Priority: %u\n", info.display_priority);

  // Create a type override rule
  rule_t override_rule = {0};
  override_rule.id = 2;
  override_rule.scope = RULE_SCOPE_RANGE;
  override_rule.target = RULE_TARGET_TYPE_OVERRIDE;
  override_rule.condition_type = RULE_CONDITION_ALWAYS;
  override_rule.effect_type = RULE_EFFECT_OVERRIDE_TYPE;
  override_rule.effect_params.override_type = TILE_MAGENTA;
  override_rule.affected_range = 2;
  override_rule.is_active = true;

  rule_display_info_t override_info = rule_get_display_info(&override_rule);

  assert(override_info.type_icon != NULL);
  assert(strcmp(override_info.type_icon, "🎭") == 0);
  assert(strcmp(override_info.name, "Type Override") == 0);

  printf("  ✓ Override rule info generated successfully\n");
  printf("    - Type: %s %s\n", override_info.type_icon, override_info.name);
  printf("    - Description: %s\n", override_info.description);
}

/**
 * @brief Test icon generation functions
 */
void test_icon_functions(void) {
  printf("Testing icon generation functions...\n");

  // Test scope icons
  assert(strcmp(rule_get_scope_icon(RULE_SCOPE_SELF), "👤") == 0);
  assert(strcmp(rule_get_scope_icon(RULE_SCOPE_NEIGHBORS), "👥") == 0);
  assert(strcmp(rule_get_scope_icon(RULE_SCOPE_RANGE), "🎯") == 0);
  assert(strcmp(rule_get_scope_icon(RULE_SCOPE_POOL), "🌊") == 0);
  assert(strcmp(rule_get_scope_icon(RULE_SCOPE_TYPE_GLOBAL), "🔗") == 0);
  assert(strcmp(rule_get_scope_icon(RULE_SCOPE_BOARD_GLOBAL), "🌍") == 0);

  // Test target icons
  assert(strcmp(rule_get_target_icon(RULE_TARGET_PRODUCTION), "⚡") == 0);
  assert(strcmp(rule_get_target_icon(RULE_TARGET_RANGE), "📏") == 0);
  assert(strcmp(rule_get_target_icon(RULE_TARGET_POOL_MODIFIER), "💧") == 0);
  assert(strcmp(rule_get_target_icon(RULE_TARGET_TYPE_OVERRIDE), "🎭") == 0);

  // Test condition icons
  assert(strcmp(rule_get_condition_icon(RULE_CONDITION_ALWAYS), "✅") == 0);
  assert(strcmp(rule_get_condition_icon(RULE_CONDITION_SELF_TYPE), "🔍") == 0);
  assert(strcmp(rule_get_condition_icon(RULE_CONDITION_NEIGHBOR_COUNT), "📊") ==
         0);

  // Test effect icons
  assert(strcmp(rule_get_effect_icon(RULE_EFFECT_ADD_FLAT), "➕") == 0);
  assert(strcmp(rule_get_effect_icon(RULE_EFFECT_MULTIPLY), "✖️") == 0);
  assert(strcmp(rule_get_effect_icon(RULE_EFFECT_OVERRIDE_TYPE), "🎭") == 0);

  printf("  ✓ All icon functions working correctly\n");
}

/**
 * @brief Test text generation functions
 */
void test_text_generation(void) {
  printf("Testing text generation functions...\n");

  // Create test rule
  rule_t test_rule = {0};
  test_rule.scope = RULE_SCOPE_NEIGHBORS;
  test_rule.target = RULE_TARGET_PRODUCTION;
  test_rule.condition_type = RULE_CONDITION_NEIGHBOR_COUNT;
  test_rule.condition_params.neighbor_count.neighbor_type = TILE_YELLOW;
  test_rule.condition_params.neighbor_count.min_count = 2;
  test_rule.condition_params.neighbor_count.max_count = 2;
  test_rule.effect_type = RULE_EFFECT_ADD_FLAT;
  test_rule.effect_params.value = 3.0f;

  char buffer[256];
  int length;

  // Test description generation
  length = rule_generate_description(&test_rule, buffer, sizeof(buffer));
  assert(length > 0);
  assert(strlen(buffer) == length);
  printf("  ✓ Description: %s\n", buffer);

  // Test condition text generation
  length = rule_generate_condition_text(&test_rule, buffer, sizeof(buffer));
  assert(length > 0);
  assert(strstr(buffer, "Yellow") != NULL);
  assert(strstr(buffer, "2") != NULL);
  printf("  ✓ Condition: %s\n", buffer);

  // Test effect text generation
  length = rule_generate_effect_text(&test_rule, buffer, sizeof(buffer));
  assert(length > 0);
  assert(strstr(buffer, "+3.0") != NULL);
  assert(strstr(buffer, "Production") != NULL);
  printf("  ✓ Effect: %s\n", buffer);

  // Test scope text generation
  length = rule_generate_scope_text(&test_rule, buffer, sizeof(buffer));
  assert(length > 0);
  assert(strstr(buffer, "neighbors") != NULL);
  printf("  ✓ Scope: %s\n", buffer);
}

/**
 * @brief Test rule analysis functions
 */
void test_rule_analysis(void) {
  printf("Testing rule analysis functions...\n");

  // Create rules with different importance levels
  rule_t important_rule = {0};
  important_rule.scope = RULE_SCOPE_BOARD_GLOBAL;
  important_rule.target = RULE_TARGET_PRODUCTION;
  important_rule.effect_type = RULE_EFFECT_ADD_FLAT;
  important_rule.effect_params.value = 10.0f;

  rule_t normal_rule = {0};
  normal_rule.scope = RULE_SCOPE_SELF;
  normal_rule.target = RULE_TARGET_PRODUCTION;
  normal_rule.effect_type = RULE_EFFECT_ADD_FLAT;
  normal_rule.effect_params.value = 1.0f;

  // Test visual importance
  assert(rule_is_visually_important(&important_rule) == true);
  assert(rule_is_visually_important(&normal_rule) == false);

  // Test display priority
  uint32_t important_priority = rule_get_display_priority(&important_rule);
  uint32_t normal_priority = rule_get_display_priority(&normal_rule);
  assert(important_priority > normal_priority);

  printf("  ✓ Important rule priority: %u\n", important_priority);
  printf("  ✓ Normal rule priority: %u\n", normal_priority);

  // Test tile type functions
  assert(strcmp(rule_get_tile_type_name(TILE_MAGENTA), "Magenta") == 0);
  assert(strcmp(rule_get_tile_type_name(TILE_CYAN), "Cyan") == 0);
  assert(strcmp(rule_get_tile_type_name(TILE_YELLOW), "Yellow") == 0);
  assert(strcmp(rule_get_tile_type_name(TILE_GREEN), "Green") == 0);

  printf("  ✓ Rule analysis functions working correctly\n");
}

/**
 * @brief Test rule sorting functionality
 */
void test_rule_sorting(void) {
  printf("Testing rule sorting functionality...\n");

  // Create rules with different priorities
  rule_t rule1 = {0};
  rule1.scope = RULE_SCOPE_SELF;
  rule1.target = RULE_TARGET_PRODUCTION;

  rule_t rule2 = {0};
  rule2.scope = RULE_SCOPE_BOARD_GLOBAL;
  rule2.target = RULE_TARGET_TYPE_OVERRIDE;

  rule_t rule3 = {0};
  rule3.scope = RULE_SCOPE_NEIGHBORS;
  rule3.target = RULE_TARGET_RANGE;

  const rule_t *rules[] = {&rule1, &rule2, &rule3};
  uint32_t rule_count = 3;

  // Get initial priorities
  uint32_t priority1 = rule_get_display_priority(&rule1);
  uint32_t priority2 = rule_get_display_priority(&rule2);
  uint32_t priority3 = rule_get_display_priority(&rule3);

  printf("  Original priorities: %u, %u, %u\n", priority1, priority2,
         priority3);

  // Sort rules
  rule_sort_by_priority(rules, rule_count);

  // Verify sorted order
  for (uint32_t i = 0; i < rule_count - 1; i++) {
    uint32_t current_priority = rule_get_display_priority(rules[i]);
    uint32_t next_priority = rule_get_display_priority(rules[i + 1]);
    assert(current_priority >= next_priority);
  }

  printf("  ✓ Rules sorted correctly by priority\n");
}

/**
 * @brief Test configuration functions
 */
void test_configuration(void) {
  printf("Testing configuration functions...\n");

  // Test default configurations
  assert(RULE_CARD_CONFIG_FULL.show_icons == true);
  assert(RULE_CARD_CONFIG_FULL.show_descriptions == true);
  assert(RULE_CARD_CONFIG_FULL.show_conditions == true);
  assert(RULE_CARD_CONFIG_FULL.show_effects == true);
  assert(RULE_CARD_CONFIG_FULL.compact_layout == false);

  assert(RULE_CARD_CONFIG_COMPACT.show_icons == true);
  assert(RULE_CARD_CONFIG_COMPACT.show_descriptions == false);
  assert(RULE_CARD_CONFIG_COMPACT.compact_layout == true);

  assert(RULE_CARD_CONFIG_MINIMAL.show_icons == true);
  assert(RULE_CARD_CONFIG_MINIMAL.show_descriptions == false);
  assert(RULE_CARD_CONFIG_MINIMAL.show_conditions == false);
  assert(RULE_CARD_CONFIG_MINIMAL.show_effects == false);
  assert(RULE_CARD_CONFIG_MINIMAL.compact_layout == true);

  // Test custom configuration creation
  rule_card_config_t custom = rule_card_config_create(true, false, true);
  assert(custom.show_icons == true);
  assert(custom.show_descriptions == false);
  assert(custom.compact_layout == true);
  assert(custom.show_conditions == false); // Should be false for compact
  assert(custom.show_effects == true);     // Should always be true

  printf("  ✓ All configuration tests passed\n");
}

/**
 * @brief Test edge cases and error handling
 */
void test_edge_cases(void) {
  printf("Testing edge cases and error handling...\n");

  char buffer[256];
  int result;

  // Test NULL rule handling
  rule_display_info_t null_info = rule_get_display_info(NULL);
  assert(strcmp(null_info.type_icon, "❓") == 0);
  assert(strcmp(null_info.name, "Invalid Rule") == 0);

  // Test text generation with NULL
  result = rule_generate_description(NULL, buffer, sizeof(buffer));
  assert(result == 0);

  result = rule_generate_condition_text(NULL, buffer, sizeof(buffer));
  assert(result == 0);

  result = rule_generate_effect_text(NULL, buffer, sizeof(buffer));
  assert(result == 0);

  // Test text generation with small buffer
  char small_buffer[10];
  rule_t test_rule = {0};
  test_rule.scope = RULE_SCOPE_NEIGHBORS;
  test_rule.target = RULE_TARGET_PRODUCTION;

  result =
    rule_generate_description(&test_rule, small_buffer, sizeof(small_buffer));
  assert(result > 0);
  assert(result < sizeof(small_buffer));

  // Test unknown enum values
  assert(strcmp(rule_get_scope_icon((rule_scope_t)999), "❓") == 0);
  assert(strcmp(rule_get_target_icon((rule_target_t)999), "❓") == 0);
  assert(strcmp(rule_get_condition_icon((rule_condition_type_t)999), "❓") ==
         0);
  assert(strcmp(rule_get_effect_icon((rule_effect_type_t)999), "❓") == 0);

  printf("  ✓ All edge case tests passed\n");
}

/**
 * @brief Main test function
 */
int main(void) {
  printf("Starting Rule Info Card System Tests\n");
  printf("====================================\n\n");

  test_rule_display_info();
  printf("\n");

  test_icon_functions();
  printf("\n");

  test_text_generation();
  printf("\n");

  test_rule_analysis();
  printf("\n");

  test_rule_sorting();
  printf("\n");

  test_configuration();
  printf("\n");

  test_edge_cases();
  printf("\n");

  printf("====================================\n");
  printf("All tests passed! ✓\n\n");

  printf("Rule Info Card System Features:\n");
  printf("- Visual rule representation with icons\n");
  printf("- Configurable display modes (full, compact, minimal)\n");
  printf("- Human-readable text generation\n");
  printf("- Rule analysis and prioritization\n");
  printf("- Support for all rule types and conditions\n");
  printf("- Robust error handling\n");

  return 0;
}
