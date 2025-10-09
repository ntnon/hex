#include "ui/rule_info_card.h"
#include <stdio.h>
#include <string.h>

/* ============================================================================
 * Configuration Constants
 * ============================================================================
 */

const rule_card_config_t RULE_CARD_CONFIG_FULL = {.show_icons = true,
                                                  .show_descriptions = true,
                                                  .show_conditions = true,
                                                  .show_effects = true,
                                                  .compact_layout = false,
                                                  .card_width = 280.0f,
                                                  .max_height = 200.0f};

const rule_card_config_t RULE_CARD_CONFIG_COMPACT = {.show_icons = true,
                                                     .show_descriptions = false,
                                                     .show_conditions = false,
                                                     .show_effects = true,
                                                     .compact_layout = true,
                                                     .card_width = 220.0f,
                                                     .max_height = 100.0f};

const rule_card_config_t RULE_CARD_CONFIG_MINIMAL = {.show_icons = true,
                                                     .show_descriptions = false,
                                                     .show_conditions = false,
                                                     .show_effects = false,
                                                     .compact_layout = true,
                                                     .card_width = 150.0f,
                                                     .max_height = 60.0f};

/* ============================================================================
 * Core Rule Display Functions
 * ============================================================================
 */

rule_display_info_t rule_get_display_info(const rule_t *rule) {
  rule_display_info_t info = {0};

  if (!rule) {
    info.type_icon = "❓";
    info.name = "Invalid Rule";
    info.description = "Rule data is invalid";
    info.primary_color = (Clay_Color){100, 100, 100, 255};
    info.accent_color = (Clay_Color){150, 150, 150, 255};
    return info;
  }

  // Set scope and target icons
  info.scope_icon = rule_get_scope_icon(rule->scope);
  info.target_icon = rule_get_target_icon(rule->target);

  // Determine rule type and styling based on scope/target combination
  if (rule->scope == RULE_SCOPE_NEIGHBORS &&
      rule->target == RULE_TARGET_PRODUCTION) {
    info.type_icon = "👥";
    info.name = "Neighbor Bonus";
    info.description = "Gains production from nearby tiles";
    info.primary_color = (Clay_Color){76, 175, 80, 220}; // Green
    info.accent_color = (Clay_Color){129, 199, 132, 255};
  } else if (rule->target == RULE_TARGET_RANGE) {
    info.type_icon = "📏";
    info.name = "Range Modifier";
    info.description = "Changes interaction range";
    info.primary_color = (Clay_Color){33, 150, 243, 220}; // Blue
    info.accent_color = (Clay_Color){100, 181, 246, 255};
  } else if (rule->target == RULE_TARGET_TYPE_OVERRIDE) {
    info.type_icon = "🎭";
    info.name = "Type Override";
    info.description = "Changes how tiles appear";
    info.primary_color = (Clay_Color){255, 152, 0, 220}; // Orange
    info.accent_color = (Clay_Color){255, 183, 77, 255};
  } else if (rule->scope == RULE_SCOPE_POOL) {
    info.type_icon = "🌊";
    info.name = "Pool Effect";
    info.description = "Affects entire pool";
    info.primary_color = (Clay_Color){156, 39, 176, 220}; // Purple
    info.accent_color = (Clay_Color){186, 104, 200, 255};
  } else if (rule->scope == RULE_SCOPE_BOARD_GLOBAL ||
             rule->scope == RULE_SCOPE_TYPE_GLOBAL) {
    info.type_icon = "🌍";
    info.name = "Global Effect";
    info.description = "Affects entire board";
    info.primary_color = (Clay_Color){255, 193, 7, 220}; // Amber
    info.accent_color = (Clay_Color){255, 213, 79, 255};
  } else {
    info.type_icon = "⚙️";
    info.name = "Rule Effect";
    info.description = "Game rule";
    info.primary_color = (Clay_Color){96, 125, 139, 220}; // Blue Grey
    info.accent_color = (Clay_Color){144, 164, 174, 255};
  }

  info.display_priority = rule_get_display_priority(rule);
  return info;
}

