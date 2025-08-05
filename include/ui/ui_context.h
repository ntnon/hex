#ifndef UI_CONTEXT_H
#define UI_CONTEXT_H

#include "../third_party/clay.h"
#include "../types.h"
#include "../controller/input_state.h"

typedef enum {
    SCREEN_MENU,
    SCREEN_GAME,
    SCREEN_SETTINGS
} game_screen_t;

typedef struct {
    game_screen_t current_screen;
    uint32_t active_element;
    bool settings_overlay_open;
} ui_context_t;

// Core UI functions
void ui_context_init(ui_context_t* ctx);
void ui_context_cleanup(ui_context_t* ctx);

// Screen management
void ui_switch_screen(ui_context_t* ctx, game_screen_t new_screen);

// Input handlers
void handle_menu_input(ui_context_t* ctx, input_state_t* input);
void handle_game_input(ui_context_t* ctx, input_state_t* input);
void handle_settings_input(ui_context_t* ctx, input_state_t* input);

// Render functions
void render_menu(ui_context_t* ctx);
void render_game(ui_context_t* ctx);
void render_settings(ui_context_t* ctx);

#endif // UI_CONTEXT_H
