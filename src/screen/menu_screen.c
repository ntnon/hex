#include "../../include/screen/menu_screen.h"
#include "../../include/render/renderer.h"

void
menu_screen_init (menu_screen_t *menu, int width, int height)
{
  int center_x = (width / 2) - BUTTON_WIDTH / 2;
  int start_y = (height / 2);

  menu->buttons[0].bounds
      = (rect_t){ center_x, start_y, BUTTON_WIDTH, BUTTON_HEIGHT };
  menu->buttons[0].label = "Start";
  menu->buttons[0].action = MENU_ACTION_START;

  menu->buttons[1].bounds
      = (rect_t){ center_x, start_y + BUTTON_HEIGHT + BUTTON_SPACING,
                  BUTTON_WIDTH, BUTTON_HEIGHT };
  menu->buttons[1].label = "Options";
  menu->buttons[1].action = MENU_ACTION_OPTIONS;

  menu->buttons[2].bounds
      = (rect_t){ center_x, start_y + 2 * (BUTTON_HEIGHT + BUTTON_SPACING),
                  BUTTON_WIDTH, BUTTON_HEIGHT };
  menu->buttons[2].label = "Quit";
  menu->buttons[2].action = MENU_ACTION_QUIT;

  menu->button_count = 3;
  menu->last_action = MENU_ACTION_NONE;
}

menu_action_t
menu_screen_update (menu_screen_t *menu, input_state_t *input)
{
  for (int i = 0; i < menu->button_count; i++)
    {
      if (rect_pressed (input, menu->buttons[i].bounds))
        return menu->buttons[i].action;
    }
  return MENU_ACTION_NONE;
}

void
menu_screen_unload (void *screen_data)
{
  // No dynamic resources to free in this implementation
}

void
menu_input_handler (void *screen_data, input_state_t *input)
{
  menu_screen_t *menu = (menu_screen_t *)screen_data;
  if (!menu || !input)
    return;

  menu_action_t action = menu_screen_update (menu, input);
  menu->last_action = action;
}

void
menu_action_handler (void *screen_data, screen_manager_t *mgr, bool *running)
{
  menu_screen_t *menu = (menu_screen_t *)screen_data;
  screen_manager_t *screen_mgr = (screen_manager_t *)mgr;

  if (!menu || !screen_mgr || !running)
    return;

  switch (menu->last_action)
    {
    case MENU_ACTION_START:
      screen_manager_switch (screen_mgr, SCREEN_GAME);
      break;
    case MENU_ACTION_OPTIONS:
      // TODO: Implement options screen
      break;
    case MENU_ACTION_QUIT:
      *running = false;
      break;
    case MENU_ACTION_NONE:
    default:
      break;
    }

  // Reset action after handling
  menu->last_action = MENU_ACTION_NONE;
}

void
menu_render_handler (void *screen_data)
{
  menu_screen_t *menu = (menu_screen_t *)screen_data;
  if (!menu)
    return;

  render_menu_screen (menu);
}
