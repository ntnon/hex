#include "game/inventory.h"
#include "game/reward_state.h"
#include "game/reward_system.h"
#include "raylib.h"
#include "stdio.h"
#include "string.h"
#include "tile/tile.h"
#include "ui.h"
#include "ui_types.h"

#define INVENTORY_PADDING 10
#define INVENTORY_GAP INVENTORY_PADDING

bool is_id_valid(const Clay_ElementId id) { return id.id != UI_ID_NONE.id; }

static void ui_build_add_inventory_button(game_controller_t *controller);
static void ui_build_reward_area(game_controller_t *controller);
static void ui_build_reward(game_controller_t *controller);
static void ui_build_reward_header(const reward_state_t *reward_state);
static void ui_build_reward_option(const reward_option_t *option, uint8_t index,
                                   const reward_state_t *reward_state);
static void ui_build_reward_buttons(game_controller_t *controller);

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
  if (!controller || !controller->game || !controller->game->reward_state) {
    return;
  }

  const reward_state_t *reward_state = controller->game->reward_state;

  // Get reward options
  const reward_option_t *options;
  uint8_t option_count;
  if (!reward_state_get_options(reward_state, &options, &option_count)) {
    CLAY({.id = CLAY_ID("no_rewards"),
          .backgroundColor = M_GRAY,
          .layout = {.padding = CLAY_PADDING_ALL(20)}}) {
      CLAY_TEXT(CLAY_STRING("No rewards available"), &TEXT_CONFIG_MEDIUM);
    }
    return;
  }

  // Build header
  ui_build_reward_header(reward_state);

  // Build reward options
  CLAY({.id = CLAY_ID("reward_options"),
        .layout = {
          .layoutDirection = CLAY_LEFT_TO_RIGHT,
          .sizing = {.width = CLAY_SIZING_GROW(), .height = CLAY_SIZING_FIT()},
          .childGap = 20,
          .padding = CLAY_PADDING_ALL(20)}}) {

    for (uint8_t i = 0; i < option_count; i++) {
      ui_build_reward_option(&options[i], i, reward_state);
    }
  }

  // Build buttons (confirm, skip, etc.)
  ui_build_reward_buttons(controller);
};

static void ui_build_reward_header(const reward_state_t *reward_state) {
  if (!reward_state) {
    return;
  }

  const char *phase_name = "Presenting"; // Simplified for now
  const char *trigger_name = "Manual";   // Simplified for now

  CLAY({.id = CLAY_ID("reward_header"),
        .backgroundColor = M_DARKGRAY,
        .layout = {
          .sizing = {.width = CLAY_SIZING_GROW(), .height = CLAY_SIZING_FIT()},
          .padding = CLAY_PADDING_ALL(20),
          .childGap = 10,
          .layoutDirection = CLAY_TOP_TO_BOTTOM}}) {

    CLAY_TEXT(CLAY_STRING("Choose Your Reward"), &TEXT_CONFIG_LARGE);

    static char subtitle[128];
    snprintf(subtitle, sizeof(subtitle), "Triggered by: %s | Phase: %s",
             trigger_name, phase_name);
    CLAY_TEXT(CLAY_STRING(subtitle), &TEXT_CONFIG_MEDIUM);
  }
}

