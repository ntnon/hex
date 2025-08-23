#ifndef PAUSE_H
#define PAUSE_H

#include "screen_manager.h"

//#include "raylib.h"
#include <stdbool.h>

typedef enum {
    _NONE,
    PAUSE_ACTION_RESUME,
    PAUSE_ACTION_MENU,
    PAUSE_ACTION_NONE,
    PAUSE_ACTION_QUIT
} pause_action_t;

typedef struct {
    rect_t bounds;
    const char *label;
    pause_action_t action;
} pause_button_t;

typedef struct {
    pause_button_t buttons[3];
    int button_count;
    pause_action_t last_action;
} pause_screen_t;

void pause_screen_init(pause_screen_t *pause, int width, int height);
pause_action_t pause_screen_update(pause_screen_t *pause, input_state_t *input);
void pause_screen_draw(const pause_screen_t *pause);
void pause_screen_unload(pause_screen_t *pause);

// Input and action handlers for integration with input controller
void pause_input_handler(void *screen_data);
void pause_action_handler(void *screen_data, screen_manager_t *mgr, bool *running);
void pause_render_handler(void *screen_data);

#endif // PAUSE_H
