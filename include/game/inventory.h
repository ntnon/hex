#ifndef INVENTORY_H
#define INVENTORY_H

#include "../tile/tile.h"
#include "../third_party/kvec.h"
#include "game/board.h"
#include "string.h"

// --- Types ---

typedef struct {
    int quantity;
    Clay_ElementId id;
    board_t *board;
} inventory_item_t;

typedef struct {
    kvec_t(inventory_item_t) items;
    int selected_index;
    int next_element_id;
} inventory_t;

// --- Lifecycle Functions ---

inventory_t* inventory_create(int size);
void free_inventory(inventory_t* inv);

// --- Item Management ---

inventory_item_t inventory_create_item(inventory_t *inv);
void inventory_add_item(inventory_t *inv, inventory_item_t item);
void inventory_add_random_item(inventory_t *inv);
void inventory_destroy_item(inventory_t *inv, int index);
void inventory_use_selected(inventory_t *inv);

// --- Access Functions ---

int inventory_get_size(const inventory_t *inv);
inventory_item_t inventory_get_item(const inventory_t *inv, int index);
char* inventory_get_element_id(const inventory_t *inv, int index);

// --- Selection Functions ---

void inventory_set_index(inventory_t* inv, int index);
inventory_item_t* inventory_get_selected(inventory_t* inv);
board_t* inventory_get_selected_board(inventory_t* inv);

// --- Utility Functions ---

void inventory_fill(inventory_t *inv, int size);
bool inventory_rotate_selected(inventory_t *inv, int rotation_steps);

#endif // INVENTORY_H
