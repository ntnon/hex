#include "game/game.h"
#include "game/rule_manager.h"
#include "ui.h"
#include "ui/rule_info_card.h"
#include <raylib.h>
#include <stdio.h>
#include <string.h>

// Forward declarations for functions defined later in this file
void ui_render_tile_rules_section(const rule_manager_t *rule_manager,
                                  const tile_t *tile);
void ui_render_sample_tile_rules(const tile_t *tile);
void ui_render_compact_rule_info(const rule_t *rule, const char *id_suffix);

/* ============================================================================
 * Rule Card Integration Example
 *
 * This file demonstrates how to integrate rule info cards with the existing
 * hexhex game UI system, specifically enhancing tile info cards with rule
 * information.
 * ============================================================================
 */

/**
 * @brief Enhanced tile info card that includes active rules
 *
 * This function extends the existing tile info card to display rules
 * that are currently affecting the hovered tile.
 */
void ui_build_enhanced_tile_info_card(game_t *game, Vector2 mouse_pos) {
  if (!game->should_show_tile_info || !game->hovered_tile) {
    return;
  }

  tile_t *tile = game->hovered_tile;
  pool_t *pool = pool_map_get_pool_by_tile(game->board->pools, tile);

  // Position the card (same logic as original)
  float card_width = 280;  // Slightly wider for rule info
  float card_height = 200; // Taller for rule info
  float screen_width = GetScreenWidth();
  float screen_height = GetScreenHeight();

  float card_x = mouse_pos.x + 20;
  float card_y = mouse_pos.y - card_height / 2;

  if (card_x + card_width > screen_width) {
    card_x = mouse_pos.x - card_width - 20;
  }
  if (card_y < 0) {
    card_y = 10;
  } else if (card_y + card_height > screen_height) {
    card_y = screen_height - card_height - 10;
  }

  CLAY({
    .id = UI_ID_TILE_INFO_CARD,
    .floating = {.attachTo = CLAY_ATTACH_TO_ROOT,
                 .offset = {.x = (int)card_x, .y = (int)card_y},
                 .zIndex = 1000,
                 .pointerCaptureMode = CLAY_POINTER_CAPTURE_MODE_PASSTHROUGH},
    .layout = {.sizing = {.width = CLAY_SIZING_FIT(),
                          .height = CLAY_SIZING_FIT()},
               .padding = CLAY_PADDING_ALL(12),
               .childGap = 8,
               .layoutDirection = CLAY_TOP_TO_BOTTOM},
  }) {

    // Original tile information card
    CLAY({.layout = {.sizing = {.width = CLAY_SIZING_GROW(),
                                .height = CLAY_SIZING_FIT()},
                     .padding = CLAY_PADDING_ALL(12),
                     .childGap = 8,
                     .layoutDirection = CLAY_TOP_TO_BOTTOM},
          .backgroundColor = (Clay_Color){40, 40, 40, 240},
          .cornerRadius = CLAY_CORNER_RADIUS(6),
          .border = {.color = (Clay_Color){80, 80, 80, 255}, .width = 1}}) {

      // Tile type and basic info
      const char *tile_name = rule_get_tile_type_name(tile->data.type);
      CLAY_TEXT(CLAY_STRING(tile_name), &TEXT_CONFIG_MEDIUM);

      // Tile value
      static char value_text[32];
      snprintf(value_text, sizeof(value_text), "Value: %d", tile->data.value);
      Clay_String value_string = {.chars = value_text,
                                  .length = strlen(value_text)};
      CLAY_TEXT(value_string, &TEXT_CONFIG_MEDIUM);

      // Effective production (considering rules)
      if (game->rule_manager) {
        float effective_production =
          rule_manager_calculate_tile_production(game->rule_manager, tile);
        static char production_text[32];
        snprintf(production_text, sizeof(production_text), "Production: %.1f",
                 effective_production);
        Clay_String production_string = {.chars = production_text,
                                         .length = strlen(production_text)};
        CLAY_TEXT(production_string, &TEXT_CONFIG_MEDIUM);
      }

      // Effective range (considering rules)
      if (game->rule_manager) {
        uint8_t effective_range =
          rule_manager_calculate_tile_range(game->rule_manager, tile);
        static char range_text[32];
        snprintf(range_text, sizeof(range_text), "Range: %d", effective_range);
        Clay_String range_string = {.chars = range_text,
                                    .length = strlen(range_text)};
        CLAY_TEXT(range_string, &TEXT_CONFIG_MEDIUM);
      }
    }

    // Pool information (if applicable)
    if (pool) {
      CLAY({.layout = {.sizing = {.width = CLAY_SIZING_GROW(),
                                  .height = CLAY_SIZING_FIT()},
                       .padding = CLAY_PADDING_ALL(12),
                       .childGap = 8,
                       .layoutDirection = CLAY_TOP_TO_BOTTOM},
            .backgroundColor = (Clay_Color){40, 40, 40, 240},
            .cornerRadius = CLAY_CORNER_RADIUS(6),
            .border = {.color = (Clay_Color){80, 80, 80, 255}, .width = 1}}) {

        CLAY_TEXT(CLAY_STRING("Pool Info"), &TEXT_CONFIG_MEDIUM);

        static char pool_text[32];
        snprintf(pool_text, sizeof(pool_text), "Tiles: %u",
                 pool_tile_score(pool));
        Clay_String pool_score = {.chars = pool_text,
                                  .length = strlen(pool_text)};
        CLAY_TEXT(pool_score, &TEXT_CONFIG_MEDIUM);
      }
    }

    // NEW: Rules affecting this tile
    if (game->rule_manager) {
      ui_render_tile_rules_section(game->rule_manager, tile);
    }
  }
}

