# Clay UI System Documentation

A clean, simplified UI system built with Clay and Raylib for the HexHex game.

## Overview

This Clay UI system replaces the complex screen management with a focused, declarative UI approach. It provides:

- **Simple screen management** - Just 4 screen types (Menu, Game, Settings, Game End)
- **Declarative layout** - Use Clay's layout system for responsive UIs
- **Raylib integration** - Direct rendering through existing Raylib renderer
- **Clean separation** - UI logic isolated from game logic

## Architecture

```
clay_ui_t
├── Screen Management (current_screen, transitions)
├── Clay Memory Arena (layout calculations)
├── Input Handling (mouse, keyboard)
└── Rendering (via existing Clay-Raylib renderer)
```

## Quick Start

### 1. Initialize the UI System

```c
#include "ui/clay_ui.h"

clay_ui_t ui;
if (!clay_ui_init(&ui, screen_width, screen_height)) {
    // Handle initialization error
    return -1;
}
```

### 2. Main Loop Integration

```c
while (!WindowShouldClose() && !clay_ui_should_quit(&ui)) {
    // Handle window resize
    if (IsWindowResized()) {
        clay_ui_resize(&ui, GetScreenWidth(), GetScreenHeight());
    }
    
    // Update Clay pointer state (required for interactions)
    Clay_SetPointerState((Clay_Vector2){GetMouseX(), GetMouseY()}, 
                         IsMouseButtonDown(MOUSE_BUTTON_LEFT));
    
    // Handle input
    input_state_t input;
    get_input_state(&input);
    clay_ui_handle_input(&ui, &input);
    
    // Render
    BeginDrawing();
    ClearBackground(RAYWHITE);
    clay_ui_render(&ui);
    EndDrawing();
}
```

### 3. Cleanup

```c
clay_ui_cleanup(&ui);
```

## API Reference

### Core Functions

#### `clay_ui_init(clay_ui_t *ui, float width, float height)`
- Initializes the Clay UI system
- Allocates memory arena for Clay
- Sets up Clay context and measure text function
- Returns `true` on success

#### `clay_ui_cleanup(clay_ui_t *ui)`
- Frees Clay memory arena
- Cleans up all UI resources

#### `clay_ui_resize(clay_ui_t *ui, float width, float height)`
- Updates UI for new screen dimensions
- Call when window is resized

### Screen Management

#### `clay_ui_switch_to(clay_ui_t *ui, ui_screen_type_t screen)`
- Switches to a different screen
- Handles transition logic automatically

#### `clay_ui_should_quit(clay_ui_t *ui)`
- Returns `true` if user requested quit
- Check this in your main loop

### Input Handling

#### `clay_ui_handle_input(clay_ui_t *ui, input_state_t *input)`
- Processes keyboard input for screen navigation
- Handles ESC key for settings/back navigation

### Rendering

#### `clay_ui_render(clay_ui_t *ui)`
- Renders current screen using Clay layout
- Uses existing Clay-Raylib renderer automatically
- Call between `BeginDrawing()` and `EndDrawing()`

## Screen Types

The system supports 4 main screen types:

### `UI_SCREEN_MENU`
- Main menu with Start, Settings, and Quit buttons
- Entry point for the application

### `UI_SCREEN_GAME` 
- Game screen with minimal UI overlay
- Contains game area placeholder for your game content
- Top bar shows game title

### `UI_SCREEN_SETTINGS`
- Settings overlay with Resume and Main Menu options
- Semi-transparent background
- Accessible from game screen via ESC

### `UI_SCREEN_GAME_END`
- Game over screen
- Play Again and Main Menu buttons

## Customization

### Adding New Screens

1. Add new enum value to `ui_screen_type_t` in `clay_ui.h`:
```c
typedef enum {
    UI_SCREEN_MENU,
    UI_SCREEN_GAME,
    UI_SCREEN_SETTINGS,
    UI_SCREEN_GAME_END,
    UI_SCREEN_YOUR_NEW_SCREEN,  // Add here
    UI_SCREEN_COUNT
} ui_screen_type_t;
```

2. Add render function declaration in `clay_ui.h`:
```c
void clay_ui_render_your_new_screen(clay_ui_t *ui);
```

