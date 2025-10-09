/**************************************************************************/ /**
                                                                              * @file reward_system.c
                                                                              * @brief Implementation of the reward system for generating rule-based rewards
                                                                              *****************************************************************************/

#include "game/reward_system.h"
#include "game/rule_system.h"
#include "tile/tile.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// --- Reward Templates ---

static rule_t
generate_neighbor_bonus_rule(const reward_generation_params_t *params,
                             uint32_t seed);
static rule_t
generate_range_modifier_rule(const reward_generation_params_t *params,
                             uint32_t seed);
static rule_t
generate_type_synergy_rule(const reward_generation_params_t *params,
                           uint32_t seed);
static rule_t
generate_global_modifier_rule(const reward_generation_params_t *params,
                              uint32_t seed);
static rule_t
generate_special_effect_rule(const reward_generation_params_t *params,
                             uint32_t seed);

/**
 * @brief Built-in reward templates
 */
static const reward_template_t REWARD_TEMPLATES[] = {
  // Common Production Rewards
  {.category = REWARD_CATEGORY_PRODUCTION,
   .rarity = REWARD_RARITY_COMMON,
   .title_template = "Neighbor Bonus",
   .description_template = "Adjacent %s tiles provide +%.1f production",
   .flavor_template = "Cooperation breeds success",
   .generate_rule = generate_neighbor_bonus_rule,
   .min_player_level = 1,
   .requires_specific_tiles = false,
   .base_power = 2.0f,
   .power_variance = 1.0f},
  {.category = REWARD_CATEGORY_RANGE,
   .rarity = REWARD_RARITY_COMMON,
   .title_template = "Extended Reach",
   .description_template = "Range increased by %d",
   .flavor_template = "Expand your influence",
   .generate_rule = generate_range_modifier_rule,
   .min_player_level = 1,
   .requires_specific_tiles = false,
   .base_power = 1.5f,
   .power_variance = 0.5f},

  // Uncommon Synergy Rewards
  {.category = REWARD_CATEGORY_SYNERGY,
   .rarity = REWARD_RARITY_UNCOMMON,
   .title_template = "Color Harmony",
   .description_template = "%s tiles count as %s tiles for bonuses",
   .flavor_template = "Unity in diversity",
   .generate_rule = generate_type_synergy_rule,
   .min_player_level = 3,
   .requires_specific_tiles = true,
   .base_power = 3.0f,
   .power_variance = 1.5f},

  // Rare Global Effects
  {.category = REWARD_CATEGORY_GLOBAL,
   .rarity = REWARD_RARITY_RARE,
   .title_template = "Global Amplifier",
   .description_template = "All tiles produce %.0f%% more",
   .flavor_template = "Power flows through all",
   .generate_rule = generate_global_modifier_rule,
   .min_player_level = 5,
   .requires_specific_tiles = false,
   .base_power = 5.0f,
   .power_variance = 2.0f},

  // Legendary Special Effects
  {.category = REWARD_CATEGORY_SPECIAL,
   .rarity = REWARD_RARITY_LEGENDARY,
   .title_template = "Reality Shift",
   .description_template = "Unique game-changing effect",
   .flavor_template = "The rules themselves bend",
   .generate_rule = generate_special_effect_rule,
   .min_player_level = 10,
   .requires_specific_tiles = false,
   .base_power = 8.0f,
   .power_variance = 3.0f}};

static const uint32_t NUM_REWARD_TEMPLATES =
  sizeof(REWARD_TEMPLATES) / sizeof(REWARD_TEMPLATES[0]);

// --- Utility Functions ---

static uint32_t simple_random(uint32_t seed) {
  seed ^= seed << 13;
  seed ^= seed >> 17;
  seed ^= seed << 5;
  return seed;
}

static float random_float(uint32_t *seed) {
  *seed = simple_random(*seed);
  return (*seed % 10000) / 10000.0f;
}

static int random_int(uint32_t *seed, int min, int max) {
  *seed = simple_random(*seed);
  return min + (*seed % (max - min + 1));
}

static tile_type_t random_tile_type(uint32_t *seed) {
  return (tile_type_t)random_int(seed, TILE_MAGENTA, TILE_GREEN);
}

// --- Rule Generation Functions ---

