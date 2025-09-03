#include "game/inventory.h"
#include "raylib.h"
// #include "third_party/clay_renderer_raylib.h"
#include "renderer.h"
#include "stdio.h"
#include "tile/tile.h"
#include "ui.h"
#include "ui_types.h"

#define INVENTORY_PADDING 10
#define INVENTORY_GAP INVENTORY_PADDING

bool is_id_valid(const Clay_ElementId id) { return id.id != UI_ID_NONE.id; }

static void ui_build_add_inventory_button(game_controller_t *controller);
static void ui_build_reward_area(game_controller_t *controller);
static void ui_build_reward(game_controller_t *controller);

static void ui_build_game_area(game_controller_t *controller) {
  CLAY({.id = UI_ID_GAME,
        .cornerRadius = 0,
        .backgroundColor = M_BLANK,

        .layout = {.sizing = (Clay_Sizing){.height = CLAY_SIZING_GROW(),
                                           .width = CLAY_SIZING_GROW()}}}) {
    Clay_OnHover(handle_hover, (intptr_t)controller);
    BeginMode2D(controller->game->board->camera);

    render_board_optimized(controller->game->board);

    render_board_previews(controller->game->board);
    EndMode2D();

    ui_build_add_inventory_button(controller);
  }
}

static void ui_build_add_inventory_button(game_controller_t *controller) {
  CLAY({.id = UI_BUTTON_ADD_INVENTORY_ITEM,
        .backgroundColor = M_ORANGE,
        .layout = {.sizing = (Clay_Sizing){.width = CLAY_SIZING_FIT(),
                                           .height = CLAY_SIZING_FIT()}}}) {
    CLAY_TEXT(CLAY_STRING("Add to inventory"), &TEXT_CONFIG_MEDIUM);
    Clay_OnHover(handle_hover, (intptr_t)controller);
  }
}

static void ui_build_inventory_item(game_controller_t *controller,
                                    inventory_item_t item, int index,
                                    int inventory_size, float item_height) {
  bool is_selected = (controller->game->inventory->selected_index == index);
  Clay_Color bg_color = is_selected ? M_ORANGE : M_BEIGE;

  CLAY({.id = item.id,
        .backgroundColor = bg_color,
        .aspectRatio = 1.0,
        .clip = true,
        .layout = {.layoutDirection = CLAY_TOP_TO_BOTTOM,
                   .sizing =
                     (Clay_Sizing){.height = CLAY_SIZING_FIXED(item_height),
                                   .width = CLAY_SIZING_GROW()}}}) {
    Clay_OnHover(handle_inventory_item_click, (intptr_t)controller);
  }
}

static void ui_build_inventory_area(game_controller_t *controller){
  CLAY({.id = UI_ID_INVENTORY,
        .cornerRadius = 0,
        .backgroundColor = M_BLANK,
        .layout = {.childGap = INVENTORY_GAP,
                   .padding = CLAY_PADDING_ALL(INVENTORY_PADDING),
                   .layoutDirection = CLAY_TOP_TO_BOTTOM,
                   .sizing = (Clay_Sizing){.height = CLAY_SIZING_GROW(),
                                           .width = CLAY_SIZING_FIT()}}}){
    Clay_OnHover(handle_hover, (intptr_t)controller);

int inventory_size = inventory_get_size(controller->game->inventory);
int total_height = GetScreenHeight();
float available_height =
  total_height - 2 * INVENTORY_PADDING - INVENTORY_GAP * (inventory_size - 1);
float item_height = available_height / inventory_size;

for (int i = 0; i < inventory_size; i++) {
  inventory_item_t item = inventory_get_item(controller->game->inventory, i);
  ui_build_inventory_item(controller, item, i, inventory_size, item_height);
}
}
}
;

static void ui_build_reward_area(game_controller_t *controller) {
  CLAY({.id = UI_ID_REWARDS,
        .backgroundColor = M_GRAY,

        .layout = {.layoutDirection = CLAY_LEFT_TO_RIGHT,
                   .childAlignment.y = CLAY_ALIGN_Y_CENTER,
                   .sizing = (Clay_Sizing){.width = CLAY_SIZING_GROW(),
                                           .height = CLAY_SIZING_GROW()},
                   .childGap = 40,
                   .padding = CLAY_PADDING_ALL(40)}}) {
    ui_build_reward(controller);
  }
}

static void ui_build_reward(game_controller_t *controller) {
  for (int i = 0; i < controller->game->reward_count; i++) {
    CLAY({.id = CLAY_IDI(UI_ID_REWARD_BASE_STRING, i),
          .backgroundColor = M_BEIGE,
          .aspectRatio = 1.0,
          .layout = {.layoutDirection = CLAY_LEFT_TO_RIGHT,
                     .sizing = (Clay_Sizing){.width = CLAY_SIZING_GROW(),
                                             .height = CLAY_SIZING_GROW()},
                     .childGap = 5,
                     .padding = CLAY_PADDING_ALL(8)}}) {
      // Reward content would go here
    }
  }
}

void game_screen(game_controller_t *controller) {
  BeginMode2D(controller->game->board->camera);

  render_hex_grid(controller->game->board->grid);
  render_board_optimized(controller->game->board);
  render_board_previews(controller->game->board);
  EndMode2D();

  // Test 3D hexagon rendering outside 2D mode
  for (int i = 0; i < inventory_get_size(controller->game->inventory); i++) {
    inventory_item_t item = inventory_get_item(controller->game->inventory, i);
    if (!is_id_valid(item.id))
      continue;
    Clay_BoundingBox boundingBox = Clay_GetElementData(item.id).boundingBox;
    if (boundingBox.width > 0 && boundingBox.height > 0 && item.board) {
      Rectangle bounds = {.x = boundingBox.x,
                          .y = boundingBox.y,
                          .width = boundingBox.width,
                          .height = boundingBox.height};
      render_board_in_bounds(item.board, bounds);
    }
  }
}

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
        }}) {
    if (controller->game->state == GAME_STATE_PLAYING) {
      ui_build_game_area(controller);
      ui_build_inventory_area(controller);
    }
    if (controller->game->state == GAME_STATE_REWARD) {
      ui_build_reward_area(controller);
    }
  }

  return Clay_EndLayout();
}
