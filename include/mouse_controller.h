#ifndef MOUSE_INPUT_H
#define MOUSE_INPUT_H

#include "raylib.h"
#include "game_board_controller.h"

typedef struct mouse_controller {
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

    Camera2D* camera;
    game_board_controller* game_board_controller;
} mouse_controller;

void mouse_controller_init(mouse_controller* mouse, Camera2D* camera, game_board_controller* game_board_controller);
void mouse_controller_free(mouse_controller* mouse);
// Call this at the start of each frame, after updating the camera
void mouse_controller_update(mouse_controller* mouse, Camera2D* camera, game_board_controller* game_board_controller);
void mouse_controller_update_camera(mouse_controller* mouse);
static void update_hovered_hex(mouse_controller *mouse);

// Utility functions
bool mouse_in_rect(mouse_controller* mouse, Rectangle rect);
bool mouse_in_hex(mouse_controller* mouse, layout layout);


#endif // MOUSE_INPUT_H
