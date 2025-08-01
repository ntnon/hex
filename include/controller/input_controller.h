#ifndef INPUT_CONTROLLER_H
#define INPUT_CONTROLLER_H

#include "../screen/screen_manager.h"
#include <stdbool.h>

// Forward declarations
//typedef struct screen_manager_t screen_manager_t;

// Input handler function pointer type
typedef void (*input_handler_fn)(void *screen_data);

// Action handler function pointer type
typedef void (*action_handler_fn)(void *screen_data, screen_manager_t *mgr, bool *running);

// Render handler function pointer type
typedef void (*render_handler_fn)(void *screen_data);

// Input controller struct
typedef struct {
    input_handler_fn input_handlers[NUM_SCREENS];
    action_handler_fn action_handlers[NUM_SCREENS];
    render_handler_fn render_handlers[NUM_SCREENS];
    void *screen_data[NUM_SCREENS];
} input_controller_t;

// Input controller functions
void input_controller_init(input_controller_t *controller);
void input_controller_register_screen(input_controller_t *controller,
                                     screen_type_t screen_type,
                                     input_handler_fn input_handler,
                                     action_handler_fn action_handler,
                                     render_handler_fn render_handler,
                                     void *screen_data);
void input_controller_update(input_controller_t *controller,
                           screen_manager_t *mgr,
                           bool *running);
void input_controller_render(input_controller_t *controller,
                           screen_manager_t *mgr);
void input_controller_destroy(input_controller_t *controller);

#endif // INPUT_CONTROLLER_H
