#ifndef MENU_H
#define MENU_H

#include "screen_manager.h"
#include "../controller/input_state.h"
#include "../types.h"

typedef enum {
    MENU_ACTION_NONE,
    MENU_ACTION_START,
    MENU_ACTION_OPTIONS,
    MENU_ACTION_QUIT
} menu_action_t;

typedef struct {
    menu_action_t last_action;
} menu_screen_t;

void menu_screen_init(menu_screen_t *menu, int width, int height);
void menu_screen_unload(void *screen_data);

// Handlers for generic screen callback system
void menu_input_handler(void *screen_data, input_state_t *input);
void menu_action_handler(void *screen_data, screen_manager_t *mgr, bool *running);
void menu_render_handler(void *screen_data);

#endif // MENU_H