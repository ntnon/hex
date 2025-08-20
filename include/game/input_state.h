#ifndef INPUT_STATE_H
#define INPUT_STATE_H

#include <stdbool.h>
#include "../types.h"

typedef enum {
    UI_BOARD,
    UI_INVENTORY,
    UI_MENU,
} ui_item_t;

typedef struct {
    vec2_t mouse;
    vec2_t mouse_prev;
    vec2_t mouse_delta;

    bool mouse_left_down;
    bool mouse_left_pressed;
    bool mouse_left_released;

    bool mouse_right_down;
    bool mouse_right_pressed;
    bool mouse_right_released;

    float mouse_wheel_delta;

    bool key_escape;
    bool key_shift;
    bool key_ctrl;

    // UI context (optional)
    ui_item_t active_ui;

    // Optional drag helpers
    bool mouse_dragging;
} input_state_t;

void input_state_init(input_state_t* state);

void get_input_state(input_state_t* state);

bool rect_pressed (input_state_t *input, rect_t bounds);
bool rect_released (input_state_t *input, rect_t bounds);

#endif