static rule_t
generate_neighbor_bonus_rule(const reward_generation_params_t *params,
                             uint32_t seed) {
  rule_t rule = {0};
  uint32_t local_seed = seed;

  rule.id = 0; // Will be assigned by rule manager
  rule.priority = RULE_PRIORITY_PRODUCTION;
  rule.scope = RULE_SCOPE_NEIGHBORS;
  rule.target = RULE_TARGET_PRODUCTION;

  rule.condition_type = RULE_CONDITION_NEIGHBOR_COUNT;
  rule.condition_params.neighbor_count.neighbor_type =
    random_tile_type(&local_seed);
  rule.condition_params.neighbor_count.min_count = 1;
  rule.condition_params.neighbor_count.max_count = 6;
  rule.condition_params.neighbor_count.range = 1;

  rule.effect_type = RULE_EFFECT_ADD_SCALED;
  rule.effect_params.scaled.base_value = 0.0f;
  rule.effect_params.scaled.scale_factor =
    1.0f + random_float(&local_seed) * 2.0f;
  rule.effect_params.scaled.scale_source = RULE_CONDITION_NEIGHBOR_COUNT;
  rule.effect_params.scaled.scale_params = rule.condition_params;

  rule.source_cell = (grid_cell_t){.type = GRID_TYPE_HEXAGON,
                                   .coord.hex = {.q = 0, .r = 0, .s = 0}};
  rule.affected_range = 1;
  rule.is_active = true;
  rule.needs_recalc = false;
  rule.cache_friendly = true;

  return rule;
}

static rule_t
generate_range_modifier_rule(const reward_generation_params_t *params,
                             uint32_t seed) {
  rule_t rule = {0};
  uint32_t local_seed = seed;

  rule.id = 0;
  rule.priority = RULE_PRIORITY_RANGE_MODIFY;
  rule.scope = RULE_SCOPE_SELF;
  rule.target = RULE_TARGET_RANGE;

  rule.condition_type = RULE_CONDITION_ALWAYS;

  rule.effect_type = RULE_EFFECT_MODIFY_RANGE;
  rule.effect_params.range_delta = random_int(&local_seed, 1, 2);

  rule.source_cell = (grid_cell_t){.type = GRID_TYPE_HEXAGON,
                                   .coord.hex = {.q = 0, .r = 0, .s = 0}};
  rule.affected_range = 0; // Self only
  rule.is_active = true;
  rule.needs_recalc = false;
  rule.cache_friendly = true;

  return rule;
}

static rule_t
generate_type_synergy_rule(const reward_generation_params_t *params,
                           uint32_t seed) {
  rule_t rule = {0};
  uint32_t local_seed = seed;

  rule.id = 0;
  rule.priority = RULE_PRIORITY_PERCEPTION;
  rule.scope = RULE_SCOPE_RANGE;
  rule.target = RULE_TARGET_TYPE_OVERRIDE;

  rule.condition_type = RULE_CONDITION_SELF_TYPE;
  rule.condition_params.tile_type = random_tile_type(&local_seed);

  rule.effect_type = RULE_EFFECT_OVERRIDE_TYPE;
  rule.effect_params.override_type = random_tile_type(&local_seed);

  rule.source_cell = (grid_cell_t){.type = GRID_TYPE_HEXAGON,
                                   .coord.hex = {.q = 0, .r = 0, .s = 0}};
  rule.affected_range = random_int(&local_seed, 1, 3);
  rule.is_active = true;
  rule.needs_recalc = true;
  rule.cache_friendly = false;

  return rule;
}

static rule_t
generate_global_modifier_rule(const reward_generation_params_t *params,
                              uint32_t seed) {
  rule_t rule = {0};
  uint32_t local_seed = seed;

  rule.id = 0;
  rule.priority = RULE_PRIORITY_PRODUCTION;
  rule.scope = RULE_SCOPE_BOARD_GLOBAL;
  rule.target = RULE_TARGET_PRODUCTION;

  rule.condition_type = RULE_CONDITION_ALWAYS;

  rule.effect_type = RULE_EFFECT_MULTIPLY;
  rule.effect_params.value = 1.0f + (random_float(&local_seed) * 0.5f + 0.1f);

  rule.source_cell = (grid_cell_t){.type = GRID_TYPE_HEXAGON,
                                   .coord.hex = {.q = 0, .r = 0, .s = 0}};
  rule.affected_range = 0; // Global
  rule.is_active = true;
  rule.needs_recalc = true;
  rule.cache_friendly = false;

  return rule;
}