3. Implement render function in `clay_ui.c`:
```c
void clay_ui_render_your_new_screen(clay_ui_t *ui) {
    CLAY({.id = CLAY_ID("YourScreen"),
          .layout = {.sizing = {CLAY_SIZING_GROW(), CLAY_SIZING_GROW()}}}) {
        // Your UI layout here
    }
}
```

4. Add case to `clay_ui_render()` switch statement:
```c
case UI_SCREEN_YOUR_NEW_SCREEN:
    clay_ui_render_your_new_screen(ui);
    break;
```

### Styling

Colors are defined as constants at the top of `clay_ui.c`:

```c
const Clay_Color COLOR_BACKGROUND = (Clay_Color){245, 245, 245, 255};
const Clay_Color COLOR_BUTTON_PRIMARY = (Clay_Color){100, 150, 200, 255};
const Clay_Color COLOR_BUTTON_SECONDARY = (Clay_Color){120, 120, 120, 255};
const Clay_Color COLOR_BUTTON_DANGER = (Clay_Color){200, 100, 100, 255};
// ... modify these to change the look
```

### Custom Layouts

Use Clay's declarative syntax for layouts:

```c
CLAY({.id = CLAY_ID("MyContainer"),
      .layout = {
          .sizing = {CLAY_SIZING_GROW(), CLAY_SIZING_FIXED(60)},
          .layoutDirection = CLAY_LEFT_TO_RIGHT,
          .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER},
          .childGap = 10,
          .padding = {10, 10, 10, 10}
      },
      .backgroundColor = COLOR_BACKGROUND}) {
    
    // Child elements here
    CLAY_TEXT(CLAY_STRING("Hello World!"),
              CLAY_TEXT_CONFIG({.fontSize = 24, .textColor = COLOR_TEXT_DARK}));
}
```

## Integration with Existing Game

### Game Content Area

The `UI_SCREEN_GAME` includes a game area container:

```c
CLAY({.id = CLAY_ID("GameArea"),
      .layout = {.sizing = {CLAY_SIZING_GROW(), CLAY_SIZING_GROW()}}}) {
    // Your game content renders here
    // You can call your existing game rendering functions inside this block
}
```

### Replacing Screen Manager

To replace your existing screen management:

1. Remove old screen manager includes from `main.c`
2. Replace screen manager initialization with Clay UI init
3. Replace screen callbacks with Clay UI render loop
4. Remove old screen system files

### Memory Usage

The Clay UI system uses ~1MB of memory by default (`CLAY_MEMORY_SIZE_MB`). This can be adjusted in `clay_ui.c` if needed.

## Testing

Use the included test system to verify Clay UI functionality:

```c
#include "clay_ui_test.h"

// Run standalone test
int main() {
    return clay_ui_test_main();
}

// Or integrate into existing app
if (test_mode) {
    run_clay_ui_test_mode();
}
```

## Dependencies

- **Clay** - UI layout library (included in `third_party/clay.h`)
- **Raylib** - Graphics and input (your existing dependency)
- **Standard C** - malloc, string functions

## File Structure

```
include/ui/clay_ui.h           # Public API
src/ui/clay_ui.c              # Implementation
src/clay_ui_test.c            # Test/example code
include/clay_ui_test.h        # Test header
```

## Error Handling

The system includes basic error handling:

- Memory allocation failures return `false` from init
- Clay errors are printed to console via `HandleClayErrors()`
- Null pointer checks on public API functions

## Performance Notes

- Clay recalculates layout each frame (fast for UI complexity)
- Text measurement is cached by Clay automatically
- Memory allocation happens only during init
- No dynamic allocations during render loop

## Troubleshooting

### "Clay Error" messages
- Check Clay element IDs are unique
- Ensure proper nesting of Clay elements
- Verify layout sizing constraints

### Buttons not responding
- Ensure `Clay_SetPointerState()` is called each frame
- Check mouse coordinates are correct
- Verify button IDs match in `Clay_PointerOver()` calls

### Layout issues
- Clay uses immediate mode - recalculate each frame
- Check sizing constraints (GROW, FIXED, etc.)
- Verify parent-child relationships

For more detailed Clay documentation, see the official Clay repository.