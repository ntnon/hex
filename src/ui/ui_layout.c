#include "raylib.h"
#include "stdio.h"
#include "ui.h"

Clay_RenderCommandArray ui_build_layout(game_controller_t *controller) {
  Clay_SetPointerState(
    (Clay_Vector2){controller->input.mouse.x, controller->input.mouse.y},
    controller->input.mouse_right_pressed);
  Clay_SetLayoutDimensions(
    (Clay_Dimensions){.width = GetScreenWidth(), .height = GetScreenHeight()});
  Clay_BeginLayout();

  CLAY({.id = CLAY_ID("main"),
        .layout = {.layoutDirection = CLAY_LEFT_TO_RIGHT,
                   .sizing = (Clay_Sizing){.width = CLAY_SIZING_GROW(),
                                           .height = CLAY_SIZING_GROW()},
                   .childGap = 5,
                   .padding = CLAY_PADDING_ALL(8)},
        .backgroundColor = M_BEIGE}) {

    CLAY({
      .id = CLAY_ID("right"),
      .cornerRadius = 5,
      .backgroundColor = M_BROWN,
      .layout = {.sizing = (Clay_Sizing){.height = CLAY_SIZING_GROW(),
                                         .width = CLAY_SIZING_PERCENT(0.8)}},
    }) {
      Clay_OnHover(handle_hover, (intptr_t)controller);
    };

    CLAY({
      .id = CLAY_ID("left"),
      .cornerRadius = 5,
      .backgroundColor = M_LIGHTGRAY,
      .layout = {.sizing = (Clay_Sizing){.height = CLAY_SIZING_GROW(),
                                         .width = CLAY_SIZING_PERCENT(0.2)}},
    }) {
      Clay_OnHover(handle_hover, (intptr_t)controller);
    };
  };

  return Clay_EndLayout();
}
