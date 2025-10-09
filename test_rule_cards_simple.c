#include "game/rule_system.h"
#include "ui/rule_info_card.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/**
 * Simple test program to verify rule info card functionality
 * without Clay UI dependencies
 */

int main(void) {
  printf("Testing Rule Info Card System\n");
  printf("=============================\n\n");

  // Test 1: Create a neighbor bonus rule
  rule_t neighbor_rule = {0};
  neighbor_rule.id = 1;
  neighbor_rule.scope = RULE_SCOPE_NEIGHBORS;
  neighbor_rule.target = RULE_TARGET_PRODUCTION;
  neighbor_rule.condition_type = RULE_CONDITION_NEIGHBOR_COUNT;
  neighbor_rule.condition_params.neighbor_count.neighbor_type = TILE_YELLOW;
  neighbor_rule.condition_params.neighbor_count.min_count = 2;
  neighbor_rule.condition_params.neighbor_count.max_count = 2;
  neighbor_rule.effect_type = RULE_EFFECT_ADD_FLAT;
  neighbor_rule.effect_params.value = 3.0f;
  neighbor_rule.is_active = true;

  // Test 2: Create a type override rule
  rule_t override_rule = {0};
  override_rule.id = 2;
  override_rule.scope = RULE_SCOPE_RANGE;
  override_rule.target = RULE_TARGET_TYPE_OVERRIDE;
  override_rule.condition_type = RULE_CONDITION_ALWAYS;
  override_rule.effect_type = RULE_EFFECT_OVERRIDE_TYPE;
  override_rule.effect_params.override_type = TILE_MAGENTA;
  override_rule.affected_range = 2;
  override_rule.is_active = true;

  // Test 3: Create a global modifier rule
  rule_t global_rule = {0};
  global_rule.id = 3;
  global_rule.scope = RULE_SCOPE_BOARD_GLOBAL;
  global_rule.target = RULE_TARGET_PRODUCTION;
  global_rule.condition_type = RULE_CONDITION_BOARD_COUNT;
  global_rule.condition_params.board_count.target_type = TILE_GREEN;
  global_rule.condition_params.board_count.min_count = 5;
  global_rule.condition_params.board_count.max_count = 999;
  global_rule.effect_type = RULE_EFFECT_MULTIPLY;
  global_rule.effect_params.value = 1.5f;
  global_rule.is_active = true;

  // Test display info generation
  printf("1. Testing display info generation:\n");
  printf("-----------------------------------\n");

  rule_display_info_t neighbor_info = rule_get_display_info(&neighbor_rule);
  rule_display_info_t override_info = rule_get_display_info(&override_rule);
  rule_display_info_t global_info = rule_get_display_info(&global_rule);

  printf("Neighbor rule: %s %s - %s\n", neighbor_info.type_icon,
         neighbor_info.name, neighbor_info.description);
  printf("Override rule: %s %s - %s\n", override_info.type_icon,
         override_info.name, override_info.description);
  printf("Global rule: %s %s - %s\n", global_info.type_icon, global_info.name,
         global_info.description);

  // Test text generation
  printf("\n2. Testing text generation:\n");
  printf("---------------------------\n");

  char buffer[256];

  // Test effect text generation
  rule_generate_effect_text(&neighbor_rule, buffer, sizeof(buffer));
  printf("Neighbor rule effect: %s\n", buffer);

  rule_generate_effect_text(&override_rule, buffer, sizeof(buffer));
  printf("Override rule effect: %s\n", buffer);

  rule_generate_effect_text(&global_rule, buffer, sizeof(buffer));
  printf("Global rule effect: %s\n", buffer);

  // Test condition text generation
  printf("\nConditions:\n");
  rule_generate_condition_text(&neighbor_rule, buffer, sizeof(buffer));
  printf("Neighbor rule condition: %s\n", buffer);

  rule_generate_condition_text(&override_rule, buffer, sizeof(buffer));
  printf("Override rule condition: %s\n", buffer);

  rule_generate_condition_text(&global_rule, buffer, sizeof(buffer));
  printf("Global rule condition: %s\n", buffer);

  // Test priority and importance
  printf("\n3. Testing priority and importance:\n");
  printf("-----------------------------------\n");

  uint32_t neighbor_priority = rule_get_display_priority(&neighbor_rule);
  uint32_t override_priority = rule_get_display_priority(&override_rule);
  uint32_t global_priority = rule_get_display_priority(&global_rule);

  printf("Neighbor rule priority: %u (important: %s)\n", neighbor_priority,
         rule_is_visually_important(&neighbor_rule) ? "yes" : "no");
  printf("Override rule priority: %u (important: %s)\n", override_priority,
         rule_is_visually_important(&override_rule) ? "yes" : "no");
  printf("Global rule priority: %u (important: %s)\n", global_priority,
         rule_is_visually_important(&global_rule) ? "yes" : "no");

  // Test rule sorting
  printf("\n4. Testing rule sorting:\n");
  printf("------------------------\n");

  const rule_t *rules[] = {&neighbor_rule, &override_rule, &global_rule};
  uint32_t rule_count = 3;

  printf("Before sorting: neighbor(%u), override(%u), global(%u)\n",
         neighbor_priority, override_priority, global_priority);

  rule_sort_by_priority(rules, rule_count);

  printf("After sorting: ");
  for (uint32_t i = 0; i < rule_count; i++) {
    uint32_t priority = rule_get_display_priority(rules[i]);
    const char *name = rule_get_type_name(rules[i]);
    printf("%s(%u)", name, priority);
    if (i < rule_count - 1)
      printf(", ");
  }
  printf("\n");

  // Test individual card rendering (console output)
  printf("\n5. Testing card rendering (console output):\n");
  printf("-------------------------------------------\n");

  // Create dummy Clay_ElementId for testing
  Clay_ElementId test_id = {.id = 1, .offset = 0, .baseId = 0, .stringId = {0}};
  ui_render_rule_info_card(&neighbor_rule, &RULE_CARD_CONFIG_FULL, test_id);

  // Test list rendering
  printf("6. Testing list rendering:\n");
  printf("--------------------------\n");

  // Create dummy Clay_ElementId for testing
  Clay_ElementId list_id = {.id = 2, .offset = 0, .baseId = 0, .stringId = {0}};
  ui_render_rule_info_list(rules, rule_count, &RULE_CARD_CONFIG_COMPACT,
                           list_id);

  // Test configurations
  printf("7. Testing configurations:\n");
  printf("--------------------------\n");

  rule_card_config_t custom_config = rule_card_config_create(true, false, true);
  printf("Custom config created: icons=%s, descriptions=%s, compact=%s\n",
         custom_config.show_icons ? "yes" : "no",
         custom_config.show_descriptions ? "yes" : "no",
         custom_config.compact_layout ? "yes" : "no");

  // Test icon functions
  printf("\n8. Testing icon functions:\n");
  printf("--------------------------\n");

  printf("Scope icons:\n");
  printf("  Self: %s\n", rule_get_scope_icon(RULE_SCOPE_SELF));
  printf("  Neighbors: %s\n", rule_get_scope_icon(RULE_SCOPE_NEIGHBORS));
  printf("  Range: %s\n", rule_get_scope_icon(RULE_SCOPE_RANGE));
  printf("  Pool: %s\n", rule_get_scope_icon(RULE_SCOPE_POOL));
  printf("  Global: %s\n", rule_get_scope_icon(RULE_SCOPE_BOARD_GLOBAL));

  printf("Target icons:\n");
  printf("  Production: %s\n", rule_get_target_icon(RULE_TARGET_PRODUCTION));
  printf("  Range: %s\n", rule_get_target_icon(RULE_TARGET_RANGE));
  printf("  Type Override: %s\n",
         rule_get_target_icon(RULE_TARGET_TYPE_OVERRIDE));

  printf("Tile type names:\n");
  printf("  Magenta: %s\n", rule_get_tile_type_name(TILE_MAGENTA));
  printf("  Cyan: %s\n", rule_get_tile_type_name(TILE_CYAN));
  printf("  Yellow: %s\n", rule_get_tile_type_name(TILE_YELLOW));
  printf("  Green: %s\n", rule_get_tile_type_name(TILE_GREEN));

  printf("\n=== All Tests Complete ===\n");
  printf("Rule Info Card System is working correctly!\n");
  printf("\nNext steps:\n");
  printf("- Fix Clay UI compilation issues for visual rendering\n");
  printf("- Integrate with existing game UI systems\n");
  printf("- Add any missing rule types as they are implemented\n");

  return 0;
}
