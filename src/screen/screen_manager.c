#include "../../include/screen/screen_manager.h"
#include <stdio.h>
void
screen_manager_init (screen_manager_t *manager)
{
  manager->current = SCREEN_MENU;

  for (int i = 0; i < SCREEN_MAX; ++i)
    {
      manager->screens[i].registered = false;
      manager->screens[i].screen_data = NULL;
      manager->screens[i].input_handler = NULL;
      manager->screens[i].action_handler = NULL;
      manager->screens[i].render_handler = NULL;
      manager->screens[i].unload_handler = NULL;
    }
}

void
screen_manager_switch (screen_manager_t *manager, screen_type_t new_screen)
{
  manager->current = new_screen;
}

screen_type_t
screen_manager_get_current (screen_manager_t *manager)
{
  return manager->current;
}

void
screen_manager_cleanup (screen_manager_t *manager)
{
  if (!manager)
    return;

  for (int i = 0; i < SCREEN_MAX; ++i)
    {
      screen_callbacks_t *screen = &manager->screens[i];

      if (screen->registered && screen->unload_handler && screen->screen_data)
        {
          screen->unload_handler (screen->screen_data);

          // Optional: clear references to prevent accidental reuse
          screen->screen_data = NULL;
          screen->unload_handler = NULL;
          screen->input_handler = NULL;
          screen->action_handler = NULL;
          screen->render_handler = NULL;
          screen->registered = false;
        }
    }
}

void
screen_manager_register (screen_manager_t *manager, screen_type_t screen_type,
                         screen_callbacks_t callbacks)
{
  if (!manager)
    return;
  if (screen_type < 0 || screen_type >= SCREEN_MAX)
    {
      fprintf (stderr, "Invalid screen type: %d\n", screen_type);
      return;
    }

  screen_callbacks_t *target = &manager->screens[screen_type];

  if (target->screen_data != NULL)
    {
      fprintf (stderr, "Warning: Screen %d already registered. Overwriting.\n",
               screen_type);
      // Optionally: return false to prevent overwrite
    }

  *target = callbacks;
}
