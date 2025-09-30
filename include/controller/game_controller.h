#ifndef GAME_CONTROLLER_H
#define GAME_CONTROLLER_H

#include "controller/input_state.h"
#include "controller/input_handler.h"
#include "controller/event_router.h"
#include "game/game.h"
#include "game/game_actions.h"
#include "ui_types.h"

/* Forward declaration to avoid circular dependency */
typedef struct input_area_info input_area_info_t;

typedef struct game_controller {
    game_t *game;
    int generation;
    input_state_t input;

    /* Composed components */
    input_handler_t input_handler;
    event_router_t event_router;
    game_actions_t game_actions;

    Clay_ElementData hovered_element_data;
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

/* Legacy accessors for backward compatibility */
Clay_ElementId controller_get_last_clicked_element(game_controller_t *controller);
Clay_ElementId controller_get_hovered_element(game_controller_t *controller);

/* Direct hovered element access for UI events */
Clay_ElementId controller_get_hovered_element_id(game_controller_t *controller);
void controller_set_hovered_element_id(game_controller_t *controller, Clay_ElementId elementId);

#endif // GAME_CONTROLLER_H
