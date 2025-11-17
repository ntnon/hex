#include "../include/game/inventory.h"
#include "../include/third_party/kvec.h"
#include "grid/grid_geometry.h"
#include "tile/tile.h"
#include "ui.h"
#include "utility/string.h"
#include <stdio.h>
#include <stdlib.h>

// Create the inventory
inventory_t *inventory_create(grid_type_e grid_type) {

    inventory_t *inventory = malloc(sizeof(inventory_t));
    if (inventory == NULL) {
        printf("Failed to allocate memory for inventory.\n");
        return NULL; // Handle memory allocation failure
    }

    kv_init(inventory->items);

    inventory->selected_index = -1;
    inventory->next_element_id = 0;
    inventory->grid_type = grid_type;

    return inventory;
}

void inventory_fill_single_tiles(inventory_t *inv) {

    // Create a small board for the inventory item (skip randomization to reduce
    // memory pressure)

    for (tile_type_t t = TILE_EMPTY; t < TILE_TYPE_COUNT; t++) {
        int next_id = inv->next_element_id++;

        // Generate default tile data
        tile_data_t tile_data = tile_data_create_default(t, 1);

        // Create a tile pointer
        tile_t *new_tile =
          tile_create_ptr(grid_geometry_get_origin(inv->grid_type), tile_data);

        // Create a board pointer
        board_t *board = board_create(inv->grid_type, 1, BOARD_TYPE_INVENTORY);

        // Add tile pointer to board
        board_add_tile(board, new_tile);

        // Create inventory item using the board pointer
        inventory_item_t item = {
          .quantity = 1,
          .id = CLAY_IDI(ID_INVENTORY_ITEM_BASE_STRING, inv->next_element_id),
          .board = board};
        // Add inventory item to inventory
        inventory_add_item(inv, item);
    }
}

inventory_item_t inventory_create_item_board(inventory_t *inv, int radius) {
    int next_id = inv->next_element_id++;
    // Create a small board for the inventory item (skip randomization to reduce
    // memory pressure)
    board_t *item_board =
      board_create(inv->grid_type, radius, BOARD_TYPE_INVENTORY);
    board_fill(item_board, radius, BOARD_TYPE_INVENTORY);
    return (inventory_item_t){
      .quantity = 1,
      .id = CLAY_IDI(ID_INVENTORY_ITEM_BASE_STRING, next_id),
      .board = item_board};
}

int inventory_get_size(const inventory_t *inv) { return inv->items.n; }

inventory_item_t inventory_get_item(const inventory_t *inv, int index) {
    if (index < 0 || index >= inventory_get_size(inv)) {
        printf("Invalid index: %d\n", index);
        return (inventory_item_t){.quantity = 0, .id = ID_NONE, .board = NULL};
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

bool inventory_set_selected(inventory_t *inv, int index) {
    if (inv == NULL) {
        printf("Inventory is NULL.\n");
        return false;
    }

    int size = inventory_get_size(inv);

    // Toggle off if same index
    if (index == inv->selected_index) {
        inv->selected_index = -1;
        return false;
    }

    // Bounds check
    if (index < 0 || index >= size) {
        inv->selected_index = -1;
        return false;
    }

    // Select new index
    inv->selected_index = index;
    return true;
}

int inventory_get_index(inventory_t *inv) { return inv->selected_index; }

int inventory_clear_selected_index(inventory_t *inv) {
    if (inv == NULL) {
        return -1; // Handle NULL inventory
    }
    inv->selected_index = -1;
    return 0;
}

void inventory_add_item(inventory_t *inv, inventory_item_t item) {
    kv_push(inventory_item_t, inv->items, item);
}
void inventory_fill_base_tiles(inventory_t *inv) {
    if (inv == NULL) {
        printf("Inventory is NULL.\n");
        return; // Handle NULL inventory
    }

    for (int i = 0; i < 10; i++) {
        inventory_add_item(inv, inventory_create_item_board(inv, i));
    }

    printf("inventory filled, %d items\n", 10);
}

void inventory_fill_random(inventory_t *inv, int size) {
    if (inv == NULL) {
        printf("Inventory is NULL.\n");
        return; // Handle NULL inventory
    }

    for (int i = 0; i < size; i++) {
        inventory_add_item(inv, inventory_create_item_board(inv, i));
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
    inventory_add_item(inv, inventory_create_item_board(inv, rand() % 3));
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
