/**************************************************************************//**
 * @file tile.h
 * @brief Defines the basic tile structure and related operations.
 *****************************************************************************/

#ifndef TILE_H
#define TILE_H

#include "../grid/grid_types.h" // For grid_cell_t
#include "../third_party/clay.h"

// --- Enums ---
/**
 * @brief Enum representing the different types/colors of tiles.
 */
typedef enum {
    TILE_UNDEFINED = -1, // Used for uninitialized tiles
    TILE_EMPTY = 0,
    TILE_MAGENTA,
    TILE_CYAN,
    TILE_YELLOW,
    TILE_TYPE_COUNT
} tile_type_t;

typedef struct {
    tile_type_t type;
    int value;
} tile_data_t;

// --- Tile Structure ---
// Represents a single tile placed on a cell.
typedef struct {
    grid_cell_t cell;      // The cell this tile occupies. Crucially, this will be our hash key.
    tile_data_t data;            // Any specific value for the tile (e.g., resource amount).
    uint32_t pool_id;      // ID of the pool this tile belongs to
    // ... other tile-specific data can go here ...
} tile_t;

/**
 * @brief Creates and initializes a new tile_t object.
 * @param cell The grid cell where the tile is located.
 * @param value The value associated with the tile.
 * @param type The type (color/category) of the tile.
 * @return The initialized tile_t.
 */
tile_t* tile_create_ptr(grid_cell_t cell, tile_data_t data);

tile_data_t tile_data_create (tile_type_t type, int value);
tile_data_t tile_data_create_random(void);

tile_t* tile_create_random_ptr(grid_cell_t cell);



void tile_set_coords(tile_t *tile, grid_cell_t coord);
void tile_add_coords(tile_t *tile, grid_cell_t coord);

Clay_Color
tile_get_color_from_type (const tile_data_t tile_data);

void tile_destroy(tile_t *tile);
void tile_cycle(tile_t *tile);

#endif // TILE_H
