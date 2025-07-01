#include "tile.h"
#include <stdlib.h>

#define HIGHLIGHT_MANAGER_MAX_CAPACITY 30

typedef struct {
    Color color;
}highlight;

typedef struct {
    tile_array *tile_array;
    highlight highlight;
} highlight_manager;

highlight_manager *highlight_manager_create(highlight highlight);
void highlight_manager_set_tile_array(highlight_manager *manager, tile_array tile_array);
void highlight_manager_set_tile(highlight_manager *manager, tile tile);
static void remove_highlight(highlight_manager *manager, tile *tile);
void clear_highlights(highlight_manager *manager);

highlight highlight_create(Color color);

void
highlight_manager_free (highlight_manager *manager);