static rule_t
generate_special_effect_rule(const reward_generation_params_t *params,
                             uint32_t seed) {
  rule_t rule = {0};
  uint32_t local_seed = seed;

  rule.id = 0;
  rule.priority = RULE_PRIORITY_EFFECTS;
  rule.scope = RULE_SCOPE_POOL;
  rule.target = RULE_TARGET_PRODUCTION;

  rule.condition_type = RULE_CONDITION_POOL_SIZE;
  rule.condition_params.pool_size.min_size = random_int(&local_seed, 3, 8);
  rule.condition_params.pool_size.max_size = 999;

  rule.effect_type = RULE_EFFECT_ADD_SCALED;
  rule.effect_params.scaled.base_value = 0.0f;
  rule.effect_params.scaled.scale_factor = 0.5f + random_float(&local_seed);
  rule.effect_params.scaled.scale_source = RULE_CONDITION_POOL_SIZE;
  rule.effect_params.scaled.scale_params = rule.condition_params;

  rule.source_cell = (grid_cell_t){.type = GRID_TYPE_HEXAGON,
                                   .coord.hex = {.q = 0, .r = 0, .s = 0}};
  rule.affected_range = 0; // Pool scope
  rule.is_active = true;
  rule.needs_recalc = true;
  rule.cache_friendly = false;

  return rule;
}

// --- Reward System Implementation ---

bool reward_system_init(reward_system_t *system, uint32_t seed) {
  if (!system) {
    return false;
  }

  memset(system, 0, sizeof(reward_system_t));

  system->templates = REWARD_TEMPLATES;
  system->template_count = NUM_REWARD_TEMPLATES;
  system->random_seed = seed;
  system->initialized = true;

  // Initialize selection state
  system->selection_state.selected_index = -1;
  system->selection_state.selection_confirmed = false;
  system->selection_state.rewards_generated = false;

  return true;
}

void reward_system_cleanup(reward_system_t *system) {
  if (!system || !system->initialized) {
    return;
  }

  memset(system, 0, sizeof(reward_system_t));
}

reward_generation_params_t reward_system_create_default_params(
  const board_t *board, const rule_manager_t *rule_manager,
  uint32_t player_level, uint32_t turn_number) {
  reward_generation_params_t params = {0};

  params.min_rarity = REWARD_RARITY_COMMON;
  params.max_rarity = (player_level < 5)    ? REWARD_RARITY_UNCOMMON
                      : (player_level < 10) ? REWARD_RARITY_RARE
                                            : REWARD_RARITY_LEGENDARY;

  // Set preferred categories based on game state
  params.preferred_categories[0] = REWARD_CATEGORY_PRODUCTION;
  params.preferred_categories[1] = REWARD_CATEGORY_RANGE;
  params.num_preferred_categories = 2;

  if (player_level >= 3) {
    params.preferred_categories[params.num_preferred_categories++] =
      REWARD_CATEGORY_SYNERGY;
  }
  if (player_level >= 5) {
    params.preferred_categories[params.num_preferred_categories++] =
      REWARD_CATEGORY_GLOBAL;
  }
  if (player_level >= 10) {
    params.preferred_categories[params.num_preferred_categories++] =
      REWARD_CATEGORY_SPECIAL;
  }

  params.board = board;
  params.rule_manager = rule_manager;
  params.player_level = player_level;
  params.turn_number = turn_number;
  params.seed = (uint32_t)time(NULL) + turn_number;
  params.ensure_variety = true;
  params.balance_power = true;

  return params;
}

