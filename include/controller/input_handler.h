#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include "controller/input_state.h"
#include "game/game.h"
#include "third_party/clay.h"

/* Forward declarations */
struct game_controller;

typedef struct {
    game_t *game;
    Clay_BoundingBox game_bounds;
} input_handler_t;

/* Function declarations */
void input_handler_init(input_handler_t *handler, game_t *game);
void input_handler_update(input_handler_t *handler, input_state_t *input, Clay_BoundingBox game_bounds);
void input_handler_process_keyboard(input_handler_t *handler, input_state_t *input);
void input_handler_process_camera(input_handler_t *handler, input_state_t *input);

#endif // INPUT_HANDLER_H