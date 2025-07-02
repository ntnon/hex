/**************************************************************************//**
 * @file tile.h
 * @brief Defines the basic tile structure and related operations.
 *****************************************************************************/

#ifndef TILE_H
#define TILE_H

#include "../grid/grid_types.h" // For grid_cell_t
#include "raylib.h"          // For Color

// --- Enums ---
/**
 * @brief Enum representing the different types/colors of tiles.
 */
typedef enum {
    TILE_EMPTY = 0,
    TILE_MAGENTA,
    TILE_CYAN,
    TILE_YELLOW,
    TILE_COUNT
} tile_type_t;

// --- Tile Structure ---
// Represents a single tile placed on a cell.
typedef struct {
    grid_cell_t cell;      // The cell this tile occupies. Crucially, this will be our hash key.
    int value;             // Any specific value for the tile (e.g., resource amount).
    tile_type_t type;      // The visual type/color of the tile.
    // ... other tile-specific data can go here ...
} tile_t;

/**
 * @brief Creates and initializes a new tile_t object.
 * @param cell The grid cell where the tile is located.
 * @param value The value associated with the tile.
 * @param type The type (color/category) of the tile.
 * @return The initialized tile_t.
 */
tile_t tile_create(grid_cell_t cell, tile_type_t type, int value);

#endif // TILE_H
