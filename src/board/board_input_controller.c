#include "../../include/board/board_input_controller.h"
#include "raylib.h"
#include <stdio.h>

#define ZOOM_MIN 0.1f
#define ZOOM_MAX 3.0f

void
board_input_controller_init (board_input_controller_t *ctrl)
{
  ctrl->pan = (Vector2){ 0 };
  ctrl->zoom = 1.0f;
  ctrl->has_hovered_cell = false;
}

void
board_input_controller_update (board_input_controller_t *ctrl,
                               const board_t *board, int screen_width,
                               int screen_height)
{
  // Zoom with mouse wheel

  float wheel = GetMouseWheelMove ();
  if (wheel != 0.0f)
    {
      ctrl->zoom += wheel * 0.1f;
      if (ctrl->zoom < ZOOM_MIN)
        ctrl->zoom = ZOOM_MIN;
      if (ctrl->zoom > ZOOM_MAX)
        ctrl->zoom = ZOOM_MAX;
    }

  // Pan with middle mouse drag
  if (IsMouseButtonDown (MOUSE_BUTTON_MIDDLE))
    {
      Vector2 delta = GetMouseDelta ();
      ctrl->pan.x += delta.x;
      ctrl->pan.y += delta.y;
    }

  // Hover detection
  Vector2 mouse = GetMousePosition ();
  point_t board_pos
      = { .x = (mouse.x - screen_width / 2.0f - ctrl->pan.x) / ctrl->zoom,
          .y = (mouse.y - screen_height / 2.0f - ctrl->pan.y) / ctrl->zoom };
  grid_cell_t cell = board->grid->vtable->from_pixel (board->grid, board_pos);
  if (board->grid->vtable->is_valid_cell (board->grid, cell))
    {
      ctrl->hovered_cell = cell;
      ctrl->has_hovered_cell = true;
    }
  else
    {
      ctrl->has_hovered_cell = false;
    }
}
