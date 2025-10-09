#include "game/rule_system.h"
#include "ui.h"
#include "ui/rule_info_card.h"
#include <stdio.h>

/* ============================================================================
 * Rule Info Card Demo
 *
 * This file demonstrates how to use the rule info card system to create
 * visual representations of different rule types.
 * ============================================================================
 */

/**
 * @brief Create a sample neighbor bonus rule
 */
rule_t create_sample_neighbor_bonus_rule(void) {
  rule_t rule = {0};

  rule.id = 1;
  rule.priority = RULE_PRIORITY_PRODUCTION;
  rule.scope = RULE_SCOPE_NEIGHBORS;
  rule.target = RULE_TARGET_PRODUCTION;

  rule.condition_type = RULE_CONDITION_NEIGHBOR_COUNT;
  rule.condition_params.neighbor_count.neighbor_type = TILE_YELLOW;
  rule.condition_params.neighbor_count.min_count = 1;
  rule.condition_params.neighbor_count.max_count = 6;
  rule.condition_params.neighbor_count.range = 1;

  rule.effect_type = RULE_EFFECT_ADD_SCALED;
  rule.effect_params.scaled.base_value = 0.0f;
  rule.effect_params.scaled.scale_factor = 2.0f;

  rule.source_cell = (grid_cell_t){.type = GRID_TYPE_HEXAGON,
                                   .coord.hex = {.q = 0, .r = 0, .s = 0}};
  rule.affected_range = 1;
  rule.is_active = true;
  rule.needs_recalc = false;
  rule.cache_friendly = true;

  return rule;
}

/**
 * @brief Create a sample type override rule
 */
rule_t create_sample_type_override_rule(void) {
  rule_t rule = {0};

  rule.id = 2;
  rule.priority = RULE_PRIORITY_PERCEPTION;
  rule.scope = RULE_SCOPE_RANGE;
  rule.target = RULE_TARGET_TYPE_OVERRIDE;

  rule.condition_type = RULE_CONDITION_ALWAYS;

  rule.effect_type = RULE_EFFECT_OVERRIDE_TYPE;
  rule.effect_params.override_type = TILE_MAGENTA;

  rule.source_cell = (grid_cell_t){.type = GRID_TYPE_HEXAGON,
                                   .coord.hex = {.q = 1, .r = 0, .s = -1}};
  rule.affected_range = 2;
  rule.is_active = true;
  rule.needs_recalc = false;
  rule.cache_friendly = false;

  return rule;
}

/**
 * @brief Create a sample global production modifier rule
 */
rule_t create_sample_global_modifier_rule(void) {
  rule_t rule = {0};

  rule.id = 3;
  rule.priority = RULE_PRIORITY_PRODUCTION;
  rule.scope = RULE_SCOPE_TYPE_GLOBAL;
  rule.target = RULE_TARGET_PRODUCTION;

  rule.condition_type = RULE_CONDITION_BOARD_COUNT;
  rule.condition_params.board_count.target_type = TILE_GREEN;
  rule.condition_params.board_count.min_count = 5;
  rule.condition_params.board_count.max_count = 999;

  rule.effect_type = RULE_EFFECT_MULTIPLY;
  rule.effect_params.value = 1.5f;

  rule.source_cell = (grid_cell_t){.type = GRID_TYPE_HEXAGON,
                                   .coord.hex = {.q = -1, .r = 1, .s = 0}};
  rule.affected_range = 0; // Global
  rule.is_active = true;
  rule.needs_recalc = true;
  rule.cache_friendly = false;

  return rule;
}

/**
 * @brief Create a sample range modifier rule
 */