bool reward_system_generate_options(reward_system_t *system,
                                    const reward_generation_params_t *params) {
  if (!system || !system->initialized || !params) {
    return false;
  }

  uint32_t seed = params->seed ^ system->random_seed;

  // Clear previous options
  memset(system->selection_state.options, 0,
         sizeof(system->selection_state.options));
  system->selection_state.option_count = 0;
  system->selection_state.selected_index = -1;
  system->selection_state.selection_confirmed = false;

  // Generate rewards
  for (uint8_t i = 0; i < MAX_REWARD_OPTIONS; i++) {
    // Select template based on rarity and category preferences
    uint32_t template_seed = seed + i * 1000;
    reward_rarity_t target_rarity = (reward_rarity_t)random_int(
      &template_seed, params->min_rarity, params->max_rarity);

    // Find suitable templates
    const reward_template_t *suitable_templates[NUM_REWARD_TEMPLATES];
    uint8_t suitable_count = 0;

    for (uint32_t t = 0; t < system->template_count; t++) {
      const reward_template_t *template = &system->templates[t];

      if (template->rarity == target_rarity &&
          template->min_player_level <= params->player_level) {
        suitable_templates[suitable_count++] = template;
      }
    }

    if (suitable_count == 0) {
      continue; // Skip this option
    }

    // Select random template
    uint32_t template_index = random_int(&template_seed, 0, suitable_count - 1);
    const reward_template_t *selected_template =
      suitable_templates[template_index];

    // Generate rule
    rule_t rule = selected_template->generate_rule(params, template_seed + 500);

    // Create reward option
    reward_option_t *option = &system->selection_state.options[i];
    option->rule = rule;
    option->rarity = selected_template->rarity;
    option->category = selected_template->category;
    option->title = selected_template->title_template;
    option->description = selected_template->description_template;
    option->flavor_text = selected_template->flavor_template;
    option->display_priority = (uint32_t)selected_template->rarity * 1000 +
                               (uint32_t)selected_template->category * 100;
    option->is_selected = false;
    option->is_available = true;
    option->generation_seed = template_seed;
    option->power_score =
      selected_template->base_power +
      random_float(&template_seed) * selected_template->power_variance;

    system->selection_state.option_count++;
  }

  system->selection_state.rewards_generated = true;
  system->selection_state.generation_params = *params;
  system->selection_state.generation_time = (uint32_t)time(NULL);

  return system->selection_state.option_count > 0;
}

bool reward_system_select_option(reward_system_t *system,
                                 uint8_t option_index) {
  if (!system || !system->initialized ||
      !system->selection_state.rewards_generated) {
    return false;
  }

  if (option_index >= system->selection_state.option_count) {
    return false;
  }

  // Clear previous selection
  for (uint8_t i = 0; i < system->selection_state.option_count; i++) {
    system->selection_state.options[i].is_selected = false;
  }

  // Set new selection
  system->selection_state.options[option_index].is_selected = true;
  system->selection_state.selected_index = option_index;
  system->selection_state.selection_confirmed = false;

  return true;
}

bool reward_system_confirm_selection(reward_system_t *system,
                                     rule_t *out_selected_rule) {
  if (!system || !system->initialized ||
      !system->selection_state.rewards_generated) {
    return false;
  }

  if (system->selection_state.selected_index < 0 ||
      system->selection_state.selected_index >=
        system->selection_state.option_count) {
    return false;
  }

  const reward_option_t *selected =
    &system->selection_state.options[system->selection_state.selected_index];

  if (!selected->is_available) {
    return false;
  }

  if (out_selected_rule) {
    *out_selected_rule = selected->rule;
  }

  system->selection_state.selection_confirmed = true;

  // Record selection for statistics
  reward_system_record_selection(system, selected);

  return true;
}

void reward_system_clear_selection(reward_system_t *system) {
  if (!system || !system->initialized) {
    return;
  }

  for (uint8_t i = 0; i < system->selection_state.option_count; i++) {
    system->selection_state.options[i].is_selected = false;
  }

  system->selection_state.selected_index = -1;
  system->selection_state.selection_confirmed = false;
}

bool reward_system_has_confirmed_selection(const reward_system_t *system) {
  if (!system || !system->initialized) {
    return false;
  }

  return system->selection_state.selection_confirmed;
}

bool reward_system_get_options(const reward_system_t *system,
                               reward_option_t **out_options,
                               uint8_t *out_count) {
  if (!system || !system->initialized || !out_options || !out_count) {
    return false;
  }

  if (!system->selection_state.rewards_generated) {
    *out_options = NULL;
    *out_count = 0;
    return false;
  }

  *out_options = (reward_option_t *)system->selection_state.options;
  *out_count = system->selection_state.option_count;
  return true;
}

const reward_option_t *
reward_system_get_selected_option(const reward_system_t *system) {
  if (!system || !system->initialized ||
      !system->selection_state.rewards_generated) {
    return NULL;
  }

  if (system->selection_state.selected_index < 0 ||
      system->selection_state.selected_index >=
        system->selection_state.option_count) {
    return NULL;
  }

  return &system->selection_state
            .options[system->selection_state.selected_index];
}

