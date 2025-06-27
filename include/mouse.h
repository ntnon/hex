#ifndef MOUSE_INPUT_H
#define MOUSE_INPUT_H

#include "raylib.h"
#include "hex_grid.h"

typedef struct mouse_input {
    Vector2 screen_pos;   // Mouse position in screen coordinates
    Vector2 world_pos;    // Mouse position in world coordinates (with camera)
    bool left_pressed;    // Left button pressed this frame
    bool left_released;   // Left button released this frame
    bool left_down;       // Left button is held down
    bool right_pressed;   // Right button pressed this frame
    bool right_released;  // Right button released this frame
    bool right_down;      // Right button is held down
    float wheel_move;     // Mouse wheel movement this frame

    hex hovered_hex;
    bool hovering_hex;
} mouse_input;

// Call this at the start of each frame, after updating the camera
void mouse_input_update(mouse_input* mouse, Camera2D* camera);

// Utility functions
bool mouse_in_rect(mouse_input* mouse, Rectangle rect);
bool mouse_in_hex(mouse_input* mouse, layout layout);

#endif // MOUSE_INPUT_H