rule_t create_sample_range_modifier_rule(void) {
  rule_t rule = {0};

  rule.id = 4;
  rule.priority = RULE_PRIORITY_RANGE_MODIFY;
  rule.scope = RULE_SCOPE_SELF;
  rule.target = RULE_TARGET_RANGE;

  rule.condition_type = RULE_CONDITION_POOL_SIZE;
  rule.condition_params.pool_size.min_size = 3;
  rule.condition_params.pool_size.max_size = 999;

  rule.effect_type = RULE_EFFECT_MODIFY_RANGE;
  rule.effect_params.range_delta = 1;

  rule.source_cell = (grid_cell_t){.type = GRID_TYPE_HEXAGON,
                                   .coord.hex = {.q = 2, .r = -1, .s = -1}};
  rule.affected_range = 0; // Self only
  rule.is_active = true;
  rule.needs_recalc = false;
  rule.cache_friendly = true;

  return rule;
}

/**
 * @brief Demo function showing different rule card configurations
 */
void demo_rule_card_configurations(void) {
  printf("=== Rule Info Card Demo ===\n\n");

  // Create sample rules
  rule_t neighbor_rule = create_sample_neighbor_bonus_rule();
  rule_t override_rule = create_sample_type_override_rule();
  rule_t global_rule = create_sample_global_modifier_rule();
  rule_t range_rule = create_sample_range_modifier_rule();

  // Demo 1: Full configuration cards
  printf("Demo 1: Full Rule Cards\n");
  printf("-----------------------\n");

  // Note: In a real implementation, these would be called within a Clay UI
  // context Here we just demonstrate the API usage

  ui_render_rule_info_card(&neighbor_rule, &RULE_CARD_CONFIG_FULL,
                           CLAY_ID("neighbor_card"));
  ui_render_rule_info_card(&override_rule, &RULE_CARD_CONFIG_FULL,
                           CLAY_ID("override_card"));
  ui_render_rule_info_card(&global_rule, &RULE_CARD_CONFIG_FULL,
                           CLAY_ID("global_card"));
  ui_render_rule_info_card(&range_rule, &RULE_CARD_CONFIG_FULL,
                           CLAY_ID("range_card"));

  // Demo 2: Compact configuration cards
  printf("\nDemo 2: Compact Rule Cards\n");
  printf("--------------------------\n");

  ui_render_rule_info_card(&neighbor_rule, &RULE_CARD_CONFIG_COMPACT,
                           CLAY_ID("neighbor_compact"));
  ui_render_rule_info_card(&override_rule, &RULE_CARD_CONFIG_COMPACT,
                           CLAY_ID("override_compact"));
  ui_render_rule_info_card(&global_rule, &RULE_CARD_CONFIG_COMPACT,
                           CLAY_ID("global_compact"));
  ui_render_rule_info_card(&range_rule, &RULE_CARD_CONFIG_COMPACT,
                           CLAY_ID("range_compact"));

  // Demo 3: Minimal configuration cards
  printf("\nDemo 3: Minimal Rule Cards\n");
  printf("--------------------------\n");

  ui_render_rule_info_card(&neighbor_rule, &RULE_CARD_CONFIG_MINIMAL,
                           CLAY_ID("neighbor_minimal"));
  ui_render_rule_info_card(&override_rule, &RULE_CARD_CONFIG_MINIMAL,
                           CLAY_ID("override_minimal"));
  ui_render_rule_info_card(&global_rule, &RULE_CARD_CONFIG_MINIMAL,
                           CLAY_ID("global_minimal"));
  ui_render_rule_info_card(&range_rule, &RULE_CARD_CONFIG_MINIMAL,
                           CLAY_ID("range_minimal"));

  // Demo 4: Custom configuration
  printf("\nDemo 4: Custom Configuration\n");
  printf("----------------------------\n");

  rule_card_config_t custom_config =
    rule_card_config_create(true, // show_icons
                            true, // show_descriptions
                            false // compact
    );
  custom_config.card_width = 300.0f;
  custom_config.max_height = 180.0f;

  ui_render_rule_info_card(&neighbor_rule, &custom_config,
                           CLAY_ID("neighbor_custom"));

  // Demo 5: Rule list
  printf("\nDemo 5: Rule List\n");
  printf("------------------\n");

  const rule_t *rules[] = {&neighbor_rule, &override_rule, &global_rule,
                           &range_rule};
  uint32_t rule_count = 4;

  // Sort by priority first
  rule_sort_by_priority(rules, rule_count);

  ui_render_rule_info_list(rules, rule_count, &RULE_CARD_CONFIG_COMPACT,
                           CLAY_ID("rule_list"));
}

