# Rule Info Card System

A comprehensive visual representation system for game rules in the hexhex project.

## Overview

The Rule Info Card system provides a flexible way to display any game rule as a visual card with icons, colors, and human-readable descriptions. It supports multiple display configurations and integrates seamlessly with the Clay UI framework.

## Features

- **Visual Rule Representation**: Each rule is displayed with appropriate icons and color schemes
- **Multiple Display Modes**: Full, compact, and minimal card layouts
- **Human-Readable Text**: Automatic generation of descriptions, conditions, and effects
- **Rule Analysis**: Priority sorting and visual importance detection  
- **Flexible Configuration**: Customizable display options for different use cases
- **Unicode Icons**: Rich icon set for all rule types, scopes, targets, and conditions

## Quick Start

### Basic Usage

```c
#include "ui/rule_info_card.h"

// Create or obtain a rule_t structure
rule_t my_rule = create_some_rule();

// Render as a full info card
ui_render_rule_info_card(&my_rule, &RULE_CARD_CONFIG_FULL, CLAY_ID("my_rule_card"));
```

### Display Configurations

Three pre-defined configurations are available:

```c
// Full details with icons, descriptions, conditions, and effects
ui_render_rule_info_card(rule, &RULE_CARD_CONFIG_FULL, element_id);

// Compact layout with icons and effects only
ui_render_rule_info_card(rule, &RULE_CARD_CONFIG_COMPACT, element_id);

// Minimal display with just icons and rule name
ui_render_rule_info_card(rule, &RULE_CARD_CONFIG_MINIMAL, element_id);
```

### Custom Configuration

```c
rule_card_config_t custom_config = rule_card_config_create(
    true,   // show_icons
    false,  // show_descriptions  
    true    // compact_layout
);
custom_config.card_width = 300.0f;
custom_config.max_height = 150.0f;

ui_render_rule_info_card(rule, &custom_config, element_id);
```

### Rendering Multiple Rules

```c
const rule_t *rules[] = {&rule1, &rule2, &rule3};
uint32_t rule_count = 3;

// Sort by display priority first
rule_sort_by_priority(rules, rule_count);

// Render as a list
ui_render_rule_info_list(rules, rule_count, &RULE_CARD_CONFIG_COMPACT, CLAY_ID("rule_list"));
```

## Rule Types and Visual Representation

### Rule Type Icons

- **Neighbor Bonus** 👥: Rules that gain bonuses from adjacent tiles
- **Type Override** 🎭: Rules that change how tiles appear to others  
- **Range Modifier** 📏: Rules that modify interaction range
- **Pool Effect** 🌊: Rules that affect entire tile pools
- **Global Effect** 🌍: Rules that affect the entire board
- **Generic Rule** ⚙️: Other rule types

### Scope Icons

- **Self** 👤: Affects only the source tile
- **Neighbors** 👥: Affects adjacent tiles
- **Range** 🎯: Affects tiles within a specific range
- **Pool** 🌊: Affects the entire pool
- **Type Global** 🔗: Affects all tiles of a specific type
- **Board Global** 🌍: Affects the entire board

### Target Icons

- **Production** ⚡: Modifies production values
- **Range** 📏: Modifies interaction range
- **Pool Modifier** 💧: Modifies pool calculations
- **Type Override** 🎭: Changes perceived tile type
- **Placement Cost** 💰: Modifies placement costs
- **Movement Range** 🏃: Modifies movement capabilities

### Condition Icons

- **Always** ✅: Rule always applies
- **Self Type** 🔍: Depends on tile's own type
- **Neighbor Count** 📊: Depends on neighboring tiles
- **Pool Size** 📏: Depends on pool size
- **Board Count** 🔢: Depends on tiles on board
- **Production Threshold** ⚖️: Depends on production levels

### Effect Icons

- **Add Flat** ➕: Adds a fixed value
- **Add Scaled** 📈: Adds value based on conditions
- **Multiply** ✖️: Multiplies existing value
- **Set Value** 🎯: Sets to a specific value
- **Override Type** 🎭: Changes tile type
- **Modify Range** 📏: Changes range value

## Color Schemes

Rules are automatically colored based on their type:

- **Neighbor Bonus**: Green (76, 175, 80)
- **Range Modifier**: Blue (33, 150, 243)  
- **Type Override**: Orange (255, 152, 0)
- **Pool Effect**: Purple (156, 39, 176)
- **Global Effect**: Amber (255, 193, 7)
- **Generic Rule**: Blue Grey (96, 125, 139)

## Text Generation

The system automatically generates human-readable text for all rule components:

### Descriptions
```c
char buffer[256];
rule_generate_description(rule, buffer, sizeof(buffer));
// Result: "Gains production from nearby tiles"
```

