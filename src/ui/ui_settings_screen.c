#include "../../include/third_party/clay.h"
#include "../../include/ui/clay_ui.h"
#include "raylib.h"

// External color definitions from clay_ui.c
extern const Clay_Color COLOR_OVERLAY;
extern const Clay_Color COLOR_PANEL;
extern const Clay_Color COLOR_TEXT_LIGHT;

void
clay_ui_render_settings (clay_ui_t *ui)
{
  // Semi-transparent overlay
  CLAY ({ .id = CLAY_ID ("SettingsOverlay"),
          .layout = { .sizing = { CLAY_SIZING_GROW (), CLAY_SIZING_GROW () } },
          .backgroundColor = COLOR_OVERLAY })
  {

    // Settings panel
    CLAY ({ .id = CLAY_ID ("SettingsPanel"),
            .layout
            = { .sizing = { CLAY_SIZING_FIXED (400), CLAY_SIZING_FIXED (300) },
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
                .childAlignment = { CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER },
                .childGap = 20,
                .padding = { 20, 20, 20, 20 } },
            .backgroundColor = COLOR_PANEL })
    {

      // Title
      CLAY_TEXT (CLAY_STRING ("Settings"),
                 CLAY_TEXT_CONFIG ({ .fontSize = 32,
                                     .textColor = COLOR_TEXT_LIGHT,
                                     .letterSpacing = 2 }));

      // Resume button
      CLAY ({ .id = CLAY_ID ("ResumeButton"),
              .layout = { .sizing = { CLAY_SIZING_FIXED (200),
                                      CLAY_SIZING_FIXED (50) },
                          .childAlignment
                          = { CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER } },
              .backgroundColor = (Clay_Color){ 100, 150, 100, 255 } })
      {
        CLAY_TEXT (CLAY_STRING ("Resume"),
                   CLAY_TEXT_CONFIG ({ .fontSize = 20,
                                       .textColor = COLOR_TEXT_LIGHT,
                                       .letterSpacing = 1 }));

        if (Clay_PointerOver (CLAY_ID ("ResumeButton"))
            && IsMouseButtonPressed (MOUSE_BUTTON_LEFT))
          {
            clay_ui_switch_to (ui, UI_SCREEN_GAME);
          }
      }

      // Main menu button
      CLAY ({ .id = CLAY_ID ("MainMenuButton"),
              .layout = { .sizing = { CLAY_SIZING_FIXED (200),
                                      CLAY_SIZING_FIXED (50) },
                          .childAlignment
                          = { CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER } },
              .backgroundColor = (Clay_Color){ 150, 100, 100, 255 } })
      {
        CLAY_TEXT (CLAY_STRING ("Main Menu"),
                   CLAY_TEXT_CONFIG ({ .fontSize = 20,
                                       .textColor = COLOR_TEXT_LIGHT,
                                       .letterSpacing = 1 }));

        if (Clay_PointerOver (CLAY_ID ("MainMenuButton"))
            && IsMouseButtonPressed (MOUSE_BUTTON_LEFT))
          {
            clay_ui_switch_to (ui, UI_SCREEN_MENU);
          }
      }
    }
  }
}
