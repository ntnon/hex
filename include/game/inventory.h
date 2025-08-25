#ifndef INVENTORY_H
#define INVENTORY_H

#include "../tile/tile.h"
#include "raylib.h"
#include "../third_party/kvec.h"
#include "string.h"
#include "game/board.h"

typedef struct {
    tile_data_t tile_data;
    int quantity;
    Clay_ElementId id;
    board_t board;
} inventory_item_t;

// Define inventory structure
typedef struct {
    kvec_t(inventory_item_t) items; // Array of inventory items           // Size of the inventory
    int selected_index;      // Currently selected index
    int next_element_id;
} inventory_t;

inventory_t* inventory_create(int size);

int inventory_get_size(const inventory_t *inv);
inventory_item_t inventory_get(const inventory_t *inv, int index);
inventory_item_t inventory_create_item(inventory_t *inv);

void inventory_fill(inventory_t *inv, int size);
void inventory_add_item(inventory_t *inv, inventory_item_t item);
char* inventory_get_element_id(const inventory_t *inv, int index);

void inventory_set_index(inventory_t* inv, int index);
void inventory_render(inventory_t* inv);
void free_inventory(inventory_t* inv);
void inventory_destroy_item(inventory_t *inv, int index);
bool
inventory_hit_test (inventory_t *inv, Vector2 mouse);
void inventory_handle_input(inventory_t *inv);
void inventory_add_random_item(inventory_t *inv);
void inventory_use_selected(inventory_t *inv);

#endif // INVENTORY_H
