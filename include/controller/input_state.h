#ifndef INPUT_STATE_H
#define INPUT_STATE_H

#include <stdbool.h>
#include "raylib.h"
#include "third_party/clay.h"

typedef struct {
    Vector2 mouse;
    Vector2 mouse_prev;
    Vector2 mouse_delta;

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

    // Optional drag helpers
    bool mouse_dragging;
    bool cancel_drag;
    Clay_BoundingBox drag_bounds;
    Clay_ElementId hovered_element_id;
} input_state_t;

void input_state_init(input_state_t* state);

void get_input_state(input_state_t* state);
void input_reset_drag(input_state_t* state);

bool rect_pressed (input_state_t *input, Rectangle bounds);
bool rect_released (input_state_t *input, Rectangle bounds);

#endif
