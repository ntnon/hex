#include "game/camera.h"

void
update_camera (Camera2D *camera, const input_state_t *input)
{
  // --- Zoom with mouse wheel ---
  camera->zoom += input->mouse_wheel_delta * 0.1f;
  if (camera->zoom < 0.1f)
    camera->zoom = 0.1f;

  // --- Pan with drag ---
  // On Mac trackpads, right-click often doesn't register, so use:
  //   - Left-drag for normal pan
  //   - Optionally, Ctrl + left-drag for alternate pan
  bool pan_drag = input->mouse_left_down; // left click/drag
  bool alt_pan
      = input->mouse_right_down || (input->key_ctrl && input->mouse_left_down);

  if (pan_drag || alt_pan)
    {
      camera->target.x -= input->mouse_delta.x / camera->zoom;
      camera->target.y -= input->mouse_delta.y / camera->zoom;
    }
}
