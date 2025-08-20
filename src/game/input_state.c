#include "game/input_state.h"
#include <stdbool.h>

#include "raylib.h"
#include "stdio.h"
#include <math.h>
#include <stdbool.h>

void get_input_state(input_state_t *out) {
  static vec2_t last_mouse = {0, 0};
  const float drag_threshold = 0.1f;

  // --- Mouse / Trackpad position ---
  vec2_t current_pos;
  current_pos.x = GetMouseX();
  current_pos.y = GetMouseY();

  // --- Update mouse fields ---
  out->mouse_prev = last_mouse;
  out->mouse = current_pos;
  out->mouse_delta.x = out->mouse.x - out->mouse_prev.x;
  out->mouse_delta.y = out->mouse.y - out->mouse_prev.y;
  last_mouse = out->mouse;

  // --- Mouse buttons ---
  out->mouse_left_pressed = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
  out->mouse_left_released = IsMouseButtonReleased(MOUSE_LEFT_BUTTON);
  out->mouse_left_down = IsMouseButtonDown(MOUSE_LEFT_BUTTON);

  out->mouse_right_pressed = IsMouseButtonPressed(MOUSE_RIGHT_BUTTON);
  out->mouse_right_released = IsMouseButtonReleased(MOUSE_RIGHT_BUTTON);
  out->mouse_right_down = IsMouseButtonDown(MOUSE_RIGHT_BUTTON);

  // --- Drag detection ---
  // Drag is left-button down + movement above threshold
  out->mouse_dragging =
   out->mouse_left_down && (fabsf(out->mouse_delta.x) > drag_threshold ||
                            fabsf(out->mouse_delta.y) > drag_threshold);

  // --- Modifier keys ---
  out->key_shift = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
  out->key_ctrl = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
  out->key_escape = IsKeyPressed(KEY_ESCAPE);

  // --- Mouse wheel ---
  out->mouse_wheel_delta = GetMouseWheelMove();

  // --- Debug prints ---
  if (out->mouse_left_pressed)
    printf("Mouse left pressed\n");
  if (out->mouse_left_released)
    printf("Mouse left released\n");
  if (out->mouse_right_pressed)
    printf("Mouse right pressed\n");
  if (out->mouse_right_released)
    printf("Mouse right released\n");
  if (out->mouse_dragging)
    printf("Dragging... delta=(%.2f, %.2f)\n", out->mouse_delta.x,
           out->mouse_delta.y);
}

bool point_in_rect(vec2_t p, rect_t r) {
  return p.x >= r.x && p.x <= r.x + r.width && p.y >= r.y &&
         p.y <= r.y + r.height;
}

bool rect_pressed(input_state_t *input, rect_t bounds) {
  return point_in_rect(input->mouse, bounds) && input->mouse_left_pressed;
}

bool rect_released(input_state_t *input, rect_t bounds) {
  return point_in_rect(input->mouse, bounds) && input->mouse_left_released;
}
