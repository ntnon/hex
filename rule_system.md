Hex Game Rule System Architecture Summary

## Core Concepts

### Rule Lifecycles
- **Instant Rules**: Execute once, permanently modify base values, then self-destruct
  - Examples: "Increase production by 1", "Pool modifier +0.1x"
  - Target: `tile->data.value` or `pool->base_multiplier`
- **Persistent Rules**: Stay active, evaluated during each calculation cycle
  - Examples: "+1 production per yellow neighbor", "Neighbors appear as same color"
  - Target: Runtime calculations

### Rule Phases (for Persistent Rules)
- **Perception Phase**: Modify how game state is "seen" (color overrides, neighbor modifications)
- **Calculation Phase**: Apply bonuses/penalties using the effective (post-perception) state

### Rule Components
```c
typedef struct {
    rule_lifecycle_t lifecycle;    // INSTANT or PERSISTENT
    rule_phase_t phase;           // PERCEPTION or CALCULATION
    rule_scope_t scope;           // TILE, POOL, NEIGHBORS, GLOBAL
    rule_condition_t condition;    // When rule applies
    rule_effect_t effect;         // What the rule does
    rule_target_t target;         // BASE_VALUE, POOL_MODIFIER, RUNTIME_CALC
} rule_t;
```

## Architecture Design

### Centralized Rule Registry
- **No rule clutter**: Core `tile_t` and `pool_t` structures remain clean
- **Spatial indexing**: Hash map for O(1) lookup of rules affecting any cell
- **Memory efficient**: Only allocate rule storage when needed
- **Easy cleanup**: Remove rules when source tiles are destroyed

### Rule Application Flow

**Instant Rules:**
1. Execute immediately when triggered
2. Permanently modify base values
3. Mark as executed and garbage collect

**Persistent Rules:**
1. Store in centralized registry with spatial indexing
2. During production calculation:
   - Phase 1: Apply perception rules → create effective game state
   - Phase 2: Apply calculation rules → compute final values

## Key Benefits

- **Stacking**: Multiple rules can target the same tile/pool without conflicts
- **Order deterministic**: Perception rules always run before calculation rules
- **Performance**: Efficient spatial queries, cache-friendly core structures
- **Modularity**: Easy to add/remove/modify rules without touching game entities
- **Scalability**: System handles complex rule interactions (e.g., color overrides affecting neighbor bonuses)

## Example Interaction
```
Tile A: "All neighbors appear red" (Persistent, Perception)
Tile B: "+1 per same-color neighbor" (Persistent, Calculation)

Evaluation:
1. Perception: Tile A makes neighbors appear red to everyone
2. Calculation: Tile B counts "red" neighbors (even if originally blue)
   and gains production accordingly
```

This architecture provides a flexible, performant foundation for complex rule-based gameplay while keeping core game structures clean and maintainable.