/**
 * @brief Render rules section for a tile
 *
 * This function renders a section showing all rules that are currently
 * affecting the given tile.
 */
void ui_render_tile_rules_section(const rule_manager_t *rule_manager,
                                  const tile_t *tile) {
  if (!rule_manager || !tile) {
    return;
  }

  // For this example, we'll create some sample rules since we don't have
  // direct access to the rule manager's internal rule list
  // In a real implementation, you'd query the rule manager for rules affecting
  // this tile

  CLAY({.layout = {.sizing = {.width = CLAY_SIZING_GROW(),
                              .height = CLAY_SIZING_FIT()},
                   .padding = CLAY_PADDING_ALL(12),
                   .childGap = 6,
                   .layoutDirection = CLAY_TOP_TO_BOTTOM},
        .backgroundColor = (Clay_Color){50, 50, 80, 200},
        .cornerRadius = CLAY_CORNER_RADIUS(4),
        .border = {.color = (Clay_Color){100, 100, 150, 255}, .width = 1}}) {

    // Section header
    CLAY({.layout = {.sizing = {.width = CLAY_SIZING_GROW(),
                                .height = CLAY_SIZING_FIT()},
                     .layoutDirection = CLAY_LEFT_TO_RIGHT,
                     .childAlignment = {.y = CLAY_ALIGN_Y_CENTER},
                     .childGap = 6}}) {
      CLAY_TEXT(CLAY_STRING("⚙️"), &TEXT_CONFIG_MEDIUM);
      CLAY_TEXT(CLAY_STRING("Active Rules"), &TEXT_CONFIG_MEDIUM);
    }

    // Example rules (in real implementation, query from rule_manager)
    ui_render_sample_tile_rules(tile);
  }
}

/**
 * @brief Render sample rules for demonstration
 *
 * In a real implementation, this would query the rule manager for actual
 * rules affecting the tile and render them using the rule info card system.
 */
void ui_render_sample_tile_rules(const tile_t *tile) {
  // Create sample rules based on tile type for demonstration

  if (tile->data.type == TILE_YELLOW) {
    // Example: Yellow tiles get neighbor bonus
    rule_t sample_rule = {0};
    sample_rule.scope = RULE_SCOPE_NEIGHBORS;
    sample_rule.target = RULE_TARGET_PRODUCTION;
    sample_rule.condition_type = RULE_CONDITION_NEIGHBOR_COUNT;
    sample_rule.condition_params.neighbor_count.neighbor_type = TILE_GREEN;
    sample_rule.condition_params.neighbor_count.min_count = 1;
    sample_rule.condition_params.neighbor_count.max_count = 6;
    sample_rule.effect_type = RULE_EFFECT_ADD_FLAT;
    sample_rule.effect_params.value = 2.0f;

    ui_render_compact_rule_info(&sample_rule, "yellow_neighbor_bonus");
  }

  if (tile->data.value > 5) {
    // Example: High-value tiles get range boost
    rule_t range_rule = {0};
    range_rule.scope = RULE_SCOPE_SELF;
    range_rule.target = RULE_TARGET_RANGE;
    range_rule.condition_type = RULE_CONDITION_ALWAYS;
    range_rule.effect_type = RULE_EFFECT_MODIFY_RANGE;
    range_rule.effect_params.range_delta = 1;

    ui_render_compact_rule_info(&range_rule, "high_value_range");
  }

  // If no rules apply
  static bool has_rules = false;
  if (tile->data.type == TILE_YELLOW || tile->data.value > 5) {
    has_rules = true;
  }

  if (!has_rules) {
    CLAY_TEXT(CLAY_STRING("No active rules"), &TEXT_CONFIG_MEDIUM);
  }
}

