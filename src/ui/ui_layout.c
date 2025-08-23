#include "game/inventory.h"
#include "raylib.h"
// #include "render/clay_renderer_raylib.h"
#include "render/renderer.h"
#include "stdio.h"
#include "ui.h"

bool is_id_valid(const Clay_ElementId id) { return id.id == UI_ID_NONE.id; }

Clay_RenderCommandArray ui_build_layout(game_controller_t *controller) {
  Clay_SetPointerState(
    (Clay_Vector2){controller->input.mouse.x, controller->input.mouse.y},
    controller->input.mouse_left_down);

  Clay_SetLayoutDimensions(
    (Clay_Dimensions){.width = GetScreenWidth(), .height = GetScreenHeight()});
  Clay_BeginLayout();

  CLAY({.id = UI_ID_MAIN,
        .backgroundColor = M_LIGHTGRAY,
        .layout = {.layoutDirection = CLAY_LEFT_TO_RIGHT,
                   .sizing = (Clay_Sizing){.width = CLAY_SIZING_GROW(),
                                           .height = CLAY_SIZING_GROW()},
                   .childGap = 5,
                   .padding = CLAY_PADDING_ALL(8)}}) {

    // --- Game area ---
    CLAY({.id = UI_ID_GAME,
          .cornerRadius = 5,
          .backgroundColor = M_BROWN,
          .layout = {.sizing =
                       (Clay_Sizing){.height = CLAY_SIZING_GROW(),
                                     .width = CLAY_SIZING_PERCENT(0.8)}}}) {
      Clay_OnHover(handle_hover, (intptr_t)controller);
      BeginMode2D(controller->game_camera);
      BeginScissorMode(controller->game_bounds.x, controller->game_bounds.y,
                       controller->game_bounds.width,
                       controller->game_bounds.height);

      render_board(controller->game->board);
      EndScissorMode();
      EndMode2D();
    };

    // --- Inventory area ---
    CLAY({.id = UI_ID_INVENTORY,
          .cornerRadius = 5,
          .backgroundColor = M_BROWN,
          .layout = {.childGap = 10,
                     .padding = CLAY_PADDING_ALL(10),
                     .layoutDirection = CLAY_TOP_TO_BOTTOM,
                     .sizing =
                       (Clay_Sizing){.height = CLAY_SIZING_GROW(),
                                     .width = CLAY_SIZING_PERCENT(0.2)}}}) {
      Clay_OnHover(handle_hover, (intptr_t)controller);

      int inventory_size = inventory_get_size(controller->game->inventory);
      for (int i = 0; i < inventory_size; i++) {
        Clay_ElementId id =
          inventory_get_element_id(controller->game->inventory, i);
        if (is_id_valid(id)) {

          CLAY({.id = id.id, // optional unique ID
                .backgroundColor = M_BEIGE,
                .cornerRadius = 5,
                .layout = {.layoutDirection = CLAY_TOP_TO_BOTTOM,
                           .padding = CLAY_PADDING_ALL(7),
                           .sizing =
                             (Clay_Sizing){.height = CLAY_SIZING_GROW(),
                                           .width = CLAY_SIZING_GROW()}}}){};
        }
      }
    };
  };

  return Clay_EndLayout();
}
