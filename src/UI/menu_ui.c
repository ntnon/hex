#include "../../include/ui/menu_ui.h"
#include "../../include/screen/screen_manager.h"
#include "raylib.h"

static ui_button_config_t menu_buttons[] = {
  { .label = "Start",
    .id = CLAY_ID ("menu_start"),
    .action = MENU_ACTION_START },
  { .label = "Options",
    .id = CLAY_ID ("menu_options"),
    .action = MENU_ACTION_OPTIONS },
  { .label = "Quit", .id = CLAY_ID ("menu_quit"), .action = MENU_ACTION_QUIT }
};

static const int MENU_BUTTON_COUNT
    = sizeof (menu_buttons) / sizeof (menu_buttons[0]);

void
menu_ui_init (ui_context_t *ctx)
{
  if (!ctx)
    return;

  ctx->menu.is_open = true;
  ctx->menu.active_element = 0;
}

void
menu_ui_draw (ui_context_t *ctx)
{
  if (!ctx || !ctx->context || !ctx->menu.is_open)
    return;

  ui_draw_button_list (ctx, "HexHex", menu_buttons, MENU_BUTTON_COUNT,
                       &ctx->menu.active_element);
}

menu_action_t
menu_ui_update (ui_context_t *ctx, input_state_t *input)
{
  if (!ctx || !ctx->menu.is_open)
    return MENU_ACTION_NONE;

  // Check each button's action
  for (int i = 0; i < MENU_BUTTON_COUNT; i++)
    {
      if (ctx->menu.active_element == menu_buttons[i].id)
        {
          ctx->menu.active_element = 0; // Reset active button
          return menu_buttons[i].action;
        }
    }

  return MENU_ACTION_NONE;
}

void
menu_ui_cleanup (ui_context_t *ctx)
{
  if (!ctx)
    return;

  ctx->menu.is_open = false;
  ctx->menu.active_element = 0;
}
