#ifndef APP_CONTROLLER_H
#define APP_CONTROLLER_H

#include "controller/game_controller.h"
#include "controller/input_state.h"
#include "game/game.h"

/* Application-level states */
typedef enum {
    APP_STATE_MAIN_MENU,
    APP_STATE_SETTINGS,
    APP_STATE_GAME,
    APP_STATE_PAUSED,
    APP_STATE_QUIT
} app_state_t;

/* Forward declarations for menu/settings controllers */
typedef struct menu_controller menu_controller_t;
typedef struct settings_controller settings_controller_t;

typedef struct app_controller {
    app_state_t current_state;
    app_state_t previous_state;

    /* Game components - only active during APP_STATE_GAME */
    game_t *game;
    game_controller_t game_controller;

    /* UI controllers for different states */
    menu_controller_t *menu_controller;
    settings_controller_t *settings_controller;

    /* Application-level data */
    input_state_t input;
    bool is_initialized;
    bool should_quit;

    /* Menu navigation state */
    int selected_menu_item;
    int menu_item_count;

} app_controller_t;

/* Function declarations */

/* Initialization and cleanup */
void app_controller_init(app_controller_t *app_controller);
void app_controller_cleanup(app_controller_t *app_controller);

/* Main update loop */
void app_controller_update(app_controller_t *app_controller, input_state_t *input);
void app_controller_process_events(app_controller_t *app_controller);

/* State management */
void app_controller_set_state(app_controller_t *app_controller, app_state_t new_state);
app_state_t app_controller_get_state(app_controller_t *app_controller);
bool app_controller_should_quit(app_controller_t *app_controller);

/* State-specific actions */
void app_controller_start_game(app_controller_t *app_controller);
void app_controller_pause_game(app_controller_t *app_controller);
void app_controller_resume_game(app_controller_t *app_controller);
void app_controller_quit_to_menu(app_controller_t *app_controller);
void app_controller_open_settings(app_controller_t *app_controller);
void app_controller_quit_application(app_controller_t *app_controller);

/* Input routing based on current state */
void app_controller_handle_global_input(app_controller_t *app_controller, input_state_t *input);

#endif // APP_CONTROLLER_H
