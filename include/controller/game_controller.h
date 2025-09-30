#ifndef GAME_CONTROLLER_H
#define GAME_CONTROLLER_H

#include "controller/input_state.h"
#include "game/game.h"
#include "ui_types.h"

/* Forward declaration to avoid circular dependency */
typedef struct input_area_info input_area_info_t;

typedef struct {
    game_t *game;
    int generation;
    input_state_t input;

    Clay_ElementId last_clicked_ui_element_id;
    Clay_ElementId hovered_element_id;
    Clay_ElementData hovered_element_data;
    Clay_BoundingBox game_bounds;
    bool is_initialized;

} game_controller_t;

/* Function declarations */

void controller_init(game_controller_t *controller, game_t *game);
void controller_add_game_bounds(game_controller_t *controller, Clay_BoundingBox bounds);
void controller_update(game_controller_t *controller, input_state_t *input);
void controller_process_events(game_controller_t *controller);
void controller_hover(game_controller_t *controller, Clay_ElementId elementId);
void controller_set_hover(game_controller_t *controller, ui_event_t evt);
void controller_clear_hover(game_controller_t *controller, ui_event_t evt);



#endif // GAME_CONTROLLER_H
