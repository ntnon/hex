#include "../../include/screen/pause_screen.h"
#include "../../include/screen/screen_manager.h"
#include "raylib.h"

#define BUTTON_WIDTH 200
#define BUTTON_HEIGHT 50
#define BUTTON_SPACING 20

void
pause_screen_init (pause_screen_t *pause)
{
  int center_x = (GetScreenWidth () / 2) - BUTTON_WIDTH / 2;
  int start_y = (GetScreenHeight () / 2) - 50;

  pause->buttons[0].bounds
      = (Rectangle){ center_x, start_y, BUTTON_WIDTH, BUTTON_HEIGHT };
  pause->buttons[0].label = "Resume";
  pause->buttons[0].action = PAUSE_ACTION_RESUME;

  pause->buttons[1].bounds
      = (Rectangle){ center_x, start_y + BUTTON_HEIGHT + BUTTON_SPACING,
                     BUTTON_WIDTH, BUTTON_HEIGHT };
  pause->buttons[1].label = "Main Menu";
  pause->buttons[1].action = PAUSE_ACTION_MENU;

  pause->buttons[2].bounds
      = (Rectangle){ center_x, start_y + 2 * (BUTTON_HEIGHT + BUTTON_SPACING),
                     BUTTON_WIDTH, BUTTON_HEIGHT };
  pause->buttons[2].label = "Quit";
  pause->buttons[2].action = PAUSE_ACTION_QUIT;

  pause->button_count = 3;
  pause->last_action = PAUSE_ACTION_NONE;
}

pause_action_t
pause_screen_update (pause_screen_t *pause, Vector2 mouse, bool mouse_pressed)
{
  for (int i = 0; i < pause->button_count; i++)
    {
      if (CheckCollisionPointRec (mouse, pause->buttons[i].bounds))
        {
          if (mouse_pressed)
            {
              return pause->buttons[i].action;
            }
        }
    }
  return PAUSE_ACTION_NONE;
}

void
pause_screen_draw (const pause_screen_t *pause)
{
  // Draw semi-transparent overlay
  DrawRectangle (0, 0, GetScreenWidth (), GetScreenHeight (),
                 (Color){ 0, 0, 0, 100 });

  // Draw pause title
  const char *title = "PAUSED";
  int title_width = MeasureText (title, 48);
  DrawText (title, (GetScreenWidth () - title_width) / 2,
            GetScreenHeight () / 2 - 150, 48, WHITE);

  Vector2 mouse = GetMousePosition ();

  for (int i = 0; i < pause->button_count; i++)
    {
      Color btn_color
          = CheckCollisionPointRec (mouse, pause->buttons[i].bounds)
                ? LIGHTGRAY
                : GRAY;
      DrawRectangleRec (pause->buttons[i].bounds, btn_color);
      DrawRectangleLinesEx (pause->buttons[i].bounds, 2, DARKGRAY);

      int text_width = MeasureText (pause->buttons[i].label, 24);
      int text_x
          = pause->buttons[i].bounds.x + (BUTTON_WIDTH - text_width) / 2;
      int text_y = pause->buttons[i].bounds.y + (BUTTON_HEIGHT - 24) / 2;
      DrawText (pause->buttons[i].label, text_x, text_y, 24, BLACK);
    }
}

void
pause_screen_unload (pause_screen_t *pause)
{
  // No dynamic resources to free in this implementation
}

void
pause_input_handler (void *screen_data)
{
  pause_screen_t *pause = (pause_screen_t *)screen_data;
  if (!pause)
    return;

  Vector2 mouse = GetMousePosition ();
  bool mouse_pressed = IsMouseButtonPressed (MOUSE_LEFT_BUTTON);

  pause->last_action = pause_screen_update (pause, mouse, mouse_pressed);

  // Handle keyboard shortcuts
  if (IsKeyPressed (KEY_ESCAPE) || IsKeyPressed (KEY_SPACE))
    {
      pause->last_action = PAUSE_ACTION_RESUME;
    }
  else if (IsKeyPressed (KEY_Q))
    {
      pause->last_action = PAUSE_ACTION_QUIT;
    }
  else if (IsKeyPressed (KEY_M))
    {
      pause->last_action = PAUSE_ACTION_MENU;
    }
}

void
pause_action_handler (void *screen_data, screen_manager_t *mgr, bool *running)
{
  pause_screen_t *pause = (pause_screen_t *)screen_data;
  screen_manager_t *screen_mgr = (screen_manager_t *)mgr;

  if (!pause || !screen_mgr || !running)
    return;

  switch (pause->last_action)
    {
    case PAUSE_ACTION_RESUME:
      screen_manager_switch (screen_mgr, SCREEN_GAME);
      break;
    case PAUSE_ACTION_MENU:
      screen_manager_switch (screen_mgr, SCREEN_MENU);
      break;
    case PAUSE_ACTION_QUIT:
      *running = false;
      break;
    case PAUSE_ACTION_NONE:
    default:
      break;
    }

  // Reset action after handling
  pause->last_action = PAUSE_ACTION_NONE;
}

void
pause_render_handler (void *screen_data)
{
  pause_screen_t *pause = (pause_screen_t *)screen_data;
  if (!pause)
    return;

  pause_screen_draw (pause);
}