const reward_option_t *reward_system_get_option(const reward_system_t *system,
                                                uint8_t option_index) {
  if (!system || !system->initialized ||
      !system->selection_state.rewards_generated) {
    return NULL;
  }

  if (option_index >= system->selection_state.option_count) {
    return NULL;
  }

  return &system->selection_state.options[option_index];
}

void reward_system_get_rarity_info(reward_rarity_t rarity,
                                   const char **out_name, uint32_t *out_color) {
  static const char *rarity_names[] = {"Common", "Uncommon", "Rare",
                                       "Legendary"};

  static const uint32_t rarity_colors[] = {
    0x808080FF, // Gray for common
    0x00FF00FF, // Green for uncommon
    0x0080FFFF, // Blue for rare
    0xFF8000FF  // Orange for legendary
  };

  if (rarity >= REWARD_RARITY_COUNT) {
    rarity = REWARD_RARITY_COMMON;
  }

  if (out_name) {
    *out_name = rarity_names[rarity];
  }

  if (out_color) {
    *out_color = rarity_colors[rarity];
  }
}

void reward_system_get_category_info(reward_category_t category,
                                     const char **out_name,
                                     const char **out_icon) {
  static const char *category_names[] = {"Production", "Range", "Synergy",
                                         "Global", "Special"};

  static const char *category_icons[] = {"⚡", "🎯", "🔗", "🌍", "✨"};

  if (category >= REWARD_CATEGORY_COUNT) {
    category = REWARD_CATEGORY_PRODUCTION;
  }

  if (out_name) {
    *out_name = category_names[category];
  }

  if (out_icon) {
    *out_icon = category_icons[category];
  }
}

void reward_system_record_selection(reward_system_t *system,
                                    const reward_option_t *selected_reward) {
  if (!system || !system->initialized || !selected_reward) {
    return;
  }

  system->rewards_selected++;

  // Record rarity history
  for (int i = 9; i > 0; i--) {
    system->last_rarities[i] = system->last_rarities[i - 1];
  }
  system->last_rarities[0] = selected_reward->rarity;

  // Record category history
  for (int i = 9; i > 0; i--) {
    system->last_categories[i] = system->last_categories[i - 1];
  }
  system->last_categories[0] = selected_reward->category;
}

void reward_system_debug_print(const reward_system_t *system) {
  if (!system) {
    printf("Reward system: NULL\n");
    return;
  }

  printf("=== Reward System Debug ===\n");
  printf("Initialized: %s\n", system->initialized ? "Yes" : "No");
  printf("Rewards offered: %u\n", system->rewards_offered);
  printf("Rewards selected: %u\n", system->rewards_selected);
  printf("Random seed: %u\n", system->random_seed);
  printf("Templates: %u\n", system->template_count);

  printf("\nSelection State:\n");
  printf("  Options generated: %s\n",
         system->selection_state.rewards_generated ? "Yes" : "No");
  printf("  Option count: %u\n", system->selection_state.option_count);
  printf("  Selected index: %d\n", system->selection_state.selected_index);
  printf("  Selection confirmed: %s\n",
         system->selection_state.selection_confirmed ? "Yes" : "No");

  if (system->selection_state.rewards_generated) {
    printf("\nCurrent Options:\n");
    for (uint8_t i = 0; i < system->selection_state.option_count; i++) {
      const reward_option_t *option = &system->selection_state.options[i];
      const char *rarity_name;
      const char *category_name;
      uint32_t color;

      reward_system_get_rarity_info(option->rarity, &rarity_name, &color);
      reward_system_get_category_info(option->category, &category_name, NULL);

      printf("  [%u] %s %s - %s (Power: %.1f) %s\n", i, rarity_name,
             category_name, option->title, option->power_score,
             option->is_selected ? "[SELECTED]" : "");
    }
  }
}

bool reward_system_validate(const reward_system_t *system) {
  if (!system) {
    return false;
  }

  // Basic validation
  if (!system->initialized) {
    return false;
  }

  if (!system->templates) {
    return false;
  }

  if (system->template_count == 0) {
    return false;
  }

  // Validate selection state
  if (system->selection_state.option_count > MAX_REWARD_OPTIONS) {
    return false;
  }

  if (system->selection_state.selected_index >=
        system->selection_state.option_count &&
      system->selection_state.selected_index != -1) {
    return false;
  }

  return true;
}
