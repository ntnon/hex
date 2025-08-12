#include "../../include/third_party/clay.h"
#include "../../include/ui/clay_ui.h"
#include "raylib.h"

// External color definitions from clay_ui.c
extern const Clay_Color COLOR_TEXT_LIGHT;

void
clay_ui_render_game (clay_ui_t *ui)
{
  // Game screen with minimal UI overlay
  CLAY (
      { .id = CLAY_ID ("GameRoot"),
        .layout = { .sizing = { CLAY_SIZING_GROW (), CLAY_SIZING_GROW () } } })
  {

    // Top UI bar
    CLAY ({ .id = CLAY_ID ("GameTopBar"),
            .layout
            = { .sizing = { CLAY_SIZING_GROW (), CLAY_SIZING_FIXED (60) },
                .layoutDirection = CLAY_LEFT_TO_RIGHT,
                .childAlignment = { CLAY_ALIGN_X_LEFT, CLAY_ALIGN_Y_CENTER },
                .padding = { 10, 10, 10, 10 } },
            .backgroundColor = (Clay_Color){ 50, 50, 50, 200 } })
    {

      CLAY_TEXT (CLAY_STRING ("HexHex - Game"),
                 CLAY_TEXT_CONFIG ({ .fontSize = 20,
                                     .textColor = COLOR_TEXT_LIGHT,
                                     .letterSpacing = 1 }));
    }

    // Game area (placeholder for actual game rendering)
    CLAY ({ .id = CLAY_ID ("GameArea"),
            .layout
            = { .sizing = { CLAY_SIZING_GROW (), CLAY_SIZING_GROW () } } })
    {
      // Game content will be rendered here by the game system
    }
  }
}
