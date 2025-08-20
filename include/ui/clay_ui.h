#ifndef CLAY_UI_H
#define CLAY_UI_H

#include "../third_party/clay.h"
#include "../game/input_state.h"
#include <stdbool.h>

typedef enum {
    UI_SCREEN_MENU,
    UI_SCREEN_GAME,
    UI_SCREEN_SETTINGS,
    UI_SCREEN_GAME_END,
    UI_SCREEN_COUNT
} ui_screen_type_t;

typedef struct {
    ui_screen_type_t current_screen;
    ui_screen_type_t next_screen;
    bool screen_changed;
    bool quit_requested;

    // Clay context
    Clay_Arena clay_memory;
    uint32_t clay_memory_size;

    // Screen dimensions
    float screen_width;
    float screen_height;
} clay_ui_t;

// Core UI functions
bool clay_ui_init(clay_ui_t *ui, float screen_width, float screen_height);
void clay_ui_cleanup(clay_ui_t *ui);
void clay_ui_resize(clay_ui_t *ui, float width, float height);

// Screen management
void clay_ui_switch_to(clay_ui_t *ui, ui_screen_type_t screen);
bool clay_ui_should_quit(clay_ui_t *ui);

// Main update and render
void clay_ui_handle_input(clay_ui_t *ui, input_state_t *input);
void clay_ui_render(clay_ui_t *ui);

// Individual screen renders (called internally)
void clay_ui_render_menu(clay_ui_t *ui);
void clay_ui_render_game(clay_ui_t *ui);
void clay_ui_render_settings(clay_ui_t *ui);
void clay_ui_render_game_end(clay_ui_t *ui);

#endif // CLAY_UI_H
