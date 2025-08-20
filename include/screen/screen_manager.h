#include "game/input_state.h" //used by all screens
#include "types.h"

#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H

// Forward declare to break the dependency loop
typedef struct screen_manager_t screen_manager_t;

typedef enum {
    SCREEN_MENU,
    SCREEN_GAME_START,
    SCREEN_GAME,
    SCREEN_GAME_END,
    SCREEN_SETTINGS,
    SCREEN_MAX
} screen_type_t;

typedef struct {
    void *screen_data;
    void (*input_handler)(void *screen_data, input_state_t *input);
    void (*action_handler)(void *screen_data, screen_manager_t *mgr, bool *running);
    void (*render_handler)(void *screen_data);
    void (*unload_handler)(void *screen_data);
    bool registered;
} screen_callbacks_t;

struct screen_manager_t {
    screen_type_t current;
    screen_callbacks_t screens[SCREEN_MAX];
};

// Function declarations
void screen_manager_init(screen_manager_t *manager);
void screen_manager_switch(screen_manager_t *manager, screen_type_t new_screen);
screen_type_t screen_manager_get_current(screen_manager_t *manager);
void screen_manager_register(screen_manager_t *manager, screen_type_t type, screen_callbacks_t callbacks);
void screen_manager_cleanup(screen_manager_t *manager);

#endif // SCREEN_MANAGER_H
