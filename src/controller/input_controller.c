#include "../../include/controller/input_controller.h"
#include "../../include/screen/screen_manager.h"
#include "stdio.h"
#include <stdbool.h>
#include <stddef.h>

void
input_controller_init (input_controller_t *controller)
{
  if (!controller)
    return;

  // Initialize all handlers to NULL
  for (int i = 0; i < NUM_SCREENS; i++)
    {
      controller->input_handlers[i] = NULL;
      controller->action_handlers[i] = NULL;
      controller->render_handlers[i] = NULL;
      controller->screen_data[i] = NULL;
    }
}

void
input_controller_register_screen (input_controller_t *controller,
                                  screen_type_t screen_type,
                                  input_handler_fn input_handler,
                                  action_handler_fn action_handler,
                                  render_handler_fn render_handler,
                                  void *screen_data)
{
  if (!controller || screen_type >= NUM_SCREENS)
    return;

  controller->input_handlers[screen_type] = input_handler;
  controller->action_handlers[screen_type] = action_handler;
  controller->render_handlers[screen_type] = render_handler;
  controller->screen_data[screen_type] = screen_data;
}

void
input_controller_update (input_controller_t *controller, screen_manager_t *mgr,
                         bool *running)
{
  if (!controller || !mgr || !running)
    return;

  screen_type_t current_screen = screen_manager_get_current (mgr);

  // Handle input for current screen
  if (controller->input_handlers[current_screen])
    {
      controller->input_handlers[current_screen](
          controller->screen_data[current_screen]);
    }

  // Handle actions for current screen
  if (controller->action_handlers[current_screen])
    {
      controller->action_handlers[current_screen](
          controller->screen_data[current_screen], mgr, running);
    }
}

void
input_controller_render (input_controller_t *controller, screen_manager_t *mgr)
{

  if (!controller || !mgr)
    return;

  screen_type_t current_screen = screen_manager_get_current (mgr);

  // Render current screen
  if (controller->render_handlers[current_screen])
    {
      controller->render_handlers[current_screen](
          controller->screen_data[current_screen]);
    }
}

void
input_controller_destroy (input_controller_t *controller)
{
  if (!controller)
    return;

  // Reset all handlers and data pointers
  for (int i = 0; i < NUM_SCREENS; i++)
    {
      controller->input_handlers[i] = NULL;
      controller->action_handlers[i] = NULL;
      controller->render_handlers[i] = NULL;
      controller->screen_data[i] = NULL;
    }
}