void ui_render_rule_info_card(const rule_t *rule,
                              const rule_card_config_t *config,
                              Clay_ElementId element_id) {
  if (!rule || !config) {
    return;
  }

  // For now, output rule information to console for debugging
  rule_display_info_t info = rule_get_display_info(rule);

  printf("=== Rule Info Card ===\n");
  printf("Icon: %s Name: %s\n", info.type_icon, info.name);
  printf("Description: %s\n", info.description);

  if (config->show_effects) {
    char effect_text[128];
    int effect_len =
      rule_generate_effect_text(rule, effect_text, sizeof(effect_text));
    if (effect_len > 0) {
      printf("Effect: %s %s\n", info.target_icon, effect_text);
    }
  }

  if (config->show_conditions &&
      rule->condition_type != RULE_CONDITION_ALWAYS) {
    char condition_text[128];
    int condition_len = rule_generate_condition_text(rule, condition_text,
                                                     sizeof(condition_text));
    if (condition_len > 0) {
      printf("Condition: %s %s\n",
             rule_get_condition_icon(rule->condition_type), condition_text);
    }
  }

  printf("Priority: %u\n", info.display_priority);
  printf("====================\n\n");

  // Clay UI implementation will be added once compilation issues are resolved
  // For now, we just use the element_id as a debug identifier
  printf("Element ID used for rendering\n");
  (void)element_id;
}

void ui_render_rule_info_list(const rule_t **rules, uint32_t rule_count,
                              const rule_card_config_t *config,
                              Clay_ElementId container_id) {
  if (!rules || rule_count == 0 || !config) {
    return;
  }

  printf("=== Rule Info List (%u rules) ===\n", rule_count);

  for (uint32_t i = 0; i < rule_count; i++) {
    if (rules[i]) {
      printf("Rule %u:\n", i + 1);
      char id_str[64];
      snprintf(id_str, sizeof(id_str), "rule_card_%u", i);
      // Create a dummy Clay_ElementId for now
      Clay_ElementId dummy_id = {
        .id = i, .offset = 0, .baseId = 0, .stringId = {0}};
      ui_render_rule_info_card(rules[i], config, dummy_id);
    }
  }

  printf("=== End Rule List ===\n\n");

  // Clay UI implementation will be added once compilation issues are resolved
  (void)container_id;
}

/* ============================================================================
 * Rule Analysis Functions
 * ============================================================================
 */

const char *rule_get_type_name(const rule_t *rule) {
  if (!rule)
    return "Invalid";

  rule_display_info_t info = rule_get_display_info(rule);
  return info.name;
}

const char *rule_get_scope_name(rule_scope_t scope) {
  switch (scope) {
  case RULE_SCOPE_SELF:
    return "Self";
  case RULE_SCOPE_NEIGHBORS:
    return "Neighbors";
  case RULE_SCOPE_RANGE:
    return "Range";
  case RULE_SCOPE_POOL:
    return "Pool";
  case RULE_SCOPE_TYPE_GLOBAL:
    return "Type Global";
  case RULE_SCOPE_BOARD_GLOBAL:
    return "Board Global";
  default:
    return "Unknown";
  }
}

const char *rule_get_target_name(rule_target_t target) {
  switch (target) {
  case RULE_TARGET_PRODUCTION:
    return "Production";
  case RULE_TARGET_RANGE:
    return "Range";
  case RULE_TARGET_POOL_MODIFIER:
    return "Pool Modifier";
  case RULE_TARGET_TYPE_OVERRIDE:
    return "Type Override";
  case RULE_TARGET_PLACEMENT_COST:
    return "Placement Cost";
  case RULE_TARGET_MOVEMENT_RANGE:
    return "Movement Range";
  default:
    return "Unknown";
  }
}

const char *rule_get_condition_name(rule_condition_type_t condition_type) {
  switch (condition_type) {
  case RULE_CONDITION_ALWAYS:
    return "Always";
  case RULE_CONDITION_SELF_TYPE:
    return "Self Type";
  case RULE_CONDITION_NEIGHBOR_COUNT:
    return "Neighbor Count";
  case RULE_CONDITION_POOL_SIZE:
    return "Pool Size";
  case RULE_CONDITION_BOARD_COUNT:
    return "Board Count";
  case RULE_CONDITION_PRODUCTION_THRESHOLD:
    return "Production Threshold";
  default:
    return "Unknown";
  }
}

const char *rule_get_effect_name(rule_effect_type_t effect_type) {
  switch (effect_type) {
  case RULE_EFFECT_ADD_FLAT:
    return "Add Flat";
  case RULE_EFFECT_ADD_SCALED:
    return "Add Scaled";
  case RULE_EFFECT_MULTIPLY:
    return "Multiply";
  case RULE_EFFECT_SET_VALUE:
    return "Set Value";
  case RULE_EFFECT_OVERRIDE_TYPE:
    return "Override Type";
  case RULE_EFFECT_MODIFY_RANGE:
    return "Modify Range";
  default:
    return "Unknown";
  }
}

