#include "../../include/ui/ui_context.h"
#include "../../include/game/board.h"
#include "../../include/game/inventory.h"
#include "../../include/render/renderer.h"
#include "raylib.h"
#include <stdlib.h>

void
ui_context_init (ui_context_t *ctx)
{
  if (!ctx)
    return;

  ctx->current_screen = SCREEN_MENU;
  ctx->active_element = 0;
  ctx->settings_overlay_open = false;
}

void
ui_context_cleanup (ui_context_t *ctx)
{
  if (!ctx)
    return;

  // No cleanup needed for this simple implementation
}

void
ui_switch_screen (ui_context_t *ctx, game_screen_t new_screen)
{
  if (!ctx)
    return;

  // Handle cleanup of current screen
  switch (ctx->current_screen)
    {
    case SCREEN_GAME:
      ctx->settings_overlay_open = false;
      break;
    default:
      break;
    }

  ctx->current_screen = new_screen;
  ctx->active_element = 0;
}

void
handle_menu_input (ui_context_t *ctx, input_state_t *input)
{
  if (!ctx || !input)
    return;

  // Menu input is handled by Clay UI elements
}

void
handle_game_input (ui_context_t *ctx, input_state_t *input)
{
  if (!ctx || !input)
    return;

  // Check for settings toggle
  if (input->key_escape)
    {
      ctx->settings_overlay_open = !ctx->settings_overlay_open;
    }
}

void
handle_settings_input (ui_context_t *ctx, input_state_t *input)
{
  if (!ctx || !input)
    return;

  // Check for escape to close settings
  if (input->key_escape)
    {
      if (ctx->current_screen == SCREEN_SETTINGS)
        {
          ui_switch_screen (ctx, SCREEN_MENU);
        }
      else
        {
          ctx->settings_overlay_open = false;
        }
    }
}

void
render_menu (ui_context_t *ctx)
{
  if (!ctx)
    return;

  CLAY (CLAY_ID ("MenuContainer"),
        CLAY_LAYOUT (
            { .sizing = { CLAY_SIZING_GROW (), CLAY_SIZING_GROW () },
              .layoutDirection = CLAY_TOP_TO_BOTTOM,
              .childAlignment = { CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER },
              .childGap = 20 }))
  {
    CLAY_TEXT (
        CLAY_ID ("Title"), CLAY_STRING ("HexHex"),
        CLAY_TEXT_CONFIG ({ .fontSize = 48, .textColor = { 0, 0, 0, 255 } }));

    if (CLAY_BUTTON (
            CLAY_ID ("StartButton"),
            CLAY_LAYOUT ({ .sizing = { CLAY_SIZING_FIXED (200),
                                       CLAY_SIZING_FIXED (50) } }),
            CLAY_RECTANGLE_CONFIG ({ .color = { 200, 200, 200, 255 } })))
      {
        CLAY_TEXT (CLAY_ID ("StartText"), CLAY_STRING ("Start"),
                   CLAY_TEXT_CONFIG (
                       { .fontSize = 24, .textColor = { 0, 0, 0, 255 } }));
        ui_switch_screen (ctx, SCREEN_GAME);
      }

    if (CLAY_BUTTON (
            CLAY_ID ("SettingsButton"),
            CLAY_LAYOUT ({ .sizing = { CLAY_SIZING_FIXED (200),
                                       CLAY_SIZING_FIXED (50) } }),
            CLAY_RECTANGLE_CONFIG ({ .color = { 200, 200, 200, 255 } })))
      {
        CLAY_TEXT (CLAY_ID ("SettingsText"), CLAY_STRING ("Settings"),
                   CLAY_TEXT_CONFIG (
                       { .fontSize = 24, .textColor = { 0, 0, 0, 255 } }));
        ui_switch_screen (ctx, SCREEN_SETTINGS);
      }

    if (CLAY_BUTTON (
            CLAY_ID ("QuitButton"),
            CLAY_LAYOUT ({ .sizing = { CLAY_SIZING_FIXED (200),
                                       CLAY_SIZING_FIXED (50) } }),
            CLAY_RECTANGLE_CONFIG ({ .color = { 200, 200, 200, 255 } })))
      {
        CLAY_TEXT (CLAY_ID ("QuitText"), CLAY_STRING ("Quit"),
                   CLAY_TEXT_CONFIG (
                       { .fontSize = 24, .textColor = { 0, 0, 0, 255 } }));
        // Set quit flag - this would need to be handled in main loop
      }
  }
}

