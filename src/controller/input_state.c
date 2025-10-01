#include "controller/input_state.h"
#include "raylib.h"
#include "stdio.h"
#include <math.h>
#include <stdbool.h>

void input_state_init(input_state_t *state) {
  // Initialize all fields to zero/false
  state->mouse = (Vector2){0, 0};
  state->mouse_prev = (Vector2){0, 0};
  state->mouse_delta = (Vector2){0, 0};

  state->mouse_left_down = false;
  state->mouse_left_pressed = false;
  state->mouse_left_released = false;

  state->mouse_right_down = false;
  state->mouse_right_pressed = false;
  state->mouse_right_released = false;

  state->mouse_wheel_delta = 0.0f;

  state->key_escape = false;
  state->key_escape_pressed = false;
  state->key_shift = false;
  state->key_ctrl = false;
  state->key_r_pressed = false;
  state->key_m_pressed = false;

  state->key_up_pressed = false;
  state->key_down_pressed = false;
  state->key_left_pressed = false;
  state->key_right_pressed = false;
  state->key_enter_pressed = false;
  state->key_space_pressed = false;
  state->key_tab_pressed = false;

  state->key_f1_pressed = false;

  state->mouse_dragging = false;
  state->mouse_left_dragging = false;
  state->mouse_left_was_dragging = false;
  state->drag_bounds = (Clay_BoundingBox){0, 0, 0, 0};
  state->hovered_element_id = (Clay_ElementId){0};
}

void get_input_state(input_state_t *out) {
  static Vector2 last_mouse = {0, 0};
  const float drag_threshold =
    2.0f; // Lowered from 5.0f to 2.0f pixels for more sensitive detection

  // --- Mouse / Trackpad position ---
  Vector2 current_pos;
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
  // --- Drag detection ---

  // Check if mouse is inside the active drag bounds
  // If drag bounds are not set (all zeros), allow dragging anywhere
  bool has_drag_bounds =
    (out->drag_bounds.width > 0 && out->drag_bounds.height > 0);
  bool mouse_in_drag_bounds = true; // Default to true if no bounds set

  if (has_drag_bounds) {
    mouse_in_drag_bounds =
      out->mouse.x >= out->drag_bounds.x &&
      out->mouse.x <= out->drag_bounds.x + out->drag_bounds.width &&
      out->mouse.y >= out->drag_bounds.y &&
      out->mouse.y <= out->drag_bounds.y + out->drag_bounds.height;

    if (!mouse_in_drag_bounds) {
      // Kill drag immediately if we leave bounds
      out->mouse_dragging = false;
      out->mouse_right_down =
        false; // optional: prevents resuming until button re-press
    } else {
      out->mouse_dragging =
        out->mouse_right_down && (fabsf(out->mouse_delta.x) > drag_threshold ||
                                  fabsf(out->mouse_delta.y) > drag_threshold);
    }
  } else {
    out->mouse_dragging =
      out->mouse_right_down && (fabsf(out->mouse_delta.x) > drag_threshold ||
                                fabsf(out->mouse_delta.y) > drag_threshold);
  }

  // --- Left-click drag detection (for camera vs click distinction) ---
  static bool left_button_was_pressed = false;

  if (out->mouse_left_pressed) {

    left_button_was_pressed = true;
    out->mouse_left_dragging = false;
    out->mouse_left_was_dragging = false;
  }

  if (left_button_was_pressed && out->mouse_left_down) {
    // Only check drag bounds if they are set, otherwise allow dragging anywhere
    bool can_drag = !has_drag_bounds || mouse_in_drag_bounds;

    if (can_drag) {
      float delta_magnitude = sqrtf(out->mouse_delta.x * out->mouse_delta.x +
                                    out->mouse_delta.y * out->mouse_delta.y);
      if (delta_magnitude > drag_threshold && !out->mouse_left_dragging) {
        out->mouse_left_dragging = true;
      }
    }
  }

  if (out->mouse_left_released) {
    left_button_was_pressed = false;
    out->mouse_left_was_dragging = out->mouse_left_dragging;
    out->mouse_left_dragging = false;
  }
  // --- Modifier keys ---
  out->key_shift = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
  out->key_ctrl = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
  out->key_escape = IsKeyDown(KEY_ESCAPE);
  out->key_escape_pressed = IsKeyPressed(KEY_ESCAPE);
  out->key_r_pressed = IsKeyPressed(KEY_R);
  out->key_m_pressed = IsKeyPressed(KEY_M);

  // --- Navigation keys ---
  out->key_up_pressed = IsKeyPressed(KEY_UP);
  out->key_down_pressed = IsKeyPressed(KEY_DOWN);
  out->key_left_pressed = IsKeyPressed(KEY_LEFT);
  out->key_right_pressed = IsKeyPressed(KEY_RIGHT);
  out->key_enter_pressed = IsKeyPressed(KEY_ENTER);
  out->key_space_pressed = IsKeyPressed(KEY_SPACE);
  out->key_tab_pressed = IsKeyPressed(KEY_TAB);

  // --- Function keys ---
  out->key_f1_pressed = IsKeyPressed(KEY_F1);

  // --- Mouse wheel ---
  out->mouse_wheel_delta = GetMouseWheelMove();

  // Note: mouse_left_was_dragging is reset only on new press, not on release
  // This allows UI events to properly check drag state after mouse release

  // --- Debug prints ---
  // if (out->mouse_left_pressed)
  //   printf("Mouse left pressed\n");
  // if (out->mouse_left_released)
  //   printf("Mouse left released\n");
  // if (out->mouse_right_pressed)
  //   printf("Mouse right pressed\n");
  // if (out->mouse_right_released)
  //   printf("Mouse right released\n");
  // if (out->mouse_dragging)
  //   printf("Dragging... delta=(%.2f, %.2f)\n", out->mouse_delta.x,
  //          out->mouse_delta.y);
}
