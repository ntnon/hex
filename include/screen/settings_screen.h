#ifndef settings_H
#define settings_H

#include "screen_manager.h"

//#include "raylib.h"
#include <stdbool.h>

typedef enum {
    _NONE,
SETTINGS_ACTION_RESUME,
SETTINGS_ACTION_MENU,
SETTINGS_ACTION_NONE,
SETTINGS_ACTION_QUIT
} settings_action_t;

typedef struct {
    rect_t bounds;
    const char *label;
    settings_action_t action;
} settings_button_t;

typedef struct {
    settings_button_t buttons[3];
    int button_count;
    settings_action_t last_action;
} settings_screen_t;

void settings_screen_init(settings_screen_t *settings, int width, int height);
settings_action_t settings_screen_update(settings_screen_t *settings, input_state_t *input);
void settings_screen_draw(const settings_screen_t *settings);
void settings_screen_unload(void *screen_data);

// Input and action handlers for integration with input controller
void settings_input_handler(void *screen_data, input_state_t *input);
void settings_action_handler(void *screen_data, screen_manager_t *mgr, bool *running);
void settings_render_handler(void *screen_data);

#endif // settings_H