void
render_game (ui_context_t *ctx)
{
  if (!ctx)
    return;

  // Render settings overlay if open
  if (ctx->settings_overlay_open)
    {
      render_settings (ctx);
    }
}

void
render_settings (ui_context_t *ctx)
{
  if (!ctx)
    return;

  // Semi-transparent overlay
  CLAY (
      CLAY_ID ("SettingsOverlay"),
      CLAY_LAYOUT ({ .sizing = { CLAY_SIZING_GROW (), CLAY_SIZING_GROW () } }),
      CLAY_RECTANGLE_CONFIG ({ .color = { 0, 0, 0, 128 } }))
  {
    CLAY (CLAY_ID ("SettingsPanel"),
          CLAY_LAYOUT (
              { .layoutDirection = CLAY_TOP_TO_BOTTOM,
                .sizing = { CLAY_SIZING_GROW (), CLAY_SIZING_GROW () },
                .childAlignment = { CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER },
                .childGap = 20 }))
    {
      CLAY_TEXT (CLAY_ID ("SettingsTitle"), CLAY_STRING ("Settings"),
                 CLAY_TEXT_CONFIG (
                     { .fontSize = 36, .textColor = { 255, 255, 255, 255 } }));

      if (CLAY_BUTTON (
              CLAY_ID ("ResumeButton"),
              CLAY_LAYOUT ({ .sizing = { CLAY_SIZING_FIXED (200),
                                         CLAY_SIZING_FIXED (50) } }),
              CLAY_RECTANGLE_CONFIG ({ .color = { 200, 200, 200, 255 } })))
        {
          CLAY_TEXT (CLAY_ID ("ResumeText"), CLAY_STRING ("Resume"),
                     CLAY_TEXT_CONFIG (
                         { .fontSize = 24, .textColor = { 0, 0, 0, 255 } }));
          if (ctx->current_screen == SCREEN_SETTINGS)
            {
              ui_switch_screen (ctx, SCREEN_GAME);
            }
          else
            {
              ctx->settings_overlay_open = false;
            }
        }

      if (CLAY_BUTTON (
              CLAY_ID ("MainMenuButton"),
              CLAY_LAYOUT ({ .sizing = { CLAY_SIZING_FIXED (200),
                                         CLAY_SIZING_FIXED (50) } }),
              CLAY_RECTANGLE_CONFIG ({ .color = { 200, 200, 200, 255 } })))
        {
          CLAY_TEXT (CLAY_ID ("MainMenuText"), CLAY_STRING ("Main Menu"),
                     CLAY_TEXT_CONFIG (
                         { .fontSize = 24, .textColor = { 0, 0, 0, 255 } }));
          ui_switch_screen (ctx, SCREEN_MENU);
        }

      if (CLAY_BUTTON (
              CLAY_ID ("QuitButton2"),
              CLAY_LAYOUT ({ .sizing = { CLAY_SIZING_FIXED (200),
                                         CLAY_SIZING_FIXED (50) } }),
              CLAY_RECTANGLE_CONFIG ({ .color = { 200, 200, 200, 255 } })))
        {
          CLAY_TEXT (CLAY_ID ("QuitText2"), CLAY_STRING ("Quit"),
                     CLAY_TEXT_CONFIG (
                         { .fontSize = 24, .textColor = { 0, 0, 0, 255 } }));
          // Set quit flag - this would need to be handled in main loop
        }
    }
  }
}
