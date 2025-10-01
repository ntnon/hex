#include "game/inventory.h"
#include "raylib.h"
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
                   .sizing = (Clay_Sizing){.width = CLAY_SIZING_GROW(4, 200),
                                           .height = CLAY_SIZING_GROW()}}}){
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
};

Clay_RenderCommandArray ui_build_layout(app_controller_t *app_controller) {
  Clay_SetPointerState((Clay_Vector2){app_controller->input.mouse.x,
                                      app_controller->input.mouse.y},
                       app_controller->input.mouse_left_down);

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

    switch (app_controller_get_state(app_controller)) {
    case APP_STATE_MAIN_MENU:
      ui_build_main_menu(app_controller);
      break;
    case APP_STATE_SETTINGS:
      ui_build_settings_menu(app_controller);
      break;
    case APP_STATE_PLAYING:
      // Render game UI using the game controller
      if (app_controller->game) {
        game_controller_t *game_controller = &app_controller->game_controller;
        switch (game_controller->game->state) {
        case GAME_STATE_PLAYING:
          ui_build_game_area(game_controller);
          ui_build_inventory_area(game_controller);
          break;
        case GAME_STATE_REWARD:
          ui_build_reward_area(game_controller);
          break;
        case GAME_STATE_COLLECT:
          ui_build_game_area(game_controller);
          ui_build_inventory_area(game_controller);
          break;
        default:
          break;
        }
      }
      break;
    case APP_STATE_PAUSED:
      // Render game UI in background, then pause menu overlay
      if (app_controller->game) {
        game_controller_t *game_controller = &app_controller->game_controller;
        ui_build_game_area(game_controller);
        ui_build_inventory_area(game_controller);
      }
      ui_build_pause_menu(app_controller);
      break;
    case APP_STATE_QUIT:
    default:
      break;
    }
  }

  Clay_RenderCommandArray commands = Clay_EndLayout();
  return commands;
}
