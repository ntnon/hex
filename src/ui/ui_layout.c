#include "ui.h"

Clay_RenderCommandArray ui_build_layout(game_t *game) {
  Clay_BeginLayout();

  CLAY({.id = CLAY_ID("main"),
        .layout = {.layoutDirection = CLAY_LEFT_TO_RIGHT,
                   .sizing = (Clay_Sizing){.width = CLAY_SIZING_GROW(),
                                           .height = CLAY_SIZING_GROW()},
                   .childGap = 5,
                   .padding = CLAY_PADDING_ALL(8)},
        .backgroundColor = M_BEIGE}) {

    CLAY(
     {.id = CLAY_ID("right"),
      .layout = {.sizing = (Clay_Sizing){.height = CLAY_SIZING_GROW(),
                                         .width = CLAY_SIZING_PERCENT(0.8)}},
      .cornerRadius = 5,
      .backgroundColor = M_BROWN}){
     // Optional: insert game board rendering hook here later
    };

    CLAY(
     {.id = CLAY_ID("left"),
      .layout = {.sizing = (Clay_Sizing){.height = CLAY_SIZING_GROW(),
                                         .width = CLAY_SIZING_PERCENT(0.2)}},
      .cornerRadius = 5,
      .backgroundColor = M_LIGHTGRAY});
  };

  return Clay_EndLayout();
}
