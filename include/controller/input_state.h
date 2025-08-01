#ifndef INPUT_STATE_H
#define INPUT_STATE_H
#include <stdbool.h>
#include "../types.h"

typedef struct {
    vec2_t mouse;
    bool mouse_left_pressed;
    bool mouse_right_pressed;
    bool key_escape;
    // Add others as needed
} input_state_t;

void input_state_init(input_state_t* state);

#endif
