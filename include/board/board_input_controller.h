#include "raylib.h"
#include "board.h"
#include "../grid/grid_system.h"

typedef struct {
    Vector2 pan;
    float zoom;
    grid_cell_t hovered_cell;
    bool has_hovered_cell;
} board_input_controller_t;

#define ZOOM_MIN 0.5f
#define ZOOM_MAX 3.0f

void
board_input_controller_init (board_input_controller_t *ctrl);
void
board_input_controller_update (board_input_controller_t *ctrl,
                               const board_t *board, int screen_width,
                               int screen_height);
