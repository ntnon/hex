#ifndef TYPES_H
#define TYPES_H

#include "raylib.h"
#include <stdbool.h>

//------------------------------------------------------------------------------------
// Basic Type Definitions
//------------------------------------------------------------------------------------

// Tile color enumeration for accessibility
typedef enum {
    TILE_COLOR_MAGENTA = 0,
    TILE_COLOR_CYAN,
    TILE_COLOR_YELLOW,
    TILE_COLOR_BRIDGE,
    TILE_COLOR_COUNT
} TileColor;

// Tile color information structure
typedef struct {
    Color color;
    int dotCount;
    const char* name;
} TileColorInfo;

// Tile state enumeration
typedef enum {
    TILE_EMPTY = 0,
    TILE_WORLD_SPAWN,
    TILE_STRUCTURE_HOUSE,
    TILE_STRUCTURE_TOWER,
    TILE_STRUCTURE_FARM,
    TILE_RESOURCE_WOOD,
    TILE_RESOURCE_STONE,
    TILE_RESOURCE_WATER,
    TILE_BLOCKED,
    TILE_SELECTED,
    TILE_BASE,
    TILE_SPECIAL_MAGENTA,
    TILE_SPECIAL_CYAN,
    TILE_SPECIAL_YELLOW,
    TILE_PLACED,
    TILE_DESTROYER
} TileState;

// Tile source enumeration (tracks how tile was created)
typedef enum {
    SOURCE_NATURAL = 0,
    SOURCE_PLAYER,
    SOURCE_SYSTEM
} TileSource;

// Screen state enumeration
typedef enum {
    SCREEN_MENU = 0,
    SCREEN_GAMEPLAY,
    SCREEN_SETTINGS
} ScreenState;

// Maximum values
#define MAX_POOLS 1024
#define MAX_POOL_TILES 4096
#define MAX_HEX_EDGES 32768

#endif // TYPES_H