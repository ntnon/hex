#include "../../include/third_party/clay.h"
#include "../../include/ui/clay_ui.h"
#include "raylib.h"

// External color definitions from clay_ui.c
extern const Clay_Color COLOR_BACKGROUND;
extern const Clay_Color COLOR_BUTTON_PRIMARY;
extern const Clay_Color COLOR_BUTTON_SECONDARY;
extern const Clay_Color COLOR_BUTTON_DANGER;
extern const Clay_Color COLOR_TEXT_DARK;
extern const Clay_Color COLOR_TEXT_LIGHT;

void
clay_ui_render_menu (clay_ui_t *ui)
{
  CLAY ({ .id = CLAY_ID ("MenuRoot"),
          .layout
          = { .sizing = { CLAY_SIZING_GROW (), CLAY_SIZING_GROW () },
              .layoutDirection = CLAY_TOP_TO_BOTTOM,
              .childAlignment = { CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER },
              .childGap = 30 },
          .backgroundColor = COLOR_BACKGROUND })
  {
    // Title
    CLAY_TEXT (CLAY_STRING ("HexHex"),
               CLAY_TEXT_CONFIG ({ .fontSize = 48,
                                   .textColor = COLOR_TEXT_DARK,
                                   .letterSpacing = 2 }));

    // Start button
    CLAY (
        { .id = CLAY_ID ("StartButton"),
          .layout
          = { .sizing = { CLAY_SIZING_FIXED (200), CLAY_SIZING_FIXED (60) },
              .childAlignment = { CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER } },
          .backgroundColor = COLOR_BUTTON_PRIMARY })
    {
      CLAY_TEXT (CLAY_STRING ("Start Game"),
                 CLAY_TEXT_CONFIG ({ .fontSize = 24,
                                     .textColor = COLOR_TEXT_LIGHT,
                                     .letterSpacing = 1 }));

      if (Clay_PointerOver (CLAY_ID ("StartButton"))
          && IsMouseButtonPressed (MOUSE_BUTTON_LEFT))
        {
          clay_ui_switch_to (ui, UI_SCREEN_GAME);
        }
    }

    // Settings button
    CLAY (
        { .id = CLAY_ID ("SettingsButton"),
          .layout
          = { .sizing = { CLAY_SIZING_FIXED (200), CLAY_SIZING_FIXED (60) },
              .childAlignment = { CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER } },
          .backgroundColor = COLOR_BUTTON_SECONDARY })
    {
      CLAY_TEXT (CLAY_STRING ("Settings"),
                 CLAY_TEXT_CONFIG ({ .fontSize = 24,
                                     .textColor = COLOR_TEXT_LIGHT,
                                     .letterSpacing = 1 }));

      if (Clay_PointerOver (CLAY_ID ("SettingsButton"))
          && IsMouseButtonPressed (MOUSE_BUTTON_LEFT))
        {
          clay_ui_switch_to (ui, UI_SCREEN_SETTINGS);
        }
    }

    // Quit button
    CLAY (
        { .id = CLAY_ID ("QuitButton"),
          .layout
          = { .sizing = { CLAY_SIZING_FIXED (200), CLAY_SIZING_FIXED (60) },
              .childAlignment = { CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER } },
          .backgroundColor = COLOR_BUTTON_DANGER })
    {
      CLAY_TEXT (CLAY_STRING ("Quit"),
                 CLAY_TEXT_CONFIG ({ .fontSize = 24,
                                     .textColor = COLOR_TEXT_LIGHT,
                                     .letterSpacing = 1 }));

      if (Clay_PointerOver (CLAY_ID ("QuitButton"))
          && IsMouseButtonPressed (MOUSE_BUTTON_LEFT))
        {
          ui->quit_requested = true;
        }
    }
  }
}
