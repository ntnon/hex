#ifndef BOARD_INPUT_CONTROLLER_H
#define BOARD_INPUT_CONTROLLER_H

#include "raylib.h"
#include "board.h"
#include "../grid/grid_system.h"

typedef struct {
    Vector2 pan;
    float zoom;
    grid_cell_t hovered_cell;
    bool has_hovered_cell;
    bool dragging;
    MouseButton drag_button;
} board_input_controller_t;

#define ZOOM_MIN 0.1f
#define ZOOM_MAX 3.0f

void board_input_controller_init(board_input_controller_t *ctrl);
void board_input_controller_update(board_input_controller_t *ctrl,
                                   const board_t *board, int screen_width,
                                   int screen_height);
void
board_input_controller_deactivate (board_input_controller_t *ctrl);

#endif // BOARD_INPUT_CONTROLLER_H
