/**************************************************************************/ /**
                                                                              * @file rule_manager.c
                                                                              * @brief High-level rule management integration for single-player hexhex game
                                                                              *****************************************************************************/

#include "game/rule_manager.h"
#include "tile/tile_map.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- Helper Functions ---

static tile_t *tile_map_get_tile(const tile_map_t *map, grid_cell_t cell) {
    if (!map)
        return NULL;
    tile_map_entry_t *entry = tile_map_find(map, cell);
    return entry ? entry->tile : NULL;
}

// --- Rule Manager Lifecycle ---

bool rule_manager_init(rule_manager_t *manager, const board_t *board,
                       uint32_t max_tiles) {
    if (!manager || !board) {
        return false;
    }

    memset(manager, 0, sizeof(*manager));
    manager->board = board;

    // Initialize rule registry
    if (!rule_registry_init(&manager->registry, max_tiles)) {
        return false;
    }

    // Initialize evaluation context
    if (!rule_context_init(&manager->context, board, &manager->registry,
                           1000)) {
        rule_registry_cleanup(&manager->registry);
        return false;
    }

    manager->initialized = true;
    manager->rules_dirty = false;
    manager->evaluations_this_turn = 0;
    manager->total_evaluations = 0;
    manager->rules_applied_this_turn = 0;

    return true;
}

void rule_manager_cleanup(rule_manager_t *manager) {
    if (!manager) {
        return;
    }

    rule_context_cleanup(&manager->context);
    rule_registry_cleanup(&manager->registry);

    memset(manager, 0, sizeof(*manager));
}

// --- Rule Management ---

uint32_t rule_manager_add_rule(rule_manager_t *manager, const rule_t *rule) {
    if (!manager || !rule || !manager->initialized) {
        return 0;
    }

    uint32_t rule_id = rule_registry_add_rule(&manager->registry, rule);
    if (rule_id > 0) {
        manager->rules_dirty = true;
    }

    return rule_id;
}

bool rule_manager_remove_rule(rule_manager_t *manager, uint32_t rule_id) {
    if (!manager || !manager->initialized) {
        return false;
    }

    bool removed = rule_registry_remove_rule(&manager->registry, rule_id);
    if (removed) {
        manager->rules_dirty = true;
    }

    return removed;
}

void rule_manager_remove_rules_by_source(rule_manager_t *manager,
                                         grid_cell_t source_cell) {
    if (!manager || !manager->initialized) {
        return;
    }

    rule_registry_remove_by_source(&manager->registry, source_cell);
    manager->rules_dirty = true;
}

// --- Game Event Integration ---

void rule_manager_on_tile_placed(rule_manager_t *manager, const tile_t *tile) {
    if (!manager || !tile || !manager->initialized) {
        return;
    }

    // Create rules based on tile type (implement basic tile-based rules here)
    switch (tile->data.type) {
    case TILE_GREEN:
        // Example: Green tiles get +1 production per green neighbor in range 2
        rule_manager_add_neighbor_bonus(manager, tile, TILE_GREEN, 1.0f, 2);
        break;
    case TILE_MAGENTA:
        // Example: Magenta tiles increase range of all tiles by 1
        rule_manager_add_range_modifier(manager, tile, TILE_UNDEFINED, 1);
        break;
    case TILE_CYAN:
        // Example: Cyan tiles make neighbors appear as cyan
        rule_manager_add_type_override(manager, tile, TILE_CYAN, 1);
        break;
    case TILE_YELLOW:
        // Example: Yellow tiles get scaling bonus based on pool size
        {
            rule_t pool_scaling =
              rule_create_pool_scaling(tile->cell, 1.0f, 0.1f);
            rule_manager_add_rule(manager, &pool_scaling);
        }
        break;
    default:
        break;
    }

    manager->rules_dirty = true;
}

void rule_manager_on_tile_removed(rule_manager_t *manager, grid_cell_t cell) {
    if (!manager || !manager->initialized) {
        return;
    }

    rule_manager_remove_rules_by_source(manager, cell);
}

void rule_manager_on_pool_changed(rule_manager_t *manager, const pool_t *pool) {
    if (!manager || !pool || !manager->initialized) {
        return;
    }

    // Mark area around pool as dirty for recalculation
    rule_registry_mark_area_dirty(&manager->registry, pool->center, 3);
    manager->rules_dirty = true;
}

void rule_manager_on_turn_start(rule_manager_t *manager) {
    if (!manager || !manager->initialized) {
        return;
    }

    // Reset per-turn counters
    manager->evaluations_this_turn = 0;
    manager->rules_applied_this_turn = 0;
}

void rule_manager_on_turn_end(rule_manager_t *manager) {
    if (!manager || !manager->initialized) {
        return;
    }

    // Process any pending rule updates
    if (manager->rules_dirty) {
        rule_registry_process_dirty_tiles(&manager->registry,
                                          &manager->context);
        manager->rules_dirty = false;
    }
}

// --- Rule Evaluation ---

void rule_manager_evaluate_rules(rule_manager_t *manager,
                                 rule_timing_t timing) {
    if (!manager || !manager->initialized) {
        return;
    }

    // Process dirty tiles if needed
    if (manager->rules_dirty) {
        rule_registry_process_dirty_tiles(&manager->registry,
                                          &manager->context);
        manager->rules_dirty = false;
    }

    manager->total_evaluations++;
    manager->evaluations_this_turn++;
}

