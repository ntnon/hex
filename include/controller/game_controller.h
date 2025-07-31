#include "raylib.h"
#include "../grid/grid_types.h"

typedef enum {
    GAME_CONTROLLER_STATE_IDLE,
    GAME_CONTROLLER_STATE_BOARD,
    GAME_CONTROLLER_STATE_INVENTORY,
    GAME_CONTROLLER_STATE_HOVER,
    GAME_CONTROLLER_STATE_DRAGGING,
    GAME_CONTROLLER_STATE_TILE_PLACING,
    GAME_CONTROLLER_STATE_EXITING,
    GAME_CONTROLLER_STATE_PAUSED,
    NUM_GAME_CONTROLLER_STATES
}game_controller_state_e;

typedef struct {
    game_controller_state_e state;
    Vector2 pan;
    float zoom;
    grid_cell_t hovered_cell;
    bool has_hovered_cell;
    bool dragging;
    MouseButton drag_button;
}game_controller_t;

game_controller_t game_controller_create(void);

void update_game_controller(game_controller_t *controller);
