#include "../../include/ui/settings_ui.h"
#include "../../include/screen/screen_manager.h"
#include "raylib.h"

static ui_button_config_t settings_buttons[]
    = { { .label = "Resume",
          .id = CLAY_ID ("settings_resume"),
          .action = SETTINGS_ACTION_RESUME },
        { .label = "Main Menu",
          .id = CLAY_ID ("settings_menu"),
          .action = SETTINGS_ACTION_MENU },
        { .label = "Quit",
          .id = CLAY_ID ("settings_quit"),
          .action = SETTINGS_ACTION_QUIT } };

static const int SETTINGS_BUTTON_COUNT
    = sizeof (settings_buttons) / sizeof (settings_buttons[0]);

void
settings_ui_init (ui_context_t *ctx)
{
  if (!ctx)
    return;

  ctx->settings.is_open = true;
  ctx->settings.active_element = 0;
}

void
settings_ui_draw (ui_context_t *ctx)
{
  if (!ctx || !ctx->context || !ctx->settings.is_open)
    return;

  // Semi-transparent overlay
  Clay_Container overlay
      = Clay_Container (.id = CLAY_ID ("settings_overlay"),
                        .sizing = { CLAY_SIZING_PERCENT (100),
                                    CLAY_SIZING_PERCENT (100) });

  Clay_PushContainer (ctx->context, &overlay);
  Clay_Rectangle (ctx->context,
                  Clay_RectangleConfig (.color = CLAY_COLOR (0, 0, 0, 100)));
  Clay_PopContainer (ctx->context);

  // Draw settings menu using shared button list
  ui_draw_button_list (ctx, "Settings", settings_buttons,
                       SETTINGS_BUTTON_COUNT, &ctx->settings.active_element);
}

settings_action_t
settings_ui_update (ui_context_t *ctx, input_state_t *input)
{
  if (!ctx || !ctx->settings.is_open)
    return SETTINGS_ACTION_NONE;

  // Check for escape key to resume game
  if (input && input->key_escape)
    {
      return SETTINGS_ACTION_RESUME;
    }

  // Check each button's action
  for (int i = 0; i < SETTINGS_BUTTON_COUNT; i++)
    {
      if (ctx->settings.active_element == settings_buttons[i].id)
        {
          ctx->settings.active_element = 0; // Reset active button
          return settings_buttons[i].action;
        }
    }

  return SETTINGS_ACTION_NONE;
}

void
settings_ui_cleanup (ui_context_t *ctx)
{
  if (!ctx)
    return;

  ctx->settings.is_open = false;
  ctx->settings.active_element = 0;
}