/* ============================================================================
 * Icon/Symbol Functions
 * ============================================================================
 */

const char *rule_get_type_icon(const rule_t *rule) {
  if (!rule)
    return "❓";

  rule_display_info_t info = rule_get_display_info(rule);
  return info.type_icon;
}

const char *rule_get_scope_icon(rule_scope_t scope) {
  switch (scope) {
  case RULE_SCOPE_SELF:
    return "👤";
  case RULE_SCOPE_NEIGHBORS:
    return "👥";
  case RULE_SCOPE_RANGE:
    return "🎯";
  case RULE_SCOPE_POOL:
    return "🌊";
  case RULE_SCOPE_TYPE_GLOBAL:
    return "🔗";
  case RULE_SCOPE_BOARD_GLOBAL:
    return "🌍";
  default:
    return "❓";
  }
}

const char *rule_get_target_icon(rule_target_t target) {
  switch (target) {
  case RULE_TARGET_PRODUCTION:
    return "⚡";
  case RULE_TARGET_RANGE:
    return "📏";
  case RULE_TARGET_POOL_MODIFIER:
    return "💧";
  case RULE_TARGET_TYPE_OVERRIDE:
    return "🎭";
  case RULE_TARGET_PLACEMENT_COST:
    return "💰";
  case RULE_TARGET_MOVEMENT_RANGE:
    return "🏃";
  default:
    return "❓";
  }
}

const char *rule_get_condition_icon(rule_condition_type_t condition_type) {
  switch (condition_type) {
  case RULE_CONDITION_ALWAYS:
    return "✅";
  case RULE_CONDITION_SELF_TYPE:
    return "🔍";
  case RULE_CONDITION_NEIGHBOR_COUNT:
    return "📊";
  case RULE_CONDITION_POOL_SIZE:
    return "📏";
  case RULE_CONDITION_BOARD_COUNT:
    return "🔢";
  case RULE_CONDITION_PRODUCTION_THRESHOLD:
    return "⚖️";
  default:
    return "❓";
  }
}

const char *rule_get_effect_icon(rule_effect_type_t effect_type) {
  switch (effect_type) {
  case RULE_EFFECT_ADD_FLAT:
    return "➕";
  case RULE_EFFECT_ADD_SCALED:
    return "📈";
  case RULE_EFFECT_MULTIPLY:
    return "✖️";
  case RULE_EFFECT_SET_VALUE:
    return "🎯";
  case RULE_EFFECT_OVERRIDE_TYPE:
    return "🎭";
  case RULE_EFFECT_MODIFY_RANGE:
    return "📏";
  default:
    return "❓";
  }
}

/* ============================================================================
 * Text Generation Functions
 * ============================================================================
 */

int rule_generate_description(const rule_t *rule, char *buffer,
                              size_t buffer_size) {
  if (!rule || !buffer || buffer_size == 0)
    return 0;

  rule_display_info_t info = rule_get_display_info(rule);
  return snprintf(buffer, buffer_size, "%s", info.description);
}

int rule_generate_condition_text(const rule_t *rule, char *buffer,
                                 size_t buffer_size) {
  if (!rule || !buffer || buffer_size == 0)
    return 0;

  switch (rule->condition_type) {
  case RULE_CONDITION_ALWAYS:
    return 0; // No condition text

  case RULE_CONDITION_SELF_TYPE:
    return snprintf(buffer, buffer_size, "When tile is %s",
                    rule_get_tile_type_name(rule->condition_params.tile_type));

  case RULE_CONDITION_NEIGHBOR_COUNT: {
    if (rule->condition_params.neighbor_count.min_count ==
        rule->condition_params.neighbor_count.max_count) {
      return snprintf(buffer, buffer_size, "With exactly %d %s neighbors",
                      rule->condition_params.neighbor_count.min_count,
                      rule_get_tile_type_name(
                        rule->condition_params.neighbor_count.neighbor_type));
    } else {
      return snprintf(buffer, buffer_size, "With %d-%d %s neighbors",
                      rule->condition_params.neighbor_count.min_count,
                      rule->condition_params.neighbor_count.max_count,
                      rule_get_tile_type_name(
                        rule->condition_params.neighbor_count.neighbor_type));
    }
  }

  case RULE_CONDITION_POOL_SIZE: {
    if (rule->condition_params.pool_size.min_size ==
        rule->condition_params.pool_size.max_size) {
      return snprintf(buffer, buffer_size, "In pool of %d tiles",
                      rule->condition_params.pool_size.min_size);
    } else {
      return snprintf(buffer, buffer_size, "In pool of %d-%d tiles",
                      rule->condition_params.pool_size.min_size,
                      rule->condition_params.pool_size.max_size);
    }
  }

  case RULE_CONDITION_BOARD_COUNT: {
    return snprintf(
      buffer, buffer_size, "With %d-%d %s tiles on board",
      rule->condition_params.board_count.min_count,
      rule->condition_params.board_count.max_count,
      rule_get_tile_type_name(rule->condition_params.board_count.target_type));
  }

  case RULE_CONDITION_PRODUCTION_THRESHOLD: {
    return snprintf(buffer, buffer_size, "When production %s %.1f",
                    rule->condition_params.production_threshold.greater_than
                      ? "exceeds"
                      : "below",
                    rule->condition_params.production_threshold.threshold);
  }

  default:
    return snprintf(buffer, buffer_size, "Unknown condition");
  }
}

