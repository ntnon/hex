#include "../include/game/game.h"
#include "raylib.h"

typedef struct {
    game_t game;
} input_controller_t;

input_controller_t input_controller_create(game_t game);

void input_controller_update(input_controller_t* input_controller);

void input_controller_destroy(input_controller_t* input_controller);
