/**************************************************************************//**
 * @file tile.h
 * @brief Defines the basic tile structure and related operations.
 *****************************************************************************/

#ifndef TILE_H
#define TILE_H

#include "grid/grid_types.h"
#include "third_party/clay.h"

// Forward declarations
typedef struct tile_map tile_map_t;
typedef struct grid_t grid_t;

/**
 * @brief Enum representing the different types/colors of tiles.
 */
typedef enum {
    TILE_UNDEFINED = -1, // Used for uninitialized tiles
    TILE_EMPTY = 0,
    TILE_MAGENTA,
    TILE_CYAN,
    TILE_YELLOW,
    TILE_GREEN,
    TILE_TYPE_COUNT
} tile_type_t;

typedef struct {
    tile_type_t type;
    int value;
} tile_data_t;

/**
 * @brief Represents a single tile placed on a cell.
 */
typedef struct {
    grid_cell_t cell;    // The cell this tile occupies. Crucially, this will be our hash key.
    tile_data_t data;    // Any specific value for the tile (e.g., resource amount).
    uint32_t pool_id;    // ID of the pool this tile belongs to
    
    // 3-bit range (0-7), stored efficiently
    uint8_t range : 3;
    uint8_t _padding : 5;  // Reserved for future use
    
    // Per-tile modifier (can be negative/positive)
    float modifier;
} tile_t;

/**
 * @brief Creates and initializes a new tile_t object.
 * @param cell The grid cell where the tile is located.
 * @param value The value associated with the tile.
 * @param type The type (color/category) of the tile.
 * @return The initialized tile_t.
 */
tile_t* tile_create_ptr(grid_cell_t cell, tile_data_t data);

tile_data_t tile_data_create(tile_type_t type, int value);
tile_data_t tile_data_create_random(void);

tile_t* tile_create_random_ptr(grid_cell_t cell);

tile_t* tile_create_center_ptr(grid_cell_t cell);

void tile_set_coords(tile_t *tile, grid_cell_t coord);
void tile_add_coords(tile_t *tile, grid_cell_t coord);

Clay_Color tile_get_color_from_type(const tile_data_t tile_data);

void tile_destroy(tile_t *tile);
void tile_cycle(tile_t *tile);

// --- Range and Modifier Functions ---

/**
 * @brief Sets the range value for a tile (clamped to 0-7).
 * @param tile Pointer to the tile.
 * @param range Range value (will be clamped to 0-7).
 */
void tile_set_range(tile_t *tile, uint8_t range);

/**
 * @brief Gets the range value for a tile.
 * @param tile Pointer to the tile.
 * @return The tile's range (0-7).
 */
uint8_t tile_get_range(const tile_t *tile);

/**
 * @brief Sets the modifier value for a tile.
 * @param tile Pointer to the tile.
 * @param modifier Modifier value (can be negative or positive).
 */
void tile_set_modifier(tile_t *tile, float modifier);

/**
 * @brief Adds to the current modifier value for a tile.
 * @param tile Pointer to the tile.
 * @param modifier_delta Value to add to current modifier.
 */
void tile_add_modifier(tile_t *tile, float modifier_delta);

/**
 * @brief Gets the modifier value for a tile.
 * @param tile Pointer to the tile.
 * @return The tile's modifier value.
 */
float tile_get_modifier(const tile_t *tile);

/**
 * @brief Gets the effective production value (base value + modifier).
 * @param tile Pointer to the tile.
 * @return The effective production as a float.
 */
float tile_get_effective_production(const tile_t *tile);

// --- Range Calculation Functions ---

/**
 * @brief Gets all tiles within range of a given tile.
 * @param tile The center tile to calculate range from.
 * @param tile_map The tile map to search for tiles.
 * @param grid The grid system for coordinate calculations.
 * @param out_tiles Pointer to store array of tile pointers within range.
 * @param out_count Pointer to store the number of tiles found.
 * @note Caller is responsible for freeing the allocated array.
 */
void tile_get_tiles_in_range(const tile_t *tile, const tile_map_t *tile_map, 
                            const grid_t *grid, tile_t ***out_tiles, size_t *out_count);

/**
 * @brief Gets all coordinates within range of a tile (regardless of whether tiles exist there).
 * @param tile The center tile to calculate range from.
 * @param grid The grid system for coordinate calculations.
 * @param out_cells Pointer to store array of coordinates within range.
 * @param out_count Pointer to store the number of coordinates.
 * @note Caller is responsible for freeing the allocated array.
 */
void tile_get_coordinates_in_range(const tile_t *tile, const grid_t *grid,
                                  grid_cell_t **out_cells, size_t *out_count);




#endif // TILE_H
