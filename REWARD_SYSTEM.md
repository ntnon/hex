# Hexhex Reward System

The reward system allows players to select from procedurally generated rule-based rewards that modify game mechanics. Rewards are presented as rules to the game engine but appear as power-ups to the player.

## Overview

- **Player Perspective**: Choose from 3 reward options with different effects and rarities
- **System Perspective**: Rules that modify tile behavior, production, range, and interactions
- **Integration**: Seamlessly integrates with the existing rule system and UI

## Quick Start

1. **Trigger Rewards**: Press `SPACEBAR` in-game or rewards trigger automatically at turn end
2. **Select Reward**: Click on one of the 3 reward cards presented
3. **Confirm**: Click "Confirm Selection" to apply the reward permanently
4. **Effects**: The reward becomes an active rule affecting gameplay

## Reward Categories

### 🟢 Production
- **Effect**: Increase tile output and resource generation
- **Examples**: "Neighbor Bonus" (+1 per adjacent green tile), "High Value Boost" (+50% for tiles >5)
- **Player Impact**: More points, faster progression

### 🎯 Range  
- **Effect**: Modify tile influence radius
- **Examples**: "Extended Reach" (+1 range), "Focused Power" (-1 range, +2 production)
- **Player Impact**: Change tactical positioning and tile placement strategy

### 🔗 Synergy
- **Effect**: Create interactions between different tile types
- **Examples**: "Color Harmony" (yellow tiles count as green), "Type Mastery" (all types +1 in pools)
- **Player Impact**: Enable new combination strategies

### 🌍 Global
- **Effect**: Board-wide modifications
- **Examples**: "Universal Amplifier" (+25% all production), "Range Mastery" (+1 range globally)
- **Player Impact**: Fundamental gameplay changes

### ✨ Special
- **Effect**: Unique mechanics and game-changing effects
- **Examples**: "Pool Scaling" (+0.5 per tile in pool), "Cascade Effect" (adjacent tiles trigger each other)
- **Player Impact**: Unlock entirely new gameplay patterns

## Rarity System

### Common (Gray)
- **Frequency**: High
- **Power Level**: 1.5-2.5
- **Availability**: All player levels
- **Examples**: Basic production bonuses, small range increases

### Uncommon (Green)
- **Frequency**: Medium
- **Power Level**: 2.5-4.0  
- **Availability**: Player level 3+
- **Examples**: Type synergies, conditional bonuses

### Rare (Blue)
- **Frequency**: Low
- **Power Level**: 4.0-6.0
- **Availability**: Player level 5+
- **Examples**: Global modifiers, significant range changes

### Legendary (Orange)
- **Frequency**: Very Low
- **Power Level**: 6.0-10.0
- **Availability**: Player level 10+
- **Examples**: Game-breaking effects, unique mechanics

## Technical Architecture

### Core Components

```
reward_system.h/c    - Reward generation and selection logic
reward_state.h/c     - UI state management and transitions
game.h/c            - Integration with main game loop
ui_layout.c         - Reward selection UI rendering
event_router.c      - Input handling for reward interactions
```

### Data Flow

1. **Trigger**: Game event or manual trigger calls `game_trigger_reward_selection()`
2. **Generation**: `reward_system_generate_options()` creates 3 rewards based on game state
3. **Display**: UI renders reward cards with titles, descriptions, and visual styling
4. **Selection**: Player clicks trigger `reward_state_select_option()`
5. **Confirmation**: `reward_state_confirm_selection()` applies the rule to the game
6. **Integration**: Rule is added to `rule_manager` and affects future gameplay

### Rule Generation

Rewards are generated using templates that create rule structures:

```c
// Example: Neighbor bonus rule generation
rule_t rule = {
  .scope = RULE_SCOPE_NEIGHBORS,
  .target = RULE_TARGET_PRODUCTION,
  .condition_type = RULE_CONDITION_NEIGHBOR_COUNT,
  .effect_type = RULE_EFFECT_ADD_SCALED,
  .effect_params.scaled.scale_factor = 2.0f
};
```

## UI Integration

### Reward Selection Screen

- **Header**: Shows trigger reason and current phase
- **Cards**: 3 reward options with rarity colors, icons, and descriptions
- **Buttons**: Confirm selection, Skip reward (if available)
- **Animations**: Hover effects, selection highlighting, phase transitions

### Visual Design

- **Rarity Colors**: Border colors indicate reward power level
- **Category Icons**: Unicode symbols represent reward types (⚡🎯🔗🌍✨)
- **Layout**: Responsive card layout with consistent spacing
- **Feedback**: Visual confirmation of selections and state changes

## Controls

