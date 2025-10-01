#ifndef SETTINGS_CONTROLLER_H
#define SETTINGS_CONTROLLER_H

#include "controller/input_state.h"
#include "ui_types.h"

/* Settings categories */
typedef enum {
    SETTINGS_CATEGORY_GRAPHICS,
    SETTINGS_CATEGORY_AUDIO,
    SETTINGS_CATEGORY_GAMEPLAY,
    SETTINGS_CATEGORY_CONTROLS
} settings_category_t;

/* Settings item types */
typedef enum {
    SETTING_TYPE_TOGGLE,
    SETTING_TYPE_SLIDER,
    SETTING_TYPE_DROPDOWN,
    SETTING_TYPE_KEY_BINDING,
    SETTING_TYPE_ACTION
} setting_type_t;

/* Forward declaration */
typedef struct app_controller app_controller_t;

typedef struct settings_item {
    char name[64];
    setting_type_t type;
    settings_category_t category;
    
    union {
        struct {
            bool *value;
            bool default_value;
        } toggle;
        
        struct {
            float *value;
            float min_value;
            float max_value;
            float default_value;
        } slider;
        
        struct {
            int *value;
            char options[8][32];
            int option_count;
            int default_value;
        } dropdown;
        
        struct {
            int *key_code;
            int default_key;
        } key_binding;
        
        struct {
            void (*action)(void);
        } action;
    } data;
} settings_item_t;

typedef struct settings_controller {
    settings_category_t current_category;
    
    /* Navigation */
    int selected_item_index;
    int selected_category_index;
    
    /* UI state */
    Clay_ElementId last_clicked_element;
    Clay_ElementId hovered_element;
    
    /* Parent app controller */
    app_controller_t *app_controller;
    
    /* Settings data */
    settings_item_t *settings_items;
    int settings_count;
    int settings_capacity;
    
    /* Settings values */
    bool fullscreen;
    bool vsync;
    float master_volume;
    float sfx_volume;
    float music_volume;
    bool show_grid;
    bool show_coordinates;
    int camera_sensitivity;
    
    /* Dirty flag for saving */
    bool settings_modified;
    
} settings_controller_t;

/* Function declarations */

/* Initialization */
void settings_controller_init(settings_controller_t *settings_controller, app_controller_t *app_controller);
void settings_controller_cleanup(settings_controller_t *settings_controller);

/* Update and input */
void settings_controller_update(settings_controller_t *settings_controller, input_state_t *input);
void settings_controller_process_events(settings_controller_t *settings_controller);

/* Navigation */
void settings_controller_navigate_up(settings_controller_t *settings_controller);
void settings_controller_navigate_down(settings_controller_t *settings_controller);
void settings_controller_navigate_left(settings_controller_t *settings_controller);
void settings_controller_navigate_right(settings_controller_t *settings_controller);
void settings_controller_select_item(settings_controller_t *settings_controller);
void settings_controller_back(settings_controller_t *settings_controller);

/* Category management */
void settings_controller_set_category(settings_controller_t *settings_controller, settings_category_t category);
settings_category_t settings_controller_get_category(settings_controller_t *settings_controller);

/* Settings actions */
void settings_controller_toggle_setting(settings_controller_t *settings_controller, int item_index);
void settings_controller_adjust_slider(settings_controller_t *settings_controller, int item_index, float delta);
void settings_controller_cycle_dropdown(settings_controller_t *settings_controller, int item_index, int direction);

/* Save/Load */
void settings_controller_save_settings(settings_controller_t *settings_controller);
void settings_controller_load_settings(settings_controller_t *settings_controller);
void settings_controller_reset_to_defaults(settings_controller_t *settings_controller);

/* UI helpers */
int settings_controller_get_selected_item(settings_controller_t *settings_controller);
bool settings_controller_has_unsaved_changes(settings_controller_t *settings_controller);

#endif // SETTINGS_CONTROLLER_H