/**
 * @brief Demo function showing rule analysis features
 */
void demo_rule_analysis(void) {
  printf("\n=== Rule Analysis Demo ===\n\n");

  rule_t neighbor_rule = create_sample_neighbor_bonus_rule();
  rule_t override_rule = create_sample_type_override_rule();

  // Demo rule display info
  rule_display_info_t neighbor_info = rule_get_display_info(&neighbor_rule);
  rule_display_info_t override_info = rule_get_display_info(&override_rule);

  printf("Neighbor Rule Info:\n");
  printf("  Type Icon: %s\n", neighbor_info.type_icon);
  printf("  Name: %s\n", neighbor_info.name);
  printf("  Description: %s\n", neighbor_info.description);
  printf("  Priority: %u\n", neighbor_info.display_priority);

  printf("\nOverride Rule Info:\n");
  printf("  Type Icon: %s\n", override_info.type_icon);
  printf("  Name: %s\n", override_info.name);
  printf("  Description: %s\n", override_info.description);
  printf("  Priority: %u\n", override_info.display_priority);

  // Demo text generation
  char desc_buffer[256];
  char condition_buffer[128];
  char effect_buffer[128];

  printf("\nNeighbor Rule Text Generation:\n");

  int desc_len =
    rule_generate_description(&neighbor_rule, desc_buffer, sizeof(desc_buffer));
  printf("  Description: %s (length: %d)\n", desc_buffer, desc_len);

  int cond_len = rule_generate_condition_text(&neighbor_rule, condition_buffer,
                                              sizeof(condition_buffer));
  printf("  Condition: %s (length: %d)\n", condition_buffer, cond_len);

  int effect_len = rule_generate_effect_text(&neighbor_rule, effect_buffer,
                                             sizeof(effect_buffer));
  printf("  Effect: %s (length: %d)\n", effect_buffer, effect_len);

  // Demo rule properties
  printf("\nRule Properties:\n");
  printf("  Neighbor rule is visually important: %s\n",
         rule_is_visually_important(&neighbor_rule) ? "Yes" : "No");
  printf("  Override rule is visually important: %s\n",
         rule_is_visually_important(&override_rule) ? "Yes" : "No");

  printf("  Neighbor rule affects YELLOW tiles: %s\n",
         rule_affects_tile_type(&neighbor_rule, TILE_YELLOW) ? "Yes" : "No");
  printf("  Override rule affects YELLOW tiles: %s\n",
         rule_affects_tile_type(&override_rule, TILE_YELLOW) ? "Yes" : "No");
}

/**
 * @brief Demo function showing icon and color usage
 */
