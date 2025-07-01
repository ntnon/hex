#ifndef TILE_H
#define TILE_H

#include "hex_grid.h"
#include "raylib.h"
#include <stdbool.h>
#include "piece.h"

// tile types
typedef enum {
    TILE_EMPTY = 0,
    TILE_MAGENTA,
    TILE_CYAN,
    TILE_YELLOW,
    TILE_TYPE_COUNT
} tile_type;

// tile structure
typedef struct {
    int id;
    hex hex;
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
tile tile_create_empty(hex hex);
tile tile_create(hex hex, tile_type type, int value);
tile tile_create_random(hex hex);

void tile_cycle(tile *tile);
void tile_array_set_tile(tile_array* array, int index, tile tile);
tile* get_tile_by_index(const tile_array* array, int index);
int
get_tile_index (const tile_array *tile_array, tile *tile);

tile_array* tile_array_create(void);
void tile_array_push(tile_array* array, tile tile);
void tile_array_clear(tile_array *tile_array);
void tile_array_free(tile_array* tile_array);

// utility
tile* get_tile_by_hex(const tile_array* array, hex hex);

// tile properties
Color get_tile_type_color(tile_type type);
const char *get_tile_type_name(tile_type type);
void tile_cycle(tile *tile);

// tile rendering
// void draw_hex
void draw_tile(layout layout, tile tile);
void draw_tile_array(layout layout, tile_array* tiles);

#endif // TILE_H
