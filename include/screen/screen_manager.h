#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H

#include "raylib.h"

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

void screen_manager_init(screen_manager_t *manager);
void screen_manager_switch(screen_manager_t *manager, screen_type_t new_screen);
screen_type_t screen_manager_get_current(screen_manager_t *manager);

#endif // SCREEN_MANAGER_H
