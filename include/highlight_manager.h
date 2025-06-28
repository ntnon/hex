#include "tile.h"
#include <stdlib.h>

#define HIGHLIGHT_MANAGER_MAX_CAPACITY 30

typedef struct {
    Color color;
}highlight;

typedef struct {
    tile_array tile_array;
    highlight highlight;
    int count;
    int capacity;
} highlight_manager;

highlight_manager highlight_manager_create();
void highlight_manager_update(highlight_manager *manager, tile_array *tile_array, Color highlight_color);

static void remove_highlight(highlight_manager *manager, tile *tile);
static void clear_highlights(highlight_manager *manager);

void
highlight_manager_free (highlight_manager *manager);
