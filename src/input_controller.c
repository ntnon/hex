#include "game/screen_manager.h"
#include "raylib.h"

// Forward declarations
typedef struct menu_screen_t menu_screen_t;
typedef struct game_screen_t game_screen_t;

// Input handler function pointer type
typedef void (*input_handler_fn) (void *screen_data);

// Action handler function pointer type
typedef void (*action_handler_fn) (void *screen_data, screen_manager_t *mgr,
                                   bool *running);

// Input controller struct
typedef struct
{
  input_handler_fn input_handlers[NUM_SCREENS];
  void *screen_data[NUM_SCREENS];
} input_controller_t;

// Action controller struct
typedef struct
{
  action_handler_fn action_handlers[NUM_SCREENS];
  void *screen_data[NUM_SCREENS];
} action_controller_t;

// Main loop
void
main_loop (screen_manager_t *mgr, input_controller_t *input_ctrl,
           action_controller_t *action_ctrl, bool *running)
{
  while (*running && !WindowShouldClose ())
    {
      screen_type_t current = screen_manager_get_current (mgr);

      // Input handling
      input_ctrl->input_handlers[current](input_ctrl->screen_data[current]);

      // Action handling
      action_ctrl->action_handlers[current](action_ctrl->screen_data[current],
                                            mgr, running);

      BeginDrawing ();
      ClearBackground (RAYWHITE);

      // Render current screen (pseudo-code)
      // renderers[current](screen_data[current]);

      EndDrawing ();
    }
}

// Example: menu input handler
void
menu_input_handler (void *screen_data)
{ /* ... */
}

// Example: menu action handler
void
menu_action_handler (void *screen_data, screen_manager_t *mgr, bool *running)
{ /* ... */
}
