<<<<<<< HEAD
#include "controller/input_state.h" //used by all screens
#include "types.h"
#include "ui/ui_context.h"

=======
>>>>>>> parent of 53165fd (continued decoupling of raylib and game logic.)
#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H

#include "controller/input_state.h" //used by all screens
#include "types.h"

typedef enum {
    SCREEN_MENU,
    SCREEN_GAME,
    SCREEN_PAUSE,
    SCREEN_GAMEOVER,
    NUM_SCREENS
} screen_type_t;

typedef struct {
    screen_type_t current;
} screen_manager_t;

<<<<<<< HEAD
// Function declarations
void screen_manager_init (screen_manager_t *manager, ui_context_t *ui_ctx);

void screen_manager_switch(screen_manager_t *manager, screen_type_t new_screen);
screen_type_t screen_manager_get_current(screen_manager_t *manager);
void screen_manager_register(screen_manager_t *manager, screen_type_t type, screen_callbacks_t callbacks);
=======
void screen_manager_init(screen_manager_t *manager);
void screen_manager_switch(screen_manager_t *manager, screen_type_t new_screen);
screen_type_t screen_manager_get_current(screen_manager_t *manager);
>>>>>>> parent of 53165fd (continued decoupling of raylib and game logic.)

#endif // SCREEN_MANAGER_H
