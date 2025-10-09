#include "controller/event_router.h"
#include "game/board.h"
#include "game/game_actions.h"
#include "game/inventory.h"
#include "game/reward_state.h"
#include "grid/grid_geometry.h"
#include "ui.h"

#include "ui_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void handle_add_inventory_button_click(event_router_t *router);
static void handle_game_area_click(event_router_t *router);
static void handle_tile_placement_click(event_router_t *router);
static void handle_reward_option_click(event_router_t *router, ui_event_t evt);
static void handle_reward_button_click(event_router_t *router, ui_event_t evt);

void event_router_init(event_router_t *router, game_t *game) {
  router->game = game;
  router->last_clicked_element_id = UI_ID_NONE;
  router->hovered_element_id = UI_ID_NONE;
}

void event_router_process_events(event_router_t *router) {
  ui_event_t evt;
  while ((evt = ui_poll_event()).type != UI_EVENT_NONE) {
    switch (evt.type) {
    case UI_EVENT_HOVER_START:
      event_router_handle_hover_start(router, evt);
      break;
    case UI_EVENT_HOVER_END:
      event_router_handle_hover_end(router, evt);
      break;
    case UI_EVENT_CLICK:
      event_router_handle_click(router, evt);
      break;
    case UI_EVENT_INVENTORY_ITEM_CLICK:
      event_router_handle_inventory_click(router, evt);
      break;
    case UI_EVENT_NONE:
    default:
      break;
    }
  }
}

void event_router_handle_click(event_router_t *router, ui_event_t evt) {
  router->last_clicked_element_id = evt.element_id;
  printf("Clicked on element: %s\n", evt.element_id.stringId.chars);

  // Check for reward-related clicks first if in reward state
  if (game_is_in_reward_state(router->game)) {
    // Check for reward option clicks
    if (strncmp(evt.element_id.stringId.chars, UI_ID_REWARD_BASE_STRING,
                strlen(UI_ID_REWARD_BASE_STRING)) == 0) {
      handle_reward_option_click(router, evt);
      return;
    }

    // Check for reward button clicks
    if (evt.element_id.id == CLAY_ID("confirm_button").id ||
        evt.element_id.id == CLAY_ID("skip_button").id) {
      handle_reward_button_click(router, evt);
      return;
    }
  }

  // Regular game click handling
  if (router->last_clicked_element_id.id == UI_BUTTON_ADD_INVENTORY_ITEM.id) {
    handle_add_inventory_button_click(router);
  } else if (router->last_clicked_element_id.id == UI_ID_GAME.id) {
    handle_game_area_click(router);
  }
}

void event_router_handle_hover_start(event_router_t *router, ui_event_t evt) {
  router->hovered_element_id = evt.element_id;
  // Additional hover start logic can be added here
}

void event_router_handle_hover_end(event_router_t *router, ui_event_t evt) {
  if (router->hovered_element_id.id == evt.element_id.id) {
    router->hovered_element_id = UI_ID_NONE;
  }
}

void event_router_handle_inventory_click(event_router_t *router,
                                         ui_event_t evt) {
  printf("Inventory item clicked: %s\n", evt.element_id.stringId.chars);

  // Find which inventory item was clicked by matching the element ID
  int inventory_size = inventory_get_size(router->game->inventory);
  for (int i = 0; i < inventory_size; i++) {
    inventory_item_t item = inventory_get_item(router->game->inventory, i);
    if (item.id.id == evt.element_id.id) {
      inventory_set_index(router->game->inventory, i);
      printf("Selected inventory item at index %d\n", i);
      break;
    }
  }
}

/* Static helper functions */
static void handle_add_inventory_button_click(event_router_t *router) {
  game_actions_t actions;
  game_actions_init(&actions, router->game);
  game_actions_add_inventory_item(&actions);
}

static void handle_game_area_click(event_router_t *router) {
  // TODO: Access input state to check if this was a drag
  // For now, assume it's a click and attempt tile placement
  handle_tile_placement_click(router);
}

static void handle_tile_placement_click(event_router_t *router) {
  // Use the game-level hovered cell which is already calculated
  // We can place tiles even on empty cells, so we don't need to check for
  // hovered_tile
  grid_cell_t target_position = router->game->hovered_cell;

  game_actions_t actions;
  game_actions_init(&actions, router->game);
  game_actions_place_tile(&actions, target_position);
}

static void handle_reward_option_click(event_router_t *router, ui_event_t evt) {
  if (!game_is_in_reward_state(router->game)) {
    return;
  }

  // Extract option index from element ID
  const char *id_str = evt.element_id.stringId.chars;
  const char *index_str = id_str + strlen(UI_ID_REWARD_BASE_STRING);
  int option_index = atoi(index_str);

  printf("Reward option %d clicked\n", option_index);

  // Select the reward option
  if (router->game->reward_state) {
    reward_state_select_option(router->game->reward_state,
                               (uint8_t)option_index);
  }
}

static void handle_reward_button_click(event_router_t *router, ui_event_t evt) {
  if (!game_is_in_reward_state(router->game) || !router->game->reward_state) {
    return;
  }

  if (evt.element_id.id == CLAY_ID("confirm_button").id) {
    printf("Confirm button clicked\n");
    reward_state_confirm_selection(router->game->reward_state);
  } else if (evt.element_id.id == CLAY_ID("skip_button").id) {
    printf("Skip button clicked\n");
    reward_state_skip_selection(router->game->reward_state);
  }
}
