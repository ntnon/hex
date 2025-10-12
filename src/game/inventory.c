#include "../include/game/inventory.h"
#include "../include/third_party/kvec.h"
#include "ui.h"
#include "utility/string.h"
#include <stdio.h>
#include <stdlib.h>

// Create the inventory
inventory_t *inventory_create() {

  inventory_t *inventory = malloc(sizeof(inventory_t));
  if (inventory == NULL) {
    printf("Failed to allocate memory for inventory.\n");
    return NULL; // Handle memory allocation failure
  }

  kv_init(inventory->items);

  inventory->selected_index = -1;
  inventory->next_element_id = 0;

  return inventory;
}

inventory_item_t inventory_create_item(inventory_t *inv) {
  int next_id = inv->next_element_id++;

  // Create a small board for the inventory item (skip randomization to reduce
  // memory pressure)
  board_t *item_board =
    board_create(GRID_TYPE_HEXAGON, 1, BOARD_TYPE_INVENTORY);
  board_fill(item_board, 0, BOARD_TYPE_INVENTORY);
  return (inventory_item_t){
    .quantity = 1,
    .id = CLAY_IDI(UI_ID_INVENTORY_ITEM_BASE_STRING, next_id),
    .board = item_board};
}

int inventory_get_size(const inventory_t *inv) { return inv->items.n; }

inventory_item_t inventory_get_item(const inventory_t *inv, int index) {
  if (index < 0 || index >= inventory_get_size(inv)) {
    printf("Invalid index: %d\n", index);
    return (inventory_item_t){.quantity = 0, .id = UI_ID_NONE, .board = NULL};
  }
  return kv_A(inv->items, index);
}

// Set the selected index in the inventory
void inventory_set_index(inventory_t *inv, int index) {
  if (inv == NULL) {
    printf("Inventory is NULL.\n");
    return; // Handle NULL inventory
  }
  if (index >= 0 && index < inventory_get_size(inv) &&
      index != inv->selected_index) {
    inv->selected_index = index;
    return;
  }
  if (index == inv->selected_index) {
    inv->selected_index = -1;
    return;
  }
}

int inventory_get_index(inventory_t *inv) { return inv->selected_index; }

void inventory_add_item(inventory_t *inv, inventory_item_t item) {
  kv_push(inventory_item_t, inv->items, item);
}

void inventory_fill(inventory_t *inv, int size) {
  if (inv == NULL) {
    printf("Inventory is NULL.\n");
    return; // Handle NULL inventory
  }

  for (int i = 0; i < size; i++) {
    inventory_add_item(inv, inventory_create_item(inv));
  }

  printf("inventory filled, %d items\n", size);
}

void inventory_destroy_item(inventory_t *inv, int index) {
  if (!inv || index < 0 || index >= inventory_get_size(inv))
    return;

  inventory_item_t *item = &kv_A(inv->items, index);
  if (item->board) {
    free_board(item->board);
    item->board = NULL;
  }
}

void inventory_use_selected(inventory_t *inv) {
  int index = inv->selected_index;
  if (!inv || index < 0 || index >= inventory_get_size(inv))
    return;

  // Free the board before removing the item
  inventory_destroy_item(inv, index);

  // Remove the item
  kv_remove_at(inventory_item_t, inv->items, index);

  // Adjust selected_index
  inv->selected_index = -1;
}

void free_inventory(inventory_t *inv) {
  if (!inv) {
    return;
  }

  // Free all boards in inventory items
  for (int i = 0; i < inventory_get_size(inv); i++) {
    inventory_destroy_item(inv, i);
  }

  kv_destroy(inv->items); // destroy the inventory items
  free(inv);
  printf("Inventory freed\n");
}

void inventory_add_random_item(inventory_t *inv) {
  if (!inv) {
    return;
  }
  inventory_add_item(inv, inventory_create_item(inv));
}

inventory_item_t *inventory_get_selected(inventory_t *inv) {
  if (!inv || inv->selected_index < 0 ||
      inv->selected_index >= inventory_get_size(inv)) {
    return NULL;
  }
  return &kv_A(inv->items, inv->selected_index);
}

board_t *inventory_get_selected_board(inventory_t *inv) {
  inventory_item_t *selected_item = inventory_get_selected(inv);
  if (!selected_item) {
    return NULL;
  }
  return selected_item->board;
}

char *inventory_get_element_id(const inventory_t *inv, int index) {
  if (!inv || index < 0 || index >= inventory_get_size(inv)) {
    return NULL;
  }

  inventory_item_t item = inventory_get_item(inv, index);
  static char id_string[64];
  snprintf(id_string, sizeof(id_string), "%d", item.id.id);
  return id_string;
}

bool inventory_rotate_selected(inventory_t *inv, int rotation_steps) {
  if (!inv || inv->selected_index < 0 ||
      inv->selected_index >= inventory_get_size(inv)) {
    return false;
  }

  inventory_item_t *selected_item = inventory_get_selected(inv);
  if (!selected_item || !selected_item->board) {
    return false;
  }

  grid_cell_t center =
    grid_geometry_get_origin(selected_item->board->geometry_type);
  return board_rotate(selected_item->board, center, rotation_steps);
}