/**
 * @brief Render a compact rule info display
 *
 * This creates a single-line compact representation of a rule
 * suitable for inclusion in tile info cards.
 */
void ui_render_compact_rule_info(const rule_t *rule, const char *id_suffix) {
  if (!rule) {
    return;
  }

  rule_display_info_t info = rule_get_display_info(rule);

  CLAY({.layout = {.sizing = {.width = CLAY_SIZING_GROW(),
                              .height = CLAY_SIZING_FIT()},
                   .layoutDirection = CLAY_LEFT_TO_RIGHT,
                   .childAlignment = {.y = CLAY_ALIGN_Y_CENTER},
                   .childGap = 6,
                   .padding = CLAY_PADDING_ALL(4)},
        .backgroundColor = (Clay_Color){0, 0, 0, 30},
        .cornerRadius = CLAY_CORNER_RADIUS(3)}) {

    // Rule icon
    CLAY_TEXT(CLAY_STRING(info.type_icon), &TEXT_CONFIG_MEDIUM);

    // Effect text
    char effect_text[64];
    rule_generate_effect_text(rule, effect_text, sizeof(effect_text));
    Clay_String effect_str = {.chars = effect_text,
                              .length = strlen(effect_text)};
    CLAY_TEXT(effect_str, &TEXT_CONFIG_MEDIUM);

    // Scope indicator
    CLAY_TEXT(CLAY_STRING(info.scope_icon), &TEXT_CONFIG_MEDIUM);
  }
}

/**
 * @brief Create a rules showcase window
 *
 * This function creates a standalone window that shows examples of different
 * rule types and their visual representations.
 */
void ui_create_rules_showcase_window() {
  CLAY({.id = CLAY_ID("rules_showcase_window"),
        .floating = {.attachTo = CLAY_ATTACH_TO_ROOT,
                     .offset = {.x = 50, .y = 50},
                     .zIndex = 500},
        .layout = {.sizing = {.width = CLAY_SIZING_FIXED(600),
                              .height = CLAY_SIZING_FIT()},
                   .padding = CLAY_PADDING_ALL(20),
                   .childGap = 16,
                   .layoutDirection = CLAY_TOP_TO_BOTTOM},
        .backgroundColor = (Clay_Color){30, 30, 30, 250},
        .cornerRadius = CLAY_CORNER_RADIUS(10),
        .border = {.color = (Clay_Color){100, 100, 100, 255}, .width = 2}}) {

    // Title
    CLAY({.layout = {.sizing = {.width = CLAY_SIZING_GROW(),
                                .height = CLAY_SIZING_FIT()},
                     .childAlignment = {.x = CLAY_ALIGN_X_CENTER}}}) {
      CLAY_TEXT(CLAY_STRING("Rule System Showcase"), &TEXT_CONFIG_LARGE);
    }

    // Rules grid
    CLAY({.layout = {.sizing = {.width = CLAY_SIZING_GROW(),
                                .height = CLAY_SIZING_FIT()},
                     .layoutDirection = CLAY_LEFT_TO_RIGHT,
                     .childGap = 12}}) {

      // Column 1: Neighbor rules
      CLAY({.layout = {.sizing = {.width = CLAY_SIZING_GROW(),
                                  .height = CLAY_SIZING_FIT()},
                       .layoutDirection = CLAY_TOP_TO_BOTTOM,
                       .childGap = 8}}) {

        CLAY_TEXT(CLAY_STRING("Neighbor Rules"), &TEXT_CONFIG_MEDIUM);

        // Create sample neighbor rule
        rule_t neighbor_rule = {0};
        neighbor_rule.scope = RULE_SCOPE_NEIGHBORS;
        neighbor_rule.target = RULE_TARGET_PRODUCTION;
        neighbor_rule.condition_type = RULE_CONDITION_NEIGHBOR_COUNT;
        neighbor_rule.condition_params.neighbor_count.neighbor_type =
          TILE_YELLOW;
        neighbor_rule.condition_params.neighbor_count.min_count = 2;
        neighbor_rule.condition_params.neighbor_count.max_count = 2;
        neighbor_rule.effect_type = RULE_EFFECT_ADD_FLAT;
        neighbor_rule.effect_params.value = 3.0f;

        ui_render_compact_rule_info(&neighbor_rule, "showcase_neighbor");
      }

      // Column 2: Global rules
      CLAY({.layout = {.sizing = {.width = CLAY_SIZING_GROW(),
                                  .height = CLAY_SIZING_FIT()},
                       .layoutDirection = CLAY_TOP_TO_BOTTOM,
                       .childGap = 8}}) {

        CLAY_TEXT(CLAY_STRING("Global Rules"), &TEXT_CONFIG_MEDIUM);

        // Create sample global rule
        rule_t global_rule = {0};
        global_rule.scope = RULE_SCOPE_BOARD_GLOBAL;
        global_rule.target = RULE_TARGET_PRODUCTION;
        global_rule.condition_type = RULE_CONDITION_BOARD_COUNT;
        global_rule.condition_params.board_count.target_type = TILE_GREEN;
        global_rule.condition_params.board_count.min_count = 5;
        global_rule.condition_params.board_count.max_count = 999;
        global_rule.effect_type = RULE_EFFECT_MULTIPLY;
        global_rule.effect_params.value = 1.5f;

        ui_render_compact_rule_info(&global_rule, "showcase_global");
      }

      // Column 3: Special rules
      CLAY({.layout = {.sizing = {.width = CLAY_SIZING_GROW(),
                                  .height = CLAY_SIZING_FIT()},
                       .layoutDirection = CLAY_TOP_TO_BOTTOM,
                       .childGap = 8}}) {

        CLAY_TEXT(CLAY_STRING("Special Rules"), &TEXT_CONFIG_MEDIUM);

        // Create sample type override rule
        rule_t override_rule = {0};
        override_rule.scope = RULE_SCOPE_RANGE;
        override_rule.target = RULE_TARGET_TYPE_OVERRIDE;
        override_rule.condition_type = RULE_CONDITION_ALWAYS;
        override_rule.effect_type = RULE_EFFECT_OVERRIDE_TYPE;
        override_rule.effect_params.override_type = TILE_MAGENTA;
        override_rule.affected_range = 2;

        ui_render_compact_rule_info(&override_rule, "showcase_override");
      }
    }

    // Instructions
    CLAY({.layout = {.sizing = {.width = CLAY_SIZING_GROW(),
                                .height = CLAY_SIZING_FIT()},
                     .childAlignment = {.x = CLAY_ALIGN_X_CENTER},
                     .padding = CLAY_PADDING_ALL(8)},
          .backgroundColor = (Clay_Color){0, 0, 0, 50},
          .cornerRadius = CLAY_CORNER_RADIUS(4)}) {

      CLAY_TEXT(CLAY_STRING("Hover over tiles to see their active rules"),
                &TEXT_CONFIG_MEDIUM);
    }
  }
}

