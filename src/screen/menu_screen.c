#include "../../include/screen/menu_screen.h"

#define BUTTON_WIDTH 200
#define BUTTON_HEIGHT 50
#define BUTTON_SPACING 20

void
menu_screen_init (menu_screen_t *menu)
{
  int center_x = (GetScreenWidth () / 2) - BUTTON_WIDTH / 2;
  int start_y = (GetScreenHeight () / 2);

  menu->buttons[0].bounds
      = (Rectangle){ center_x, start_y, BUTTON_WIDTH, BUTTON_HEIGHT };
  menu->buttons[0].label = "Start";
  menu->buttons[0].action = MENU_ACTION_START;

  menu->buttons[1].bounds
      = (Rectangle){ center_x, start_y + BUTTON_HEIGHT + BUTTON_SPACING,
                     BUTTON_WIDTH, BUTTON_HEIGHT };
  menu->buttons[1].label = "Options";
  menu->buttons[1].action = MENU_ACTION_OPTIONS;

  menu->buttons[2].bounds
      = (Rectangle){ center_x, start_y + 2 * (BUTTON_HEIGHT + BUTTON_SPACING),
                     BUTTON_WIDTH, BUTTON_HEIGHT };
  menu->buttons[2].label = "Quit";
  menu->buttons[2].action = MENU_ACTION_QUIT;

  menu->button_count = 3;
  menu->last_action = MENU_ACTION_NONE;
}

menu_action_t
menu_screen_update (menu_screen_t *menu, Vector2 mouse, bool mouse_pressed)
{
  for (int i = 0; i < menu->button_count; i++)
    {
      if (CheckCollisionPointRec (mouse, menu->buttons[i].bounds))
        {
          if (mouse_pressed)
            {
              return menu->buttons[i].action;
            }
        }
    }
  return MENU_ACTION_NONE;
}

void
menu_screen_draw (const menu_screen_t *menu)
{
  // DrawText ("MY GAME TITLE", GetScreenWidth () / 3, 200, 48, DARKGRAY);

  Vector2 mouse = GetMousePosition ();

  for (int i = 0; i < menu->button_count; i++)
    {
      Color btn_color = CheckCollisionPointRec (mouse, menu->buttons[i].bounds)
                            ? LIGHTGRAY
                            : GRAY;
      DrawRectangleRec (menu->buttons[i].bounds, btn_color);
      DrawRectangleLinesEx (menu->buttons[i].bounds, 2, DARKGRAY);

      int text_width = MeasureText (menu->buttons[i].label, 24);
      int text_x
          = menu->buttons[i].bounds.x + (float)(BUTTON_WIDTH - text_width) / 2;
      int text_y = menu->buttons[i].bounds.y + (float)(BUTTON_HEIGHT - 24) / 2;
      DrawText (menu->buttons[i].label, text_x, text_y, 24, BLACK);
    }
}

void
menu_screen_unload (menu_screen_t *menu)
{
  // No dynamic resources to free in this implementation
}

void
menu_input_handler (void *screen_data)
{
  menu_screen_t *menu = (menu_screen_t *)screen_data;
  if (!menu)
    return;

  Vector2 mouse = GetMousePosition ();
  bool mouse_pressed = IsMouseButtonPressed (MOUSE_LEFT_BUTTON);

  menu->last_action = menu_screen_update (menu, mouse, mouse_pressed);

  // Handle keyboard shortcuts
  if (IsKeyPressed (KEY_ENTER))
    {
      menu->last_action = MENU_ACTION_START;
    }
  else if (IsKeyPressed (KEY_ESCAPE))
    {
      menu->last_action = MENU_ACTION_QUIT;
    }
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

  menu_screen_draw (menu);
}
