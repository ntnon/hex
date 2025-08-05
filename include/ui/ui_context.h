#define CLAY_IMPLEMENTATION
#include "../include/third_party/clay.h" // UI system

#ifndef UI_CONTEXT_H

#define UI_CONTEXT_H

#include "../third_party/clay.h"
#include "../types.h"

// Common state for UI screens
typedef struct {
    bool is_open;
    Clay_ID active_element;
} ui_screen_state_t;

// Function pointer types for UI handlers
typedef void (*ui_init_fn)(struct ui_context*);
typedef void (*ui_cleanup_fn)(struct ui_context*);
typedef void (*ui_render_fn)(struct ui_context*);
typedef void (*ui_update_fn)(struct ui_context*, input_state_t*);

// Handler collection for each screen
typedef struct {
    ui_screen_state_t* state;
    ui_init_fn init;
    ui_cleanup_fn cleanup;
    ui_render_fn render;
    ui_update_fn update;
} ui_screen_handlers_t;

// Main UI context structure
typedef struct ui_context {
    Clay_Context* context;

    // Screen-specific states
    ui_screen_state_t menu;
    ui_screen_state_t settings;
    struct {
        ui_screen_state_t base;
    } game;

    // Current screen type
    screen_type_t current_screen;

    // Screen handlers
    ui_screen_handlers_t handlers[SCREEN_MAX];
} ui_context_t;

// Button configuration for shared UI elements
typedef struct {
    const char* label;
    Clay_ID id;
    int action;
} ui_button_config_t;

// Initialize the UI context and create Clay context
void ui_context_init(ui_context_t* ctx);

// Clean up UI resources and destroy Clay context
void ui_context_cleanup(ui_context_t* ctx);

// Begin a new UI frame, should be called at start of each render loop
void ui_context_begin_frame(ui_context_t* ctx);

// End the UI frame and flush Clay commands
void ui_context_end_frame(ui_context_t* ctx);

// Helper to center a container on screen
void ui_center_container(ui_context_t* ctx, Clay_Container* container);

// Shared UI element rendering functions
void ui_draw_button_list(ui_context_t* ctx,
                        const char* title,
                        const ui_button_config_t* buttons,
                        int button_count,
                        Clay_ID* active_button);

// Screen transition helper
void ui_switch_screen(ui_context_t* ctx, screen_type_t new_screen);

#endif // UI_CONTEXT_H
