#ifndef TILE_H
#define TILE_H

#include "hex_grid.h"
#include "raylib.h"
#include <stdbool.h>

// tile types
typedef enum {
    TILE_EMPTY = 0,
    TILE_MAGENTA,
    TILE_CYAN,
    TILE_YELLOW
} tile_type;

// tile structure
typedef struct {
    int id;
    hex coord;
    tile_type type;
    int value;
} tile;

// tile array for managing collections
typedef struct {
    tile* data;
    int count;
    int capacity;
} tile_array;

// tile operations
tile tile_create_empty(hex coord);
tile tile_create(hex coord, tile_type type, int value);
tile_array tile_array_create(void);
void tile_array_push(tile_array* array, tile tile);
void tile_array_clear(tile_array *tile_array);
void tile_array_free(tile_array* array);
tile* find_tile_by_coord(tile_array* array, hex coord);

// tile properties
Color get_tile_color(tile_type type);

// tile rendering
void draw_tile(layout layout, tile tile);
void draw_tile_array(layout layout, tile_array* tiles);

#endif // TILE_H
