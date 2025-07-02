/**************************************************************************//**
 * @file tile_manager.h
 * @brief Manages the overall game state related to tiles, including tile pools.
 *
 * This acts as the central hub for tile and pool operations, coordinating
 * updates and interactions based on game logic.
 *****************************************************************************/

#ifndef TILE_MANAGER_H
#define TILE_MANAGER_H

#include "../grid/grid_types.h"
#include "tile.h"
#include "pool_map.h"
#include "tile_map.h"
#include "../collection_template.h"    // For generic collections
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

    // Hash table managing all active tile pools, keyed by pool ID.
    pool_map_entry_t* pools;

    // A counter to generate unique pool IDs.
    int next_pool_id;

    // If you have a global list of all tiles (e.g., for iteration),
    // you could have it here too, though the hash map is primary.
    // tile_t*_collection_t* all_tiles_list;

} tile_manager_t;

// --- Tile Manager Lifecycle ---

/**
 * @brief Creates and initializes a new tile manager.
 * @return A pointer to the newly created tile_manager_t, or NULL on failure.
 */
tile_manager_t* tile_manager_create(void);

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
void tile_manager_add_tile(tile_manager_t* tm, tile_map_entry_t* tile);

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

// --- Pool Management Functions (Orchestrated by Tile Manager) ---

/**
 * @brief Creates a new tile pool and adds it to the manager.
 * This is likely called when a tile is placed that starts a new group.
 * @param tm A pointer to the tile manager.
 * @param tile_ptr A pointer to the tile that is initiating this new pool.
 * @param type The type of the new pool (e.g., POOL_MAGENTA).
 * @param color The color for the new pool.
 * @return A pointer to the newly created pool_t, or NULL on failure.
 */
pool_t* tile_manager_create_pool_for_tile(tile_manager_t* tm, tile_t* tile_ptr, pool_type_t type, Color color);

/**
 * @brief Finds a pool by its ID.
 * @param tm A pointer to the tile manager.
 * @param pool_id The ID of the pool to find.
 * @return A pointer to the pool_t, or NULL if not found.
 */
pool_t* tile_manager_get_pool_by_id(const tile_manager_t* tm, int pool_id);

/**
 * @brief Adds an existing tile to a specific pool.
 * This function would typically be called during pool detection/creation.
 * @param tm A pointer to the tile manager.
 * @param pool_id The ID of the pool to add the tile to.
 * @param tile_ptr A pointer to the tile_t to add.
 * @return True if successful, false otherwise.
 */
bool tile_manager_add_tile_to_pool(tile_manager_t* tm, int pool_id, tile_t* tile_ptr);

/**
 * @brief Removes a tile from its current pool and potentially re-evaluates pool memberships.
 * This is a complex operation that might involve:
 * 1. Finding which pool the tile belongs to.
 * 2. Removing the tile from that pool.
 * 3. If the pool becomes empty or the tile was the sole member, potentially remove the pool.
 * 4. Potentially re-evaluating neighbors to form new pools or split existing ones.
 * @param tm A pointer to the tile manager.
 * @param tile_ptr A pointer to the tile being removed from its pool.
 * @return True if the tile was successfully removed from a pool, false otherwise.
 */
bool tile_manager_remove_tile_from_pool(tile_manager_t* tm, tile_t* tile_ptr);

/**
 * @brief Updates all tile pools based on the current state of tiles.
 * This is where game logic would trigger a recalculation of pools (e.g., after a tile is placed, removed, or changed).
 * It might involve:
 * - Clearing existing pool memberships.
 * - Iterating through all tiles and running a flood fill to identify new pools.
 * - Updating the global pool manager.
 * @param tm A pointer to the tile manager.
 */
void tile_manager_update_all_pools(tile_manager_t* tm);

#endif // TILE_MANAGER_H
