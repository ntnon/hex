#include "game/inventory.h"
#include "raylib.h"
// #include "third_party/clay_renderer_raylib.h"
#include "renderer.h"
#include "stdio.h"
#include "tile/tile.h"
#include "ui.h"

#define INVENTORY_PADDING 10
#define INVENTORY_GAP INVENTORY_PADDING

bool is_id_valid(const Clay_ElementId id) { return id.id != UI_ID_NONE.id; }

Clay_RenderCommandArray ui_build_layout(game_controller_t *controller) {
  Clay_SetPointerState(
    (Clay_Vector2){controller->input.mouse.x, controller->input.mouse.y},
    controller->input.mouse_left_down);

  Clay_SetLayoutDimensions(
    (Clay_Dimensions){.width = GetScreenWidth(), .height = GetScreenHeight()});
  Clay_BeginLayout();

  CLAY({.id = UI_ID_MAIN,
        .backgroundColor = M_BLANK,
        .layout = {
          .layoutDirection = CLAY_LEFT_TO_RIGHT,
          .sizing = (Clay_Sizing){.width = CLAY_SIZING_GROW(),
                                  .height = CLAY_SIZING_GROW()},
          //.childGap = 5,
          //.padding = CLAY_PADDING_ALL(8)
        }}) {

    // --- Game area ---
    CLAY({.id = UI_ID_GAME,
          .cornerRadius = 0,
          .backgroundColor = M_BLANK,
          .layout = {.sizing = (Clay_Sizing){.height = CLAY_SIZING_GROW(),
                                             .width = CLAY_SIZING_GROW()}}}) {
      Clay_OnHover(handle_hover, (intptr_t)controller);
      BeginMode2D(controller->game->board->camera);

      printf("DEBUG: UI calling render_board_optimized\n");
      render_board_optimized(controller->game->board);
      printf("DEBUG: UI render_board_optimized completed\n");
      render_board_previews(controller->game->board);
      EndMode2D();
      CLAY({.id = UI_BUTTON_ADD_INVENTORY_ITEM,
            .backgroundColor = M_ORANGE,
            .layout = {.sizing = (Clay_Sizing){.width = CLAY_SIZING_FIT(),
                                               .height = CLAY_SIZING_FIT()}}}) {
        CLAY_TEXT(CLAY_STRING("Add to inventory"), &TEXT_CONFIG_MEDIUM);
        Clay_OnHover(handle_hover, (intptr_t)controller);
      }
    };

    // --- Inventory area ---
    CLAY({.id = UI_ID_INVENTORY,
          .cornerRadius = 0,
          .backgroundColor = M_BLANK,

          .layout = {.childGap = INVENTORY_GAP,
                     .padding = CLAY_PADDING_ALL(INVENTORY_PADDING),
                     .layoutDirection = CLAY_TOP_TO_BOTTOM,

                     .sizing = (Clay_Sizing){.height = CLAY_SIZING_GROW(),
                                             .width = CLAY_SIZING_FIT()}}}) {
      Clay_OnHover(handle_hover, (intptr_t)controller);

      int inventory_size = inventory_get_size(controller->game->inventory);

      for (int i = 0; i < inventory_size; i++) {
        inventory_item_t item =
          inventory_get_item(controller->game->inventory, i);

        // Check if this item is selected
        bool is_selected = (controller->game->inventory->selected_index == i);
        Clay_Color bg_color = is_selected ? M_ORANGE : M_BEIGE;
        int total_height = GetScreenHeight();

        float available_height = total_height - 2 * INVENTORY_PADDING -
                                 INVENTORY_GAP * (inventory_size - 1);

        float item_height = available_height / inventory_size;
        CLAY({.id = item.id,
              .backgroundColor = bg_color,
              .aspectRatio = 1.0,
              .clip = true,
              .layout = {.layoutDirection = CLAY_TOP_TO_BOTTOM,

                         .sizing = (Clay_Sizing){
                           .height = CLAY_SIZING_FIXED(item_height),
                           .width = CLAY_SIZING_GROW()}}}) {
          Clay_OnHover(handle_inventory_item_click, (intptr_t)controller);
        };
      }
    }
  };

  return Clay_EndLayout();
}
