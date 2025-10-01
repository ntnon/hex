#ifndef MENU_CONTROLLER_H
#define MENU_CONTROLLER_H

#include "controller/input_state.h"
#include "ui_types.h"

/* Menu states */
typedef enum {
    MENU_STATE_MAIN,
    MENU_STATE_LOADING,
    MENU_STATE_CONFIRMATION
} menu_state_t;

/* Menu item types */
typedef enum {
    MENU_ITEM_NEW_GAME,
    MENU_ITEM_CONTINUE,
    MENU_ITEM_SETTINGS,
    MENU_ITEM_QUIT,
    MENU_ITEM_BACK,
    MENU_ITEM_CONFIRM,
    MENU_ITEM_CANCEL
} menu_item_t;

/* Forward declaration */
typedef struct app_controller app_controller_t;

typedef struct menu_controller {
    menu_state_t current_state;
    
    /* Menu navigation */
    int selected_item_index;
    int menu_item_count;
    
    /* UI state */
    Clay_ElementId last_clicked_element;
    Clay_ElementId hovered_element;
    
    /* Parent app controller for callbacks */
    app_controller_t *app_controller;
    
    /* Menu-specific data */
    bool show_continue_button;  // Based on save game existence
    char confirmation_message[256];
    
} menu_controller_t;

/* Function declarations */

/* Initialization */
void menu_controller_init(menu_controller_t *menu_controller, app_controller_t *app_controller);
void menu_controller_cleanup(menu_controller_t *menu_controller);

/* Update and input */
void menu_controller_update(menu_controller_t *menu_controller, input_state_t *input);
void menu_controller_process_events(menu_controller_t *menu_controller);

/* Menu navigation */
void menu_controller_navigate_up(menu_controller_t *menu_controller);
void menu_controller_navigate_down(menu_controller_t *menu_controller);
void menu_controller_select_item(menu_controller_t *menu_controller);
void menu_controller_back(menu_controller_t *menu_controller);

/* Menu actions */
void menu_controller_handle_new_game(menu_controller_t *menu_controller);
void menu_controller_handle_continue(menu_controller_t *menu_controller);
void menu_controller_handle_settings(menu_controller_t *menu_controller);
void menu_controller_handle_quit(menu_controller_t *menu_controller);

/* State management */
void menu_controller_set_state(menu_controller_t *menu_controller, menu_state_t state);
menu_state_t menu_controller_get_state(menu_controller_t *menu_controller);

/* UI helpers */
void menu_controller_show_confirmation(menu_controller_t *menu_controller, const char *message);
int menu_controller_get_selected_item(menu_controller_t *menu_controller);

#endif // MENU_CONTROLLER_H