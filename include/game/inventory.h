#ifndef INVENTORY_H
#define INVENTORY_H

#include "../tile/tile.h"
#include "../types.h"
#include "../third_party/kvec.h"


// Define inventory structure
typedef struct {
    kvec_t(tile_data_t) items; // Array of inventory items           // Size of the inventory
    int selected_index;      // Currently selected index
} inventory_t;

inventory_t* inventory_create(int size);

int inventory_get_size(const inventory_t *inv);

tile_data_t
inventory_create_item (void);

void inventory_fill(inventory_t *inv, int size);

void inventory_set_index(inventory_t* inv, int index);
void inventory_render(inventory_t* inv);
void free_inventory(inventory_t* inv);
void inventory_destroy_item(inventory_t *inv, int index);
bool
inventory_hit_test (inventory_t *inv, vec2_t mouse);
void inventory_handle_input(inventory_t *inv);

void inventory_use_selected(inventory_t *inv);

#endif // INVENTORY_H
