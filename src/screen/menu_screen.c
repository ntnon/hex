#include "../../include/screen/menu_screen.h"
#include "../../include/screen/screen_manager.h"
#include "../../include/ui/menu_ui.h"

void
menu_screen_init (menu_screen_t *menu, int width, int height)
{
  if (!menu)
    return;

  menu->last_action = MENU_ACTION_NONE;
}

void
menu_screen_unload (menu_screen_t *menu)
{
  menu_screen_t *menu = (menu_screen_t *)screen_data;
  if (!menu)
    return;
}
void
menu_input_handler (void *screen_data, input_state_t *input)
{
  menu_screen_t *menu = (menu_screen_t *)screen_data;
  if (!menu || !input)
    return;

  // UI input handling is now managed by Clay through menu_ui_update
}

void
menu_action_handler (void *screen_data, screen_manager_t *mgr, bool *running)
{
  menu_screen_t *menu = (menu_screen_t *)screen_data;
  screen_manager_t *screen_mgr = (screen_manager_t *)mgr;
  ui_context_t *ui_ctx = screen_mgr->ui_ctx;

  if (!menu || !screen_mgr || !running || !ui_ctx)
    return;

  // Get action from UI system
  menu_action_t action = menu_ui_update (ui_ctx, NULL);

  switch (action)
    {
    case MENU_ACTION_START:
      screen_manager_switch (screen_mgr, SCREEN_GAME);
      break;
    case MENU_ACTION_OPTIONS:
      screen_manager_switch (screen_mgr, SCREEN_SETTINGS);
      break;
    case MENU_ACTION_QUIT:
      *running = false;
      break;
    case MENU_ACTION_NONE:
    default:
      break;
    }
}

void
menu_render_handler (void *screen_data)
{
  menu_screen_t *menu = (menu_screen_t *)screen_data;
  if (!menu)
    return;

  // Menu rendering is now handled by Clay through menu_ui_draw
}
