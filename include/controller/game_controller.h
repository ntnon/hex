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

typedef enum {
    UI_AREA_NONE,
    UI_AREA_BOARD,
    UI_AREA_INVENTORY,
    UI_AREA_INFO,
} ui_area_t;

typedef struct {
    game_controller_state_e state;
    float pan_x;
    float pan_y;
    float zoom;
    grid_cell_t hovered_cell;
    bool has_hovered_cell;
    bool dragging;
    int drag_button;
    ui_area_t active_area;
}game_controller_t;

game_controller_t game_controller_create(void);

void update_game_controller(game_controller_t *controller);
