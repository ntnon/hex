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
  // Events are now handled directly in UI callbacks
  // This function is kept for compatibility but does nothing
}

void event_router_handle_click(event_router_t *router,
                               Clay_ElementId elementId) {
  router->last_clicked_element_id = elementId;
  printf("Clicked on element: %s\n", elementId.stringId.chars);

  if (router->last_clicked_element_id.id == UI_BUTTON_ADD_INVENTORY_ITEM.id) {
    handle_add_inventory_button_click(router);
  } else if (router->last_clicked_element_id.id == UI_ID_GAME_AREA.id) {
    handle_game_area_click(router);
  }
}

void event_router_handle_hover_start(event_router_t *router,
                                     Clay_ElementId elementId) {
  router->hovered_element_id = elementId;
  // Additional hover start logic can be added here
}

void event_router_handle_hover_end(event_router_t *router,
                                   Clay_ElementId elementId) {
  if (router->hovered_element_id.id == elementId.id) {
    router->hovered_element_id = UI_ID_NONE;
  }
}

void event_router_handle_inventory_click(event_router_t *router,
                                         Clay_ElementId elementId) {
  printf("Inventory item clicked: %s\n", elementId.stringId.chars);

  // Find which inventory item was clicked by matching the element ID
  int inventory_size = inventory_get_size(router->game->inventory);
  for (int i = 0; i < inventory_size; i++) {
    inventory_item_t item = inventory_get_item(router->game->inventory, i);
    if (item.id.id == elementId.id) {
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