static void ui_build_reward_option(const reward_option_t *option, uint8_t index,
                                   const reward_state_t *reward_state) {
  if (!option || !reward_state) {
    return;
  }

  // Simplified display information
  bool is_selected = false; // TODO: implement selection tracking
  bool is_hovered = false;  // TODO: implement hover tracking

  // Calculate colors based on state
  Clay_Color bg_color =
    is_selected ? M_GREEN : (is_hovered ? M_LIGHTGRAY : M_BEIGE);
  Clay_Color border_color = {.r = 100, .g = 100, .b = 100, .a = 255};

  CLAY({.id = CLAY_IDI(UI_ID_REWARD_BASE_STRING, index),
        .backgroundColor = bg_color,
        .border = {.color = border_color, .width = is_selected ? 3 : 1},
        .cornerRadius = CLAY_CORNER_RADIUS(8),
        .layout = {.sizing = {.width = CLAY_SIZING_FIXED(280),
                              .height = CLAY_SIZING_FIXED(200)},
                   .layoutDirection = CLAY_TOP_TO_BOTTOM,
                   .childGap = 8,
                   .padding = CLAY_PADDING_ALL(16)}}) {

    // Header with rarity and category
    CLAY({.id = CLAY_IDI("reward_option_header", index),
          .layout = {.layoutDirection = CLAY_LEFT_TO_RIGHT,
                     .sizing = {.width = CLAY_SIZING_GROW(),
                                .height = CLAY_SIZING_FIT()},
                     .childAlignment = {.y = CLAY_ALIGN_Y_CENTER},
                     .childGap = 8}}) {

      CLAY_TEXT(CLAY_STRING("⚡"), &TEXT_CONFIG_MEDIUM); // Simplified icon
      CLAY_TEXT(CLAY_STRING("Common"),
                &TEXT_CONFIG_MEDIUM); // Simplified rarity
      CLAY_TEXT(CLAY_STRING("Production"),
                &TEXT_CONFIG_MEDIUM); // Simplified category
    }

    // Title
    CLAY_TEXT(CLAY_STRING("Sample Reward"), &TEXT_CONFIG_LARGE);

    // Description
    CLAY_TEXT(CLAY_STRING("This is a sample reward description"),
              &TEXT_CONFIG_MEDIUM);

    // Power score
    static char power_text[64];
    snprintf(power_text, sizeof(power_text), "Power: %.1f", 2.5f);
    CLAY_TEXT(CLAY_STRING(power_text), &TEXT_CONFIG_MEDIUM);

    // Flavor text
    CLAY({.id = CLAY_IDI("reward_flavor", index),
          .layout = {.padding = CLAY_PADDING_ALL(4)}}) {
      CLAY_TEXT(CLAY_STRING("Sample flavor text"), &TEXT_CONFIG_MEDIUM);
    }
  }
}

static void ui_build_reward_buttons(game_controller_t *controller) {
  if (!controller || !controller->game || !controller->game->reward_state) {
    return;
  }

  const reward_state_t *reward_state = controller->game->reward_state;
  bool can_interact = true;   // TODO: implement proper interaction checking
  bool has_selection = false; // TODO: implement selection checking

  CLAY({.id = CLAY_ID("reward_buttons"),
        .layout = {
          .layoutDirection = CLAY_LEFT_TO_RIGHT,
          .sizing = {.width = CLAY_SIZING_GROW(), .height = CLAY_SIZING_FIT()},
          .childGap = 20,
          .padding = CLAY_PADDING_ALL(20),
          .childAlignment = {.x = CLAY_ALIGN_X_CENTER}}}) {

    // Confirm button
    if (has_selection && can_interact) {
      CLAY({.id = CLAY_ID("confirm_button"),
            .backgroundColor = M_GREEN,
            .border = {.color = M_DARKGREEN, .width = 2},
            .cornerRadius = CLAY_CORNER_RADIUS(6),
            .layout = {.padding = CLAY_PADDING_ALL(12)}}) {
        CLAY_TEXT(CLAY_STRING("Confirm Selection"), &TEXT_CONFIG_MEDIUM);
      }
    }

    // Skip button (if allowed) - simplified for now
    if (can_interact) {
      CLAY({.id = CLAY_ID("skip_button"),
            .backgroundColor = M_LIGHTGRAY,
            .border = {.color = M_GRAY, .width = 2},
            .cornerRadius = CLAY_CORNER_RADIUS(6),
            .layout = {.padding = CLAY_PADDING_ALL(12)}}) {
        CLAY_TEXT(CLAY_STRING("Skip Reward"), &TEXT_CONFIG_MEDIUM);
      }
    }

    // Phase info
    CLAY({.id = CLAY_ID("phase_info"),
          .layout = {.padding = CLAY_PADDING_ALL(8)}}) {
      CLAY_TEXT(CLAY_STRING("Phase: Presenting"), &TEXT_CONFIG_MEDIUM);
    }
  }
}

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
          // Add tile info card overlay
          ui_build_tile_info_card(game_controller->game,
                                  (Vector2){app_controller->input.mouse.x,
                                            app_controller->input.mouse.y});
          break;
        case GAME_STATE_REWARD:
          ui_build_reward_area(game_controller);
          break;
        case GAME_STATE_COLLECT:
          ui_build_game_area(game_controller);
          ui_build_inventory_area(game_controller);
          // Add tile info card overlay
          ui_build_tile_info_card(game_controller->game,
                                  (Vector2){app_controller->input.mouse.x,
                                            app_controller->input.mouse.y});
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
        // Add tile info card overlay even when paused
        ui_build_tile_info_card(game_controller->game,
                                (Vector2){app_controller->input.mouse.x,
                                          app_controller->input.mouse.y});
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
