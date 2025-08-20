#ifndef GAME_CONTROLLER_H
#define GAME_CONTROLLER_H

#include "input_state.h"
#include "game/game.h"

typedef struct {
    game_t *game;
    input_state_t input;
    Camera2D camera;
    Clay_ElementId hovered_element_id;
} game_controller_t;

void controller_init(game_controller_t *controller, game_t *game);
void controller_update(game_controller_t *controller, input_state_t *input);
void controller_handle_events(game_controller_t *controller);
void controller_hover(game_controller_t *controller, Clay_ElementId elementId);

#endif // GAME_CONTROLLER_H
