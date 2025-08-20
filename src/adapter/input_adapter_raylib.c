#include "adapter/input_adapter.h"
#include "adapter/raylib_bridge.h"
#include "raylib.h"
#include <math.h>

void
get_input_state_adapter (input_state_t *out)
{
  static vec2_t last_mouse = { 0 };

  // Get current mouse position
  out->mouse.x = GetMouseX ();
  out->mouse.y = GetMouseY ();

  // Previous position
  out->mouse_prev = last_mouse;

  // Delta
  out->mouse_delta.x = out->mouse.x - last_mouse.x;
  out->mouse_delta.y = out->mouse.y - last_mouse.y;

  // Update for next frame
  last_mouse = out->mouse;

  // Mouse button states
  out->mouse_left_pressed = IsMouseButtonPressed (MOUSE_LEFT_BUTTON);
  out->mouse_left_released = IsMouseButtonReleased (MOUSE_LEFT_BUTTON);
  out->mouse_right_pressed = IsMouseButtonPressed (MOUSE_RIGHT_BUTTON);
  out->mouse_right_released = IsMouseButtonReleased (MOUSE_RIGHT_BUTTON);
  out->mouse_left_down = IsMouseButtonDown (MOUSE_LEFT_BUTTON);
  out->mouse_right_down = IsMouseButtonDown (MOUSE_RIGHT_BUTTON);

  // Drag detection
  out->mouse_dragging = out->mouse_left_pressed
                        || (out->mouse_left_down
                            && (fabsf (out->mouse_delta.x) > 0.0f
                                || fabsf (out->mouse_delta.y) > 0.0f));

  // Modifier keys
  out->key_shift = IsKeyDown (KEY_LEFT_SHIFT) || IsKeyDown (KEY_RIGHT_SHIFT);
  out->key_ctrl
      = IsKeyDown (KEY_LEFT_CONTROL) || IsKeyDown (KEY_RIGHT_CONTROL);

  // Mouse wheel
  out->mouse_wheel_delta = GetMouseWheelMove ();

  // Other keys
  out->key_escape = IsKeyPressed (KEY_ESCAPE);
}
