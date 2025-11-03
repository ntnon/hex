#include <stdint.h>
#include <stdlib.h>
#include "game/rule.h"
#include "game/board.h"

typedef struct {
    rule_t *rules;
    size_t count;
    size_t capacity;
    uint32_t next_id;
} rule_registry_t;

// Core API - minimal and focused
rule_registry_t* rule_registry_create(void);
void rule_registry_destroy(rule_registry_t *registry);

// Add/remove rules
uint32_t rule_add(rule_registry_t *registry, const rule_t *rule);
void rule_remove(rule_registry_t *registry, uint32_t id);
void rule_remove_by_source(rule_registry_t *registry, grid_cell_t source);

// Apply instant rules (modifies base values directly)
void rule_apply_instant(rule_registry_t *registry, rule_t *rule,
                       board_t *board, tile_t *tile);

// Calculate values with persistent rules (no caching yet)
float rule_calc_production(rule_registry_t *registry, board_t *board,
                          tile_t *tile, grid_cell_t cell);
float rule_calc_pool_multiplier(rule_registry_t *registry, board_t *board,
                               pool_t *pool);

// Execute phase-based rules
void rule_execute_phase(rule_registry_t *registry, board_t *board,
                       rule_phase_t phase);

// Helper to create common rules
rule_t rule_make_neighbor_bonus(grid_cell_t source, tile_type_t neighbor_type,
                               float bonus_per);
rule_t rule_make_pool_multiplier(grid_cell_t source, float multiplier);
rule_t rule_make_production_boost(grid_cell_t source, float boost);