int rule_generate_effect_text(const rule_t *rule, char *buffer,
                              size_t buffer_size) {
  if (!rule || !buffer || buffer_size == 0)
    return 0;

  const char *target_name = rule_get_target_name(rule->target);

  switch (rule->effect_type) {
  case RULE_EFFECT_ADD_FLAT: {
    float value = rule->effect_params.value;
    if (value >= 0) {
      return snprintf(buffer, buffer_size, "+%.1f %s", value, target_name);
    } else {
      return snprintf(buffer, buffer_size, "%.1f %s", value, target_name);
    }
  }

  case RULE_EFFECT_MULTIPLY:
    return snprintf(buffer, buffer_size, "×%.2f %s", rule->effect_params.value,
                    target_name);

  case RULE_EFFECT_SET_VALUE:
    return snprintf(buffer, buffer_size, "Set %s to %.1f", target_name,
                    rule->effect_params.value);

  case RULE_EFFECT_OVERRIDE_TYPE:
    return snprintf(buffer, buffer_size, "Appear as %s",
                    rule_get_tile_type_name(rule->effect_params.override_type));

  case RULE_EFFECT_MODIFY_RANGE: {
    int8_t delta = rule->effect_params.range_delta;
    if (delta >= 0) {
      return snprintf(buffer, buffer_size, "+%d %s", delta, target_name);
    } else {
      return snprintf(buffer, buffer_size, "%d %s", delta, target_name);
    }
  }

  case RULE_EFFECT_ADD_SCALED:
    return snprintf(buffer, buffer_size, "+%.1f %s per match",
                    rule->effect_params.scaled.scale_factor, target_name);

  default:
    return snprintf(buffer, buffer_size, "Unknown effect");
  }
}

int rule_generate_scope_text(const rule_t *rule, char *buffer,
                             size_t buffer_size) {
  if (!rule || !buffer || buffer_size == 0)
    return 0;

  switch (rule->scope) {
  case RULE_SCOPE_SELF:
    return snprintf(buffer, buffer_size, "Affects self");
  case RULE_SCOPE_NEIGHBORS:
    return snprintf(buffer, buffer_size, "Affects neighbors");
  case RULE_SCOPE_RANGE:
    return snprintf(buffer, buffer_size, "Affects range %d",
                    rule->affected_range);
  case RULE_SCOPE_POOL:
    return snprintf(buffer, buffer_size, "Affects entire pool");
  case RULE_SCOPE_TYPE_GLOBAL:
    return snprintf(buffer, buffer_size, "Affects all tiles of type");
  case RULE_SCOPE_BOARD_GLOBAL:
    return snprintf(buffer, buffer_size, "Affects entire board");
  default:
    return snprintf(buffer, buffer_size, "Unknown scope");
  }
}

/* ============================================================================
 * Color and Styling Functions
 * ============================================================================
 */

void rule_get_colors(const rule_t *rule, Clay_Color *primary,
                     Clay_Color *accent) {
  if (!rule || !primary || !accent)
    return;

  rule_display_info_t info = rule_get_display_info(rule);
  *primary = info.primary_color;
  *accent = info.accent_color;
}

