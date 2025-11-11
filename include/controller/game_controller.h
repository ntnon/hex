#ifndef GAME_CONTROLLER_H
#define GAME_CONTROLLER_H

#include "controller/input_state.h"
#include "controller/input_handler.h"
#include "game/game.h"
#include "ui_types.h"

/* Game state enum - now managed by controller */
typedef enum {
    GAME_STATE_VIEW,
    GAME_STATE_PLACE,
    GAME_STATE_INSPECT,
    GAME_STATE_INVENTORY,
    GAME_STATE_REWARD,
    GAME_STATE_GAME_OVER,
    GAME_STATE_COUNT
} game_state_e;

/* Forward declaration to avoid circular dependency */
typedef struct input_area_info input_area_info_t;

typedef struct game_controller {
    game_t *game;
    int generation;

    /* Input handling */
    input_handler_t input_handler;

    /* State management */
    game_state_e state;

    /* Simple state flags for fine-grained control */
    bool inventory_open;
    bool placing_tile;
    bool camera_locked;

    /* Currently selected/held data for coordination */
    int selected_inventory_index;
    board_t *held_piece;  /* What we're currently placing */

    /* UI interaction data */
    Clay_ElementData hovered_element_data;
    Clay_ElementId last_clicked_element_id;
    Clay_ElementId hovered_element_id;

    bool is_initialized;

    /* Derived UI state - computed from input for UI consumption */
    tile_t *hovered_tile;         /* Currently hovered tile (moved from game_t) */
    grid_cell_t hovered_cell;     /* Currently hovered cell (moved from game_t) */
    bool should_show_tile_info;   /* Whether UI should show tooltip (moved from game_t) */

} game_controller_t;

/* Function declarations */

/* Initialization */
void game_controller_init(game_controller_t *controller, game_t *game);

/* Main update - orchestrates input to subsystems */
void game_controller_update(game_controller_t *controller, const input_state_t *input);

/* Derived state updates - called internally by game_controller_update */
void game_controller_update_hover_state(game_controller_t *controller, const input_state_t *input);

/* Input orchestration - returns true if input was consumed */
bool game_controller_handle_inventory_input(game_controller_t *controller, const input_state_t *input);
bool game_controller_handle_placement_input(game_controller_t *controller, const input_state_t *input);
bool game_controller_handle_board_input(game_controller_t *controller, const input_state_t *input);
bool game_controller_handle_camera_input(game_controller_t *controller, const input_state_t *input);

/* State coordination between subsystems */
void game_controller_enter_placement_mode(game_controller_t *controller);
void game_controller_exit_placement_mode(game_controller_t *controller);
void game_controller_toggle_inventory(game_controller_t *controller);

/* State management */
void game_controller_set_state(game_controller_t *controller, game_state_e new_state);
game_state_e game_controller_get_state(game_controller_t *controller);
void game_controller_cycle_state(game_controller_t *controller);
const char *game_controller_state_to_string(game_state_e state);

/* UI element tracking */
Clay_ElementId game_controller_get_hovered_element_id(game_controller_t *controller);
void game_controller_set_hovered_element_id(game_controller_t *controller, Clay_ElementId elementId);

#endif // GAME_CONTROLLER_H