/**
 * @brief Integration helper: Replace existing tile info card
 *
 * Call this function instead of the original ui_build_tile_info_card
 * to get enhanced functionality with rule information.
 */
void ui_integrate_enhanced_tile_info(game_t *game, Vector2 mouse_pos) {
  // Check if rule manager is available
  if (game->rule_manager) {
    // Use enhanced version with rules
    ui_build_enhanced_tile_info_card(game, mouse_pos);
  } else {
    // Fallback to original implementation
    ui_build_tile_info_card(game, mouse_pos);
  }
}

/**
 * @brief Example of how to modify existing game loop integration
 */
void example_game_loop_integration() {
  /*
   * To integrate the enhanced tile info cards into your game:
   *
   * 1. In your main game rendering loop, replace calls to:
   *    ui_build_tile_info_card(game, mouse_pos);
   *
   *    With:
   *    ui_integrate_enhanced_tile_info(game, mouse_pos);
   *
   * 2. Make sure your game_t structure has a rule_manager field:
   *    game_t *game = ...;
   *    if (!game->rule_manager) {
   *        game->rule_manager = rule_manager_create();
   *        rule_manager_init(game->rule_manager, game->board, MAX_TILES);
   *    }
   *
   * 3. When tiles are placed/removed, notify the rule manager:
   *    rule_manager_on_tile_placed(game->rule_manager, placed_tile);
   *    rule_manager_on_tile_removed(game->rule_manager, removed_cell);
   *
   * 4. Optional: Add a debug key to show the rules showcase window:
   *    if (IsKeyPressed(KEY_F1)) {
   *        ui_create_rules_showcase_window();
   *    }
   */
}

/**
 * @brief Function declarations for integration
 *
 * Add these to your header files for proper integration:
 */
/*
// In ui.h or game_ui.h:
void ui_build_enhanced_tile_info_card(game_t *game, Vector2 mouse_pos);
void ui_render_tile_rules_section(const rule_manager_t *rule_manager, const
tile_t *tile); void ui_render_compact_rule_info(const rule_t *rule, const char
*id_suffix); void ui_create_rules_showcase_window(void); void
ui_integrate_enhanced_tile_info(game_t *game, Vector2 mouse_pos);

// Usage examples:
// - Replace ui_build_tile_info_card() calls with
ui_integrate_enhanced_tile_info()
// - Call ui_create_rules_showcase_window() for debugging/showcase
// - Use ui_render_compact_rule_info() in other UI contexts where you want to
show rule info
*/
