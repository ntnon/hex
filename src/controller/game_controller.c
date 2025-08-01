#include "../../include/controller/game_controller.h"

#define ZOOM_MIN 0.1f
#define ZOOM_MAX 3.0f

game_controller_t
game_controller_create (void)
{
  game_controller_t controller;
  controller.pan_x = 0.0;
  controller.pan_y = 0.0;
  controller.state = GAME_CONTROLLER_STATE_IDLE;
  controller.zoom = 1.0f;

  grid_cell_t hex_cell
      = { .type = GRID_TYPE_HEXAGON,
          .coord.hex = (hex_coord_t){ .q = 0, .r = 0, .s = 0 } };

  controller.hovered_cell = hex_cell;
  controller.has_hovered_cell = false;
  controller.dragging = false;
  controller.drag_button = 0;

  return controller;
}
