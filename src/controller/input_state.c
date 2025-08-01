#include "controller/input_state.h"
#include <stdbool.h>

bool
point_in_rect (vec2_t p, rect_t r)
{
  return p.x >= r.x && p.x <= r.x + r.width && p.y >= r.y
         && p.y <= r.y + r.height;
}

bool
rect_pressed (input_state_t *input, rect_t bounds)
{
  return point_in_rect (input->mouse, bounds) && input->mouse_left_pressed;
}

bool
rect_released (input_state_t *input, rect_t bounds)
{
  return point_in_rect (input->mouse, bounds) && input->mouse_left_released;
}
