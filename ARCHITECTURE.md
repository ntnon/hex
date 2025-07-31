# HexHex Game Architecture Documentation

## Overview

This document describes the modular, extensible architecture of the HexHex game. The architecture is designed to be highly readable, easily expandable, and maintainable, with clear separation of concerns between different systems.

## Core Architecture Principles

1. **Separation of Concerns**: Each module has a single, well-defined responsibility
2. **Extensibility**: New screens and input areas can be added without modifying existing code
3. **Consistency**: All components follow the same naming conventions and patterns
4. **Modularity**: Components are loosely coupled and can be tested independently
5. **Clean Main Loop**: main.c contains minimal logic and delegates to specialized systems

## System Components

### 1. Screen Manager (`screen_manager.h/.c`)

**Purpose**: Manages the current active screen and transitions between screens.

**Key Functions**:
- `screen_manager_init()` - Initialize with default screen
- `screen_manager_switch()` - Switch to a new screen
- `screen_manager_get_current()` - Get currently active screen

**Screens**:
- `SCREEN_MENU` - Main menu
- `SCREEN_GAME` - Game screen
- `SCREEN_PAUSE` - Pause overlay
- `SCREEN_GAMEOVER` - Game over screen

### 2. Input Controller (`input_controller.h/.c`)

**Purpose**: Centralized input handling system that delegates to screen-specific handlers.

**Key Functions**:
- `input_controller_init()` - Initialize the controller
- `input_controller_register_screen()` - Register handlers for a screen
- `input_controller_update()` - Process input and actions for current screen
- `input_controller_render()` - Render the current screen

**Handler Types**:
- `input_handler_fn` - Processes raw input (mouse, keyboard) for a screen
- `action_handler_fn` - Handles actions/intents set by input handlers
- `render_handler_fn` - Renders the screen

### 3. Screen Implementations

Each screen follows the same pattern for consistency and maintainability:

#### Pattern Structure:
```c
// Data structure
typedef struct {
    // Screen-specific data
    screen_action_t last_action;
} screen_name_t;

// Core functions
void screen_name_init(screen_name_t *screen);
void screen_name_update(screen_name_t *screen, ...);
void screen_name_draw(const screen_name_t *screen);
void screen_name_unload(screen_name_t *screen);

// Input controller integration
void screen_name_input_handler(void *screen_data);
void screen_name_action_handler(void *screen_data, void *mgr, bool *running);
void screen_name_render_handler(void *screen_data);
```

#### Current Screen Implementations:

**Menu Screen** (`UI/menu.h/.c`):
- Handles main menu navigation
- Supports mouse and keyboard input
- Transitions to game or quits application

**Game Screen** (`game/game.h/.c`):
- Manages game state (board, inventory)
- Handles multiple input areas (board, inventory, pause)
- Integrates with board input controller for hex grid interaction

**Pause Screen** (`UI/pause.h/.c`):
- Overlay screen with semi-transparent background
- Options to resume, return to menu, or quit
- Keyboard shortcuts for quick actions

### 4. Board Input Controller (`board/board_input_controller.h/.c`)

**Purpose**: Specialized input handling for the hex grid board.

**Features**:
- Pan and zoom controls
- Hex cell hover detection
- Tile placement and selection
- Configurable zoom limits

**Integration**: Called by the game screen's input handler, demonstrating how multiple input areas can coexist.

### 5. Game Systems

**Inventory System** (`game/inventory.h/.c`):
- Manages player's tile inventory
- Keyboard shortcuts for selection (1-9 keys)
- Visual rendering with selection highlighting

**Board System** (`board/board.h/.c`):
- Hex grid management
- Tile placement and interaction
- Pool and resource management

## Data Flow

```
Main Loop
    ↓
Input Controller
    ↓
Screen-Specific Input Handler
    ↓ (sets action/intent)
Screen-Specific Action Handler
    ↓ (performs transitions/updates)
Screen-Specific Render Handler
    ↓
Screen Rendering
```

## Adding New Screens

To add a new screen (e.g., Options screen):

1. **Create header file** (`include/UI/options.h`):
```c
#ifndef OPTIONS_H
#define OPTIONS_H

typedef enum {
    OPTIONS_ACTION_NONE,
    OPTIONS_ACTION_BACK,
    // ... other actions
} options_action_t;

typedef struct {
    options_action_t last_action;
    // ... screen data
} options_screen_t;

void options_screen_init(options_screen_t *screen);
void options_input_handler(void *screen_data);
void options_action_handler(void *screen_data, void *mgr, bool *running);
void options_render_handler(void *screen_data);

#endif
```

2. **Implement source file** (`src/UI/options.c`):
```c
#include "../../include/UI/options.h"
// ... implementation following the established pattern
```

3. **Add to screen manager** (`include/game/screen_manager.h`):
```c
typedef enum {
    SCREEN_MENU,
    SCREEN_GAME,
    SCREEN_PAUSE,
    SCREEN_OPTIONS,  // Add new screen
    NUM_SCREENS
} screen_type_t;
```

4. **Register in main.c**:
```c
options_screen_t options_screen;
options_screen_init(&options_screen);

input_controller_register_screen(&input_ctrl, SCREEN_OPTIONS,
                                options_input_handler, options_action_handler,
                                options_render_handler, &options_screen);
```

## Adding New Input Areas

To add a new input area within an existing screen:

1. **Create specialized controller** (e.g., `ui_input_controller.h/.c`)
2. **Call from screen's input handler**:
```c
void game_input_handler(void *screen_data) {
    game_screen_t *game = (game_screen_t *)screen_data;
    
    // Handle board input
    board_input_controller_update(game->board_controller, ...);
    
    // Handle UI input
    ui_input_controller_update(game->ui_controller, ...);
    
    // Handle inventory input
    // ... etc
}
```

## Naming Conventions

### Files and Directories:
- **snake_case** for file names
- **Logical grouping** in directories (`UI/`, `game/`, `board/`, etc.)

### Functions:
- **snake_case** for all functions
- **Consistent prefixes** by module (e.g., `screen_manager_`, `input_controller_`)
- **Descriptive names** that clearly indicate purpose

### Types:
- **snake_case** with `_t` suffix for all types
- **Consistent enum naming** with module prefix (e.g., `MENU_ACTION_START`)

### Variables:
- **snake_case** for all variables
- **Descriptive names** that indicate purpose and scope

## Memory Management

- **Initialization**: All screens have `_init()` functions
- **Cleanup**: All screens have `_unload()` or `_destroy()` functions
- **Ownership**: Each screen owns its data and is responsible for cleanup
- **RAII Pattern**: Resources are acquired in init and released in destroy

## Error Handling

- **Null Checks**: All public functions check for null parameters
- **Graceful Degradation**: System continues to function even if individual components fail
- **Early Returns**: Functions return early on invalid input

## Testing Strategy

- **Unit Testing**: Each module can be tested independently
- **Integration Testing**: Screen transitions and input handling
- **Separation**: Business logic separated from input/rendering for easier testing

## Performance Considerations

- **Minimal Allocations**: Most data structures are stack-allocated
- **Efficient Rendering**: Each screen only renders what's necessary
- **Input Polling**: Centralized input polling prevents redundant system calls
- **Modular Updates**: Only active screen's systems are updated

## Future Enhancements

- **State Stack**: For overlapping screens (e.g., popup menus)
- **Animation System**: Smooth transitions between screens
- **Configuration System**: Runtime customization of controls and settings
- **Save/Load System**: Persistent game state management
- **Localization**: Multi-language support through string tables

This architecture provides a solid foundation for the HexHex game while maintaining flexibility for future development and ensuring code maintainability.