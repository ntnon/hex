#include "adapter/input_adapter.h"
#include "raylib.h"

void
get_input_state (input_state_t *out)
{
  out->mouse.x = GetMouseX ();
  out->mouse.y = GetMouseY ();
  out->mouse_left_pressed = IsMouseButtonPressed (MOUSE_LEFT_BUTTON);
  out->mouse_right_pressed = IsMouseButtonPressed (MOUSE_RIGHT_BUTTON);
  out->key_escape = IsKeyPressed (KEY_ESCAPE);
}