float rule_manager_calculate_tile_production(rule_manager_t *manager,
                                             const tile_t *tile) {
    if (!manager || !tile || !manager->initialized) {
        return 0.0f;
    }

    float production = rule_calculate_tile_production(&manager->registry,
                                                      &manager->context, tile);
    manager->evaluations_this_turn++;

    return production;
}

float rule_manager_calculate_pool_multiplier(rule_manager_t *manager,
                                             const pool_t *pool) {
    if (!manager || !pool || !manager->initialized) {
        return 1.0f;
    }

    // TODO: Implement pool-specific rule evaluation
    // For now, return base multiplier
    return 1.0f;
}

tile_type_t rule_manager_get_perceived_type(rule_manager_t *manager,
                                            const tile_t *tile,
                                            grid_cell_t observer_cell) {
    if (!manager || !tile || !manager->initialized) {
        return tile->data.type;
    }

    tile_type_t perceived = rule_calculate_perceived_type(
      &manager->registry, &manager->context, tile, observer_cell);
    manager->evaluations_this_turn++;

    return perceived;
}

// --- Rule Creation Helpers ---

void rule_manager_add_neighbor_bonus(rule_manager_t *manager,
                                     const tile_t *source_tile,
                                     tile_type_t neighbor_type,
                                     float bonus_per_neighbor, uint8_t range) {
    if (!manager || !source_tile || !manager->initialized) {
        return;
    }

    rule_t rule = rule_create_neighbor_bonus(source_tile->cell, neighbor_type,
                                             bonus_per_neighbor, range);
    rule_manager_add_rule(manager, &rule);
}

void rule_manager_add_range_modifier(rule_manager_t *manager,
                                     const tile_t *source_tile,
                                     tile_type_t target_type,
                                     int8_t range_delta) {
    if (!manager || !source_tile || !manager->initialized) {
        return;
    }

    rule_t rule =
      rule_create_range_modifier(source_tile->cell, target_type, range_delta);
    rule_manager_add_rule(manager, &rule);
}

void rule_manager_add_type_override(rule_manager_t *manager,
                                    const tile_t *source_tile,
                                    tile_type_t override_type, uint8_t range) {
    if (!manager || !source_tile || !manager->initialized) {
        return;
    }

    rule_t rule =
      rule_create_type_override(source_tile->cell, override_type, range);
    rule_manager_add_rule(manager, &rule);
}

// --- Performance and Debugging ---

void rule_manager_get_performance_stats(const rule_manager_t *manager,
                                        uint32_t *out_active_rules,
                                        uint32_t *out_evaluations_this_turn,
                                        uint32_t *out_total_evaluations) {
    if (!manager) {
        if (out_active_rules)
            *out_active_rules = 0;
        if (out_evaluations_this_turn)
            *out_evaluations_this_turn = 0;
        if (out_total_evaluations)
            *out_total_evaluations = 0;
        return;
    }

    if (out_active_rules)
        *out_active_rules = manager->registry.rule_count;
    if (out_evaluations_this_turn)
        *out_evaluations_this_turn = manager->evaluations_this_turn;
    if (out_total_evaluations)
        *out_total_evaluations = manager->total_evaluations;
}

void rule_manager_debug_print(const rule_manager_t *manager) {
    if (!manager) {
        return;
    }

    printf("=== Rule Manager Debug Info ===\n");
    printf("Initialized: %s\n", manager->initialized ? "YES" : "NO");
    printf("Rules dirty: %s\n", manager->rules_dirty ? "YES" : "NO");
    printf("Evaluations this turn: %u\n", manager->evaluations_this_turn);
    printf("Total evaluations: %u\n", manager->total_evaluations);
    printf("Rules applied this turn: %u\n", manager->rules_applied_this_turn);

    rule_registry_print_stats(&manager->registry);
}

void rule_manager_debug_print_tile_rules(const rule_manager_t *manager,
                                         const tile_t *tile) {
    if (!manager || !tile) {
        return;
    }

    uint32_t tile_index = (tile->cell.coord.hex.r * 100 +
                           tile->cell.coord.hex.q); // TODO: proper indexing
    rule_registry_print_tile_rules(&manager->registry, tile_index);
}

// --- Utility Functions ---

bool rule_manager_needs_evaluation(const rule_manager_t *manager) {
    if (!manager) {
        return false;
    }

    return manager->rules_dirty;
}

void rule_manager_mark_dirty(rule_manager_t *manager) {
    if (!manager) {
        return;
    }

    manager->rules_dirty = true;
}

void rule_manager_mark_clean(rule_manager_t *manager) {
    if (!manager) {
        return;
    }

    manager->rules_dirty = false;
}

uint32_t rule_manager_get_rule_count(const rule_manager_t *manager) {
    if (!manager) {
        return 0;
    }

    return manager->registry.rule_count;
}

void rule_manager_set_batch_mode(rule_manager_t *manager, bool enabled) {
    if (!manager) {
        return;
    }

    rule_registry_set_batch_mode(&manager->registry, enabled);
}

void rule_manager_process_batch_updates(rule_manager_t *manager) {
    if (!manager) {
        return;
    }

    rule_registry_process_dirty_tiles(&manager->registry, &manager->context);
    manager->rules_dirty = false;
}
