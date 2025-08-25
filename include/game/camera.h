#include "raylib.h"
#include "controller/input_state.h"

void camera_init(Camera2D *camera);
void update_camera(Camera2D *camera, const input_state_t *input);
void camera_set_offset(Camera2D *camera, int screen_width, int screen_height);
