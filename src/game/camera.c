#include "game/camera.h"
#include "stdio.h"

void camera_init(Camera2D *camera, int screen_width, int screen_height) {
  camera->zoom = 0.6f;
  camera->rotation = 0.0f;
  printf("Hello World!\n");
  printf("%d, %d\n", screen_width, screen_height);
  camera->target = (Vector2){0.0f, 0.0f}; // center on world origin
  camera->offset = (Vector2){(float)screen_width / 2,
                             (float)screen_height / 2}; // center of screen
}

void update_camera(Camera2D *camera, const input_state_t *input) {
  // --- Zoom with mouse wheel ---
  if (input->mouse_wheel_delta != 0.0f) {
    // Screen-space mouse position
    Vector2 mouse_screen = input->mouse;

    // Convert mouse to world before zoom
    Vector2 world_before = GetScreenToWorld2D(mouse_screen, *camera);

    // Apply zoom change
    float old_zoom = camera->zoom;
    camera->zoom += input->mouse_wheel_delta * 0.05f;
    if (camera->zoom < 0.6f)
      camera->zoom = 0.6f;

    // Convert mouse to world after zoom
    Vector2 world_after = GetScreenToWorld2D(mouse_screen, *camera);

    // Adjust camera target so mouse world position stays fixed
    camera->target.x += (world_before.x - world_after.x);
    camera->target.y += (world_before.y - world_after.y);
  }

  // --- Pan with drag ---
  bool pan_drag = input->mouse_left_down;
  bool alt_pan =
    input->mouse_right_down || (input->key_ctrl && input->mouse_left_down);

  if (pan_drag || alt_pan) {
    camera->target.x -= input->mouse_delta.x / camera->zoom;
    camera->target.y -= input->mouse_delta.y / camera->zoom;
  }
}