uint32_t rule_get_display_priority(const rule_t *rule) {
  if (!rule)
    return 0;

  uint32_t priority = 100; // Base priority

  // Boost for important effects
  if (rule_is_visually_important(rule)) {
    priority += 200;
  }

  // Scope priority
  switch (rule->scope) {
  case RULE_SCOPE_BOARD_GLOBAL:
    priority += 150;
    break;
  case RULE_SCOPE_TYPE_GLOBAL:
    priority += 140;
    break;
  case RULE_SCOPE_POOL:
    priority += 130;
    break;
  case RULE_SCOPE_RANGE:
    priority += 120;
    break;
  case RULE_SCOPE_NEIGHBORS:
    priority += 110;
    break;
  case RULE_SCOPE_SELF:
    priority += 100;
    break;
  }

  // Target priority
  switch (rule->target) {
  case RULE_TARGET_TYPE_OVERRIDE:
    priority += 50;
    break;
  case RULE_TARGET_PRODUCTION:
    priority += 40;
    break;
  case RULE_TARGET_RANGE:
    priority += 30;
    break;
  case RULE_TARGET_POOL_MODIFIER:
    priority += 20;
    break;
  default:
    break;
  }

  return priority;
}

bool rule_is_visually_important(const rule_t *rule) {
  if (!rule)
    return false;

  // Global effects are important
  if (rule->scope == RULE_SCOPE_BOARD_GLOBAL ||
      rule->scope == RULE_SCOPE_TYPE_GLOBAL) {
    return true;
  }

  // Type overrides are visually significant
  if (rule->target == RULE_TARGET_TYPE_OVERRIDE) {
    return true;
  }

  // Large numeric effects
  if (rule->effect_type == RULE_EFFECT_ADD_FLAT) {
    float value = rule->effect_params.value;
    if (value > 5.0f || value < -2.0f) {
      return true;
    }
  }

  return false;
}

/* ============================================================================
 * Configuration Helpers
 * ============================================================================
 */

rule_card_config_t
rule_card_config_create(bool show_icons, bool show_descriptions, bool compact) {
  rule_card_config_t config = {.show_icons = show_icons,
                               .show_descriptions = show_descriptions,
                               .show_conditions = !compact,
                               .show_effects = true,
                               .compact_layout = compact,
                               .card_width = compact ? 200.0f : 280.0f,
                               .max_height = compact ? 100.0f : 200.0f};
  return config;
}

void rule_sort_by_priority(const rule_t **rules, uint32_t rule_count) {
  if (!rules || rule_count <= 1)
    return;

  // Simple bubble sort by priority (descending)
  for (uint32_t i = 0; i < rule_count - 1; i++) {
    for (uint32_t j = 0; j < rule_count - i - 1; j++) {
      uint32_t priority_j = rule_get_display_priority(rules[j]);
      uint32_t priority_j1 = rule_get_display_priority(rules[j + 1]);

      if (priority_j < priority_j1) {
        const rule_t *temp = rules[j];
        rules[j] = rules[j + 1];
        rules[j + 1] = temp;
      }
    }
  }
}

/* ============================================================================
 * Utility Functions
 * ============================================================================
 */

const char *rule_get_tile_type_name(tile_type_t tile_type) {
  switch (tile_type) {
  case TILE_MAGENTA:
    return "Magenta";
  case TILE_CYAN:
    return "Cyan";
  case TILE_YELLOW:
    return "Yellow";
  case TILE_GREEN:
    return "Green";
  case TILE_EMPTY:
    return "Empty";
  case TILE_UNDEFINED:
    return "Undefined";
  default:
    return "Unknown";
  }
}

Clay_Color rule_get_tile_type_color(tile_type_t tile_type) {
  switch (tile_type) {
  case TILE_MAGENTA:
    return (Clay_Color){255, 0, 255, 255};
  case TILE_CYAN:
    return (Clay_Color){0, 255, 255, 255};
  case TILE_YELLOW:
    return (Clay_Color){255, 255, 0, 255};
  case TILE_GREEN:
    return (Clay_Color){0, 255, 0, 255};
  case TILE_EMPTY:
    return (Clay_Color){128, 128, 128, 255};
  default:
    return (Clay_Color){200, 200, 200, 255};
  }
}

bool rule_affects_tile_type(const rule_t *rule, tile_type_t tile_type) {
  if (!rule)
    return false;

  // Check if rule's condition includes this tile type
  switch (rule->condition_type) {
  case RULE_CONDITION_SELF_TYPE:
    return rule->condition_params.tile_type == tile_type;

  case RULE_CONDITION_NEIGHBOR_COUNT:
    return rule->condition_params.neighbor_count.neighbor_type == tile_type;

  case RULE_CONDITION_BOARD_COUNT:
    return rule->condition_params.board_count.target_type == tile_type;

  default:
    return true; // Other conditions might affect any tile type
  }
}
