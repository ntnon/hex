#include "../../include/screen/settings_screen.h"
#include "../../include/screen/screen_manager.h"
#include "controller/input_state.h"
#include "render/renderer.h"

#define BUTTON_WIDTH 200
#define BUTTON_HEIGHT 50
#define BUTTON_SPACING 20

void
settings_screen_init (settings_screen_t *settings, int width, int height)
{
  int center_x = (width / 2) - BUTTON_WIDTH / 2;
  int start_y = (height / 2) - 50;

  settings->buttons[0].bounds
      = (rect_t){ center_x, start_y, BUTTON_WIDTH, BUTTON_HEIGHT };
  settings->buttons[0].label = "Resume";
  settings->buttons[0].action = SETTINGS_ACTION_RESUME;

  settings->buttons[1].bounds
      = (rect_t){ center_x, start_y + BUTTON_HEIGHT + BUTTON_SPACING,
                  BUTTON_WIDTH, BUTTON_HEIGHT };
  settings->buttons[1].label = "Main Menu";
  settings->buttons[1].action = SETTINGS_ACTION_MENU;

  settings->buttons[2].bounds
      = (rect_t){ center_x, start_y + 2 * (BUTTON_HEIGHT + BUTTON_SPACING),
                  BUTTON_WIDTH, BUTTON_HEIGHT };
  settings->buttons[2].label = "Quit";
  settings->buttons[2].action = SETTINGS_ACTION_QUIT;

  settings->button_count = 3;
  settings->last_action = SETTINGS_ACTION_NONE;
}

settings_action_t
settings_screen_update (settings_screen_t *settings, input_state_t *input)
{
  if (!settings || !input)
    return SETTINGS_ACTION_NONE;
  return SETTINGS_ACTION_NONE;
}

void
settings_screen_unload (void *settings)
{
  // No dynamic resources to free in this implementation
}

void
settings_input_handler (void *screen_data, input_state_t *input)
{
  settings_screen_t *settings = (settings_screen_t *)screen_data;
  if (!settings)
    return;
}

void
settings_action_handler (void *screen_data, screen_manager_t *mgr,
                         bool *running)
{
  settings_screen_t *settings = (settings_screen_t *)screen_data;
  screen_manager_t *screen_mgr = (screen_manager_t *)mgr;

  if (!settings || !screen_mgr || !running)
    return;

  switch (settings->last_action)
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

  // Reset action after handling
  settings->last_action = SETTINGS_ACTION_NONE;
}

void
settings_render_handler (void *screen_data)
{
  settings_screen_t *settings = (settings_screen_t *)screen_data;
  if (!settings)
    return;

  render_settings_screen (settings);
}
