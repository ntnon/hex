#include "raylib.h"
#include "input_state.h"

void camera_init(Camera2D *camera, int screen_width, int screen_height);
void update_camera(Camera2D *camera, const input_state_t *input);