### Keyboard
- `SPACEBAR` - Trigger reward selection (debug/manual)
- `1/2/3` - Select reward option by number
- `ENTER` - Confirm current selection
- `ESC` - Skip reward (if allowed)

### Mouse
- **Click Card** - Select reward option
- **Click Confirm** - Apply selected reward
- **Click Skip** - Dismiss reward selection
- **Hover** - Preview reward effects

## Configuration

### Reward Generation Parameters

```c
reward_generation_params_t params = {
  .min_rarity = REWARD_RARITY_COMMON,
  .max_rarity = REWARD_RARITY_RARE,  // Based on player level
  .ensure_variety = true,            // Avoid duplicate categories
  .balance_power = true,             // Balance overall reward strength
  .seed = current_turn_seed          // Deterministic generation
};
```

### State Configuration

```c
reward_state_configure(
  reward_state,
  true,    // auto_advance_phases
  2.0f,    // min_presentation_time
  true,    // require_confirmation
  false    // allow_skip
);
```

## Testing

### Unit Tests
```bash
./test_reward_system      # Core functionality tests
./demo_rewards           # Interactive demonstration
```

### Integration Tests
- Reward generation with various game states
- Selection and confirmation workflow
- Rule application and effect verification
- UI interaction and state management

## Performance Considerations

### Generation
- Templates pre-compiled at initialization
- O(1) template lookup by category/rarity
- Minimal memory allocation during generation

### Selection
- No dynamic memory allocation during UI updates
- Cached display information for smooth animations
- Efficient state transitions

### Rule Integration
- Rules added to existing rule manager
- No performance impact on core game loop
- Batch processing for multiple rule applications

## Extensibility

### Adding New Reward Types

1. **Create Template**: Define in `REWARD_TEMPLATES` array
2. **Generation Function**: Implement rule creation logic
3. **Display Info**: Add title, description, and flavor text
4. **Testing**: Add test cases for new reward type

### Custom Triggers

```c
// Add new trigger type to enum
typedef enum {
  REWARD_TRIGGER_CUSTOM_EVENT,
  // ... existing triggers
} reward_trigger_t;

// Implement trigger logic
if (custom_condition_met) {
  game_trigger_reward_selection(game, REWARD_TRIGGER_CUSTOM_EVENT);
}
```

### UI Customization

- Modify card layouts in `ui_build_reward_option()`
- Add new animations in `reward_state_update_animations()`
- Customize colors and styling in reward rendering functions

## Debugging

### Console Output
```
Triggered reward selection
Generated 3 reward options
Reward option 0 clicked
Confirm button clicked
Applied reward: Neighbor Bonus (Rule ID: 42)
```

### Debug Functions
```c
reward_system_debug_print(system);      // Print system state
reward_state_debug_print(state);        // Print UI state
rule_manager_debug_print_tile_rules();  // Show active rules
```

### Common Issues

1. **No Rewards Generated**: Check player level vs template requirements
2. **Selection Not Working**: Verify UI element IDs match event handling
3. **Rules Not Applying**: Check rule manager initialization and integration
4. **UI Not Showing**: Ensure game state is GAME_STATE_REWARD

## Future Enhancements

### Planned Features
- **Reward History**: Track previously selected rewards
- **Reward Synergies**: Rewards that interact with each other  
- **Dynamic Rarity**: Adjust rarity based on game performance
- **Reward Tooltips**: Detailed hover information
- **Animation Polish**: Enhanced visual effects and transitions

### Potential Additions
- **Negative Rewards**: Risk/reward mechanics with drawbacks
- **Temporary Rewards**: Time-limited effects
- **Conditional Rewards**: Rewards that activate under specific conditions
- **Player Preferences**: Learn and adapt to player choices

## API Reference

### Key Functions

```c
// Reward System
bool reward_system_init(reward_system_t *system, uint32_t seed);
bool reward_system_generate_options(reward_system_t *system, const reward_generation_params_t *params);
bool reward_system_select_option(reward_system_t *system, uint8_t option_index);
bool reward_system_confirm_selection(reward_system_t *system, rule_t *out_rule);

// Reward State  
bool reward_state_init(reward_state_t *state, uint32_t seed);
bool reward_state_enter(reward_state_t *state, game_t *game, reward_trigger_t trigger);
void reward_state_update(reward_state_t *state, game_t *game, float delta_time);
bool reward_state_is_active(const reward_state_t *state);

// Game Integration
bool game_trigger_reward_selection(game_t *game, int trigger);
bool game_handle_reward_input(game_t *game, const input_state_t *input);
bool game_is_in_reward_state(const game_t *game);
```

The reward system is designed to be both powerful for the game engine and intuitive for players, providing a seamless way to modify and enhance gameplay through rule-based rewards.