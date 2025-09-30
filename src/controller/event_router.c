#include "controller/event_router.h"
#include "game/board.h"
#include "game/game_actions.h"
#include "game/inventory.h"
#include "grid/grid_geometry.h"
#include "ui.h"

#include "ui_types.h"
#include <stdio.h>

static void handle_add_inventory_button_click(event_router_t *router);
static void handle_game_area_click(event_router_t *router);
static void handle_tile_placement_click(event_router_t *router);

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
  if (!router->game->board->hovered_grid_cell) {
    printf("No valid hover position on game board\n");
    return;
  }

  grid_cell_t target_position = *(router->game->board->hovered_grid_cell);

  game_actions_t actions;
  game_actions_init(&actions, router->game);
  game_actions_place_tile(&actions, target_position);
}
