#include "../tile/tile.h"

typedef struct {
tile_t* tile;
} inv_item_t;

typedef struct{
    inv_item_t**  item_array;
    int size;
    int selected_index;
} inv_t;

inv_t* create_inventory(int size);

void inventory_set_index(inv_t* inv, int index);

void inventory_render   (inv_t* inv);
void free_inventory(inv_t* inv);
