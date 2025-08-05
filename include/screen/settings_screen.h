#ifndef SETTINGS_SCREEN_H
#define SETTINGS_SCREEN_H

#include "screen_manager.h"
#include "../controller/input_state.h"
#include "../types.h"

typedef enum {
    SETTINGS_ACTION_NONE,
    SETTINGS_ACTION_RESUME,
    SETTINGS_ACTION_MENU,
    SETTINGS_ACTION_QUIT
} settings_action_t;

typedef struct {
    settings_action_t last_action;
} settings_screen_t;

void settings_screen_init(settings_screen_t* settings, int width, int height);
void settings_screen_unload(void* screen_data);

// Handlers for generic screen callback system
void settings_input_handler(void* screen_data, input_state_t* input);
void settings_action_handler(void* screen_data, screen_manager_t* mgr, bool* running);
void settings_render_handler(void* screen_data);

#endif // SETTINGS_SCREEN_H