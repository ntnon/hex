/**************************************************************************//**
 * @file tile_manager.h
 * @brief Manages the overall game state related to tiles, including tile pools.
 *
 * This acts as the central hub for tile and pool operations, coordinating
 * updates and interactions based on game logic.
 *****************************************************************************/

#ifndef TILE_MANAGER_H
#define TILE_MANAGER_H

//#include "../grid/grid_system.h"
#include "pool_manager.h"
#include "tile.h"
#include "pool_map.h"
#include "tile_map.h"
#include "raylib.h"

// --- Tile Manager Structure ---
/**
 * @brief The main manager for all tiles and their associated pools.
 *
 * This struct holds the global collection of tiles (likely a hash map)
 * and the manager for all tile pools. It's responsible for game logic
 * related to tile interactions and pool updates.
 */
typedef struct tile_manager {
    // Global hash table mapping grid_cell_t to tile_map_entry_t.
    // This is where all active tiles are stored.
    tile_map_entry_t* tiles;

    pool_manager_t* pool_manager;

    grid_t* grid;

    // If you have a global list of all tiles (e.g., for iteration),
    // you could have it here too, though the hash map is primary.
    // tile_t*_collection_t* all_tiles_list;

} tile_manager_t;

// --- Tile Manager Lifecycle ---

/**
 * @brief Creates and initializes a new tile manager.
 * @return A pointer to the newly created tile_manager_t, or NULL on failure.
 */
tile_manager_t* tile_manager_create(grid_t* grid);

/**
 * @brief Frees all memory associated with the tile manager, including
 *        all tiles and all tile pools.
 * @param tm A pointer to the tile_manager_t to be freed.
 */
void tile_manager_free(tile_manager_t* tm);

// --- Tile Management Functions ---

/**
 * @brief Adds a new tile to the manager at a specific cell.
 * If a tile already exists at that cell, it might be replaced or updated
 * depending on game logic.
 * @param tm A pointer to the tile manager.
 * @param cell The grid cell where the tile should be placed.
 * @param value The initial value for the new tile.
 * @param type The type (color/category) of the new tile.
 * @return A pointer to the newly created tile_t entry, or NULL on failure.
 */
void tile_manager_add_tile(tile_manager_t* tm, tile_t* tile);

/**
 * @brief Removes a tile from the manager (and its pools) at a given cell.
 * @param tm A pointer to the tile manager.
 * @param cell The grid cell from which to remove the tile.
 * @return True if a tile was found and removed, false otherwise.
 */
bool tile_manager_remove_tile(tile_manager_t* tm, grid_cell_t cell);

/**
 * @brief Retrieves a tile at a specific cell.
 * @param tm A pointer to the tile manager.
 * @param cell The grid cell to look for.
 * @return A pointer to the tile_t at that cell, or NULL if no tile exists there.
 */
tile_t* tile_manager_get_tile(const tile_manager_t* tm, grid_cell_t cell);

/**
 * @brief Sends the tile to the pool manager where its pool membership is updated.
 * @param tm A pointer to the tile manager.
 * @param tile The tile to be pooled.
 */
void tile_manager_pool_tile(tile_manager_t* tm, tile_t* tile);


void tile_manager_randomize_board(tile_manager_t* tm);

#endif // TILE_MANAGER_H
