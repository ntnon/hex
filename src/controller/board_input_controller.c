// #include "../../include/controller/board_input_controller.h"
// #include "raylib.h"
// #include <stdio.h>

// void
// board_input_controller_init (board_input_controller_t *ctrl)
// {
//   ctrl->pan = (vec2_t){ .x = 0, .y = 0 };
//   ctrl->zoom = 1.0f;
//   ctrl->has_hovered_cell = false;
//   ctrl->dragging = false;
//   ctrl->drag_button = MOUSE_BUTTON_LEFT;
// }

// // Add these to your controller struct:
// // bool dragging;
// // int drag_button;

// void
// board_input_controller_update (board_input_controller_t *ctrl,
//                                const board_t *board, int screen_width,
//                                int screen_height)
// {
//   float wheel = GetMouseWheelMove ();
//   if (wheel != 0.0f)
//     {
//       ctrl->zoom += wheel * 0.1f;
//       if (ctrl->zoom < ZOOM_MIN)
//         ctrl->zoom = ZOOM_MIN;
//       if (ctrl->zoom > ZOOM_MAX)
//         ctrl->zoom = ZOOM_MAX;
//     }

//   // Start drag on mouse button down
//   if (!ctrl->dragging
//       && (IsMouseButtonPressed (MOUSE_BUTTON_LEFT)
//           || IsMouseButtonPressed (MOUSE_BUTTON_MIDDLE)
//           || IsMouseButtonPressed (MOUSE_BUTTON_RIGHT)))
//     {
//       if (IsMouseButtonPressed (MOUSE_BUTTON_LEFT))
//         ctrl->drag_button = MOUSE_BUTTON_LEFT;
//       else if (IsMouseButtonPressed (MOUSE_BUTTON_MIDDLE))
//         ctrl->drag_button = MOUSE_BUTTON_MIDDLE;
//       else
//         ctrl->drag_button = MOUSE_BUTTON_RIGHT;
//       ctrl->dragging = true;
//     }

//   // End drag on mouse button up
//   if (ctrl->dragging && IsMouseButtonReleased (ctrl->drag_button))
//     {
//       ctrl->dragging = false;
//     }

//   // Pan while dragging
//   if (ctrl->dragging)
//     {
//       vec2_t delta = GetMouseDelta ();
//       ctrl->pan.x += delta.x;
//       ctrl->pan.y += delta.y;
//     }

//   // Hover detection (unchanged)
//   Vector2 mouse = GetMousePosition ();
//   point_t board_pos
//       = { .x = (mouse.x - screen_width / 2.0f - ctrl->pan.x) / ctrl->zoom,
//           .y = (mouse.y - screen_height / 2.0f - ctrl->pan.y) / ctrl->zoom
//           };
//   grid_cell_t cell = board->grid->vtable->from_pixel (board->grid,
//   board_pos); if (board->grid->vtable->is_valid_cell (board->grid, cell))
//     {
//       ctrl->hovered_cell = cell;
//       ctrl->has_hovered_cell = true;
//     }
//   else
//     {
//       ctrl->has_hovered_cell = false;
//     }
// }

// void
// board_input_controller_deactivate (board_input_controller_t *ctrl)
// {
//   ctrl->dragging = false;
//   ctrl->has_hovered_cell = false;
// }
