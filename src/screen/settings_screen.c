#include "../../include/screen/settings_screen.h"
#include "../../include/screen/screen_manager.h"
#include "../../include/ui/settings_ui.h"

void
settings_screen_init (settings_screen_t *settings, int width, int height)
{
  if (!settings)
    return;

  settings->last_action = SETTINGS_ACTION_NONE;
}

void
settings_screen_unload (void *screen_data)
{
  settings_screen_t *settings = (settings_screen_t *)screen_data;
  if (!settings)
    return;
}

void
settings_input_handler (void *screen_data, input_state_t *input)
{
  settings_screen_t *settings = (settings_screen_t *)screen_data;
  if (!settings || !input)
    return;

  // UI input handling is now managed by Clay through settings_ui_update
}

void
settings_action_handler (void *screen_data, screen_manager_t *mgr,
                         bool *running)
{
  settings_screen_t *settings = (settings_screen_t *)screen_data;
  screen_manager_t *screen_mgr = (screen_manager_t *)mgr;
  ui_context_t *ui_ctx = screen_mgr->ui_ctx;

  if (!settings || !screen_mgr || !running || !ui_ctx)
    return;

  // Get action from UI system
  settings_action_t action = settings_ui_update (ui_ctx, NULL);

  switch (action)
    {
    case SETTINGS_ACTION_RESUME:
      screen_manager_switch (screen_mgr, SCREEN_GAME);
      break;
    case SETTINGS_ACTION_MENU:
      screen_manager_switch (screen_mgr, SCREEN_MENU);
      break;
    case SETTINGS_ACTION_QUIT:
      *running = false;
      break;
    case SETTINGS_ACTION_NONE:
    default:
      break;
    }
}

void
settings_render_handler (void *screen_data)
{
  settings_screen_t *settings = (settings_screen_t *)screen_data;
  if (!settings)
    return;

  // Settings rendering is now handled by Clay through settings_ui_draw
}