### Conditions
```c
rule_generate_condition_text(rule, buffer, sizeof(buffer));
// Result: "With 2-3 Yellow neighbors"
```

### Effects
```c
rule_generate_effect_text(rule, buffer, sizeof(buffer));
// Result: "+2.0 Production"
```

## Rule Analysis

### Display Priority

Rules are automatically assigned display priorities based on:
- Visual importance (global effects, large values)
- Scope (global > pool > range > neighbors > self)
- Target type (type override > production > range > pool modifier)

```c
uint32_t priority = rule_get_display_priority(rule);
bool is_important = rule_is_visually_important(rule);
```

### Sorting

```c
const rule_t *rules[] = {&rule1, &rule2, &rule3};
rule_sort_by_priority(rules, 3);  // Sorts in descending priority order
```

## Integration Examples

### In Tile Info Cards

```c
void render_enhanced_tile_info(tile_t *tile, rule_manager_t *rule_manager) {
    // Your existing tile info code...
    
    // Add rule section
    const rule_t *affecting_rules[MAX_RULES_PER_TILE];
    uint32_t rule_count = get_rules_affecting_tile(rule_manager, tile, affecting_rules);
    
    if (rule_count > 0) {
        rule_sort_by_priority(affecting_rules, rule_count);
        ui_render_rule_info_list(affecting_rules, rule_count, 
                                &RULE_CARD_CONFIG_COMPACT, 
                                CLAY_ID("tile_rules"));
    }
}
```

### In Help/Reference UI

```c
void render_rule_reference() {
    // Display all possible rule types as examples
    const rule_t *example_rules[] = {
        &neighbor_bonus_example,
        &type_override_example,
        &global_modifier_example
    };
    
    ui_render_rule_info_list(example_rules, 3, &RULE_CARD_CONFIG_FULL, 
                            CLAY_ID("rule_reference"));
}
```

## API Reference

### Core Functions

- `rule_display_info_t rule_get_display_info(const rule_t *rule)` - Get complete display information
- `void ui_render_rule_info_card(const rule_t *rule, const rule_card_config_t *config, Clay_ElementId element_id)` - Render single rule card
- `void ui_render_rule_info_list(const rule_t **rules, uint32_t rule_count, const rule_card_config_t *config, Clay_ElementId container_id)` - Render rule list

### Text Generation

- `int rule_generate_description(const rule_t *rule, char *buffer, size_t buffer_size)` - Generate full description
- `int rule_generate_condition_text(const rule_t *rule, char *buffer, size_t buffer_size)` - Generate condition text
- `int rule_generate_effect_text(const rule_t *rule, char *buffer, size_t buffer_size)` - Generate effect text

### Analysis Functions

- `uint32_t rule_get_display_priority(const rule_t *rule)` - Get display priority
- `bool rule_is_visually_important(const rule_t *rule)` - Check if rule should be highlighted
- `void rule_sort_by_priority(const rule_t **rules, uint32_t rule_count)` - Sort rules by priority

### Icon Functions

- `const char* rule_get_type_icon(const rule_t *rule)` - Get main rule icon
- `const char* rule_get_scope_icon(rule_scope_t scope)` - Get scope icon
- `const char* rule_get_target_icon(rule_target_t target)` - Get target icon
- `const char* rule_get_condition_icon(rule_condition_type_t condition_type)` - Get condition icon
- `const char* rule_get_effect_icon(rule_effect_type_t effect_type)` - Get effect icon

### Configuration

- `rule_card_config_t rule_card_config_create(bool show_icons, bool show_descriptions, bool compact)` - Create custom config
- Pre-defined configs: `RULE_CARD_CONFIG_FULL`, `RULE_CARD_CONFIG_COMPACT`, `RULE_CARD_CONFIG_MINIMAL`

## Files

- `include/ui/rule_info_card.h` - Header with all function declarations
- `src/ui/rule_info_card.c` - Implementation
- `examples/rule_info_card_demo.c` - Usage examples
- `tests/test_rule_info_card.c` - Test program

## Testing

Build and run the test program to verify functionality:

```bash
# Build the test program (adjust paths as needed)
gcc -I./include -I./src tests/test_rule_info_card.c src/ui/rule_info_card.c -o test_rule_cards

# Run tests
./test_rule_cards
```

## Dependencies

- Clay UI framework for rendering
- Game rule system (`game/rule_system.h`)
- Standard C library for text manipulation

## Notes

- All text generation is done at runtime - no static strings are stored
- Unicode icons require proper font support in your rendering system
- Colors use Clay's RGBA color format
- Thread-safe for read-only operations on rule data