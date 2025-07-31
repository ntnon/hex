#include "../../include/screen/screen_manager.h"

void
screen_manager_init (screen_manager_t *manager)
{
  manager->current = SCREEN_MENU;
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
