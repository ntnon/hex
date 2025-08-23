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
<<<<<<< HEAD
=======
    rect_t bounds;
    const char *label;
    menu_action_t action;
} menu_button_t;

typedef struct {
    menu_button_t buttons[3];
    int button_count;

>>>>>>> parent of 53165fd (continued decoupling of raylib and game logic.)
    menu_action_t last_action;
} menu_screen_t;

void menu_screen_init(menu_screen_t *menu, int width, int height);
<<<<<<< HEAD
void menu_screen_unload(void *screen_data);
=======
menu_action_t menu_screen_update(menu_screen_t *menu, input_state_t *input);
void menu_screen_draw(const menu_screen_t *menu, input_state_t *input);
void menu_screen_unload(menu_screen_t *menu);
>>>>>>> parent of 53165fd (continued decoupling of raylib and game logic.)

// Input and action handlers for integration with input controller
void menu_input_handler(void *screen_data, input_state_t *input);
void menu_action_handler(void *screen_data, screen_manager_t *mgr, bool *running);
void menu_render_handler(void *screen_data);

<<<<<<< HEAD
#endif // MENU_H
=======
#endif // MENU_H
>>>>>>> parent of 53165fd (continued decoupling of raylib and game logic.)
