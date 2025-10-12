#ifndef GAME_CONTROLLER_H
#define GAME_CONTROLLER_H

#include "controller/input_state.h"
#include "controller/input_handler.h"
#include "game/game.h"
#include "ui_types.h"

/* Forward declaration to avoid circular dependency */
typedef struct input_area_info input_area_info_t;

typedef struct game_controller {
    game_t *game;
    int generation;

    /* Input handling */
    input_handler_t input_handler;

    /* Simple state flags instead of complex state enum */
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

} game_controller_t;

/* Function declarations */

/* Initialization */
void game_controller_init(game_controller_t *controller, game_t *game);

/* Main update - orchestrates input to subsystems */
void game_controller_update(game_controller_t *controller, const input_state_t *input);

/* Input orchestration - returns true if input was consumed */
bool game_controller_handle_inventory_input(game_controller_t *controller, const input_state_t *input);
bool game_controller_handle_placement_input(game_controller_t *controller, const input_state_t *input);
bool game_controller_handle_board_input(game_controller_t *controller, const input_state_t *input);
bool game_controller_handle_camera_input(game_controller_t *controller, const input_state_t *input);

/* State coordination between subsystems */
void game_controller_enter_placement_mode(game_controller_t *controller);
void game_controller_exit_placement_mode(game_controller_t *controller);
void game_controller_toggle_inventory(game_controller_t *controller);

/* UI element tracking */
Clay_ElementId game_controller_get_hovered_element_id(game_controller_t *controller);
void game_controller_set_hovered_element_id(game_controller_t *controller, Clay_ElementId elementId);

#endif // GAME_CONTROLLER_H