void demo_icons_and_colors(void) {
  printf("\n=== Icons and Colors Demo ===\n\n");

  // Demo scope icons
  printf("Scope Icons:\n");
  printf("  SELF: %s\n", rule_get_scope_icon(RULE_SCOPE_SELF));
  printf("  NEIGHBORS: %s\n", rule_get_scope_icon(RULE_SCOPE_NEIGHBORS));
  printf("  RANGE: %s\n", rule_get_scope_icon(RULE_SCOPE_RANGE));
  printf("  POOL: %s\n", rule_get_scope_icon(RULE_SCOPE_POOL));
  printf("  TYPE_GLOBAL: %s\n", rule_get_scope_icon(RULE_SCOPE_TYPE_GLOBAL));
  printf("  BOARD_GLOBAL: %s\n", rule_get_scope_icon(RULE_SCOPE_BOARD_GLOBAL));

  // Demo target icons
  printf("\nTarget Icons:\n");
  printf("  PRODUCTION: %s\n", rule_get_target_icon(RULE_TARGET_PRODUCTION));
  printf("  RANGE: %s\n", rule_get_target_icon(RULE_TARGET_RANGE));
  printf("  POOL_MODIFIER: %s\n",
         rule_get_target_icon(RULE_TARGET_POOL_MODIFIER));
  printf("  TYPE_OVERRIDE: %s\n",
         rule_get_target_icon(RULE_TARGET_TYPE_OVERRIDE));
  printf("  PLACEMENT_COST: %s\n",
         rule_get_target_icon(RULE_TARGET_PLACEMENT_COST));
  printf("  MOVEMENT_RANGE: %s\n",
         rule_get_target_icon(RULE_TARGET_MOVEMENT_RANGE));

  // Demo condition icons
  printf("\nCondition Icons:\n");
  printf("  ALWAYS: %s\n", rule_get_condition_icon(RULE_CONDITION_ALWAYS));
  printf("  SELF_TYPE: %s\n",
         rule_get_condition_icon(RULE_CONDITION_SELF_TYPE));
  printf("  NEIGHBOR_COUNT: %s\n",
         rule_get_condition_icon(RULE_CONDITION_NEIGHBOR_COUNT));
  printf("  POOL_SIZE: %s\n",
         rule_get_condition_icon(RULE_CONDITION_POOL_SIZE));
  printf("  BOARD_COUNT: %s\n",
         rule_get_condition_icon(RULE_CONDITION_BOARD_COUNT));
  printf("  PRODUCTION_THRESHOLD: %s\n",
         rule_get_condition_icon(RULE_CONDITION_PRODUCTION_THRESHOLD));

  // Demo effect icons
  printf("\nEffect Icons:\n");
  printf("  ADD_FLAT: %s\n", rule_get_effect_icon(RULE_EFFECT_ADD_FLAT));
  printf("  ADD_SCALED: %s\n", rule_get_effect_icon(RULE_EFFECT_ADD_SCALED));
  printf("  MULTIPLY: %s\n", rule_get_effect_icon(RULE_EFFECT_MULTIPLY));
  printf("  SET_VALUE: %s\n", rule_get_effect_icon(RULE_EFFECT_SET_VALUE));
  printf("  OVERRIDE_TYPE: %s\n",
         rule_get_effect_icon(RULE_EFFECT_OVERRIDE_TYPE));
  printf("  MODIFY_RANGE: %s\n",
         rule_get_effect_icon(RULE_EFFECT_MODIFY_RANGE));

  // Demo tile type colors and names
  printf("\nTile Type Info:\n");
  tile_type_t types[] = {TILE_MAGENTA, TILE_CYAN, TILE_YELLOW, TILE_GREEN,
                         TILE_EMPTY};
  for (int i = 0; i < 5; i++) {
    Clay_Color color = rule_get_tile_type_color(types[i]);
    const char *name = rule_get_tile_type_name(types[i]);
    printf("  %s: RGB(%d, %d, %d)\n", name, color.r, color.g, color.b);
  }
}

/**
 * @brief Main demo function - call this to run all demos
 */
void run_rule_info_card_demo(void) {
  printf("Starting Rule Info Card System Demo...\n");
  printf("=====================================\n");

  // Note: In a real application, you would initialize the UI system first:
  // UI_Context ui_ctx = ui_init(screen_width, screen_height);
  // ui_load_fonts();
  // ui_init_text_configs();

  demo_rule_analysis();
  demo_icons_and_colors();
  demo_rule_card_configurations();

  printf("\n=== Demo Complete ===\n");
  printf("To use these rule cards in your game:\n");
  printf("1. Initialize the UI system with ui_init()\n");
  printf("2. Create rule_t structures with your game rules\n");
  printf("3. Call ui_render_rule_info_card() within a Clay UI context\n");
  printf("4. Use different configurations for different display needs\n");

  // Note: In a real application, you would cleanup:
  // ui_shutdown(&ui_ctx);
}
