/**************************************************************************//**
 * @file tile_pool.h
 * @brief Defines structures and functions for managing tile pools (groups of tiles).
 *****************************************************************************/

#ifndef TILE_POOL_H
#define TILE_POOL_H

#include "../grid/grid_types.h" // For grid_cell_t
#include "../tile/tile_map.h"
#include "../grid/edge_map.h"
#include "tile.h"
#include <stdlib.h>

// --- Enums ---
/**
 * @brief Enum representing the type or color of a tile pool.
 */

 typedef struct tile_pool {
     int id;
     int highest_n;
     grid_cell_t center;
     edge_map_entry_t *edges;
     tile_map_t *tiles;
     tile_type_t accepted_tile_type;
     
     // Pool modifier (can be negative/positive)
     float modifier;
     
     // Geometric properties
     int diameter;              // Farthest distance between any two tiles
     float avg_center_distance; // Average distance from geometric center
     int edge_count;           // External edges
     float compactness_score;  // edge_count / tile_count ratio
 } pool_t;

// --- Pool Lifecycle Functions ---

/**
 * @brief Creates and initializes a new tile pool.
 * @param id A unique identifier for the pool.
 * @param type The type of pool (e.g., POOL_MAGENTA).
 * @param color The associated color for the pool.
 * @return A pointer to the newly created pool_t, or NULL on failure.
 */

pool_t * pool_create (void);
int pool_compatibility_score(const pool_t *pool);

void pool_free(pool_t* pool);

int pool_tile_score(const pool_t *pool);

// --- Pool Membership Functions ---

/**
 * @brief Adds a tile to a pool.
 * @param pool A pointer to the pool to add the tile to.
 * @param tile_ptr A pointer to the tile_t to add.
 */

bool pool_add_tile_to_pool (pool_t *pool, const tile_t *tile, grid_type_e geometry_type);

/**
 * @brief Removes a tile from a pool.
 * @param pool A pointer to the pool to remove the tile from.
 * @param tile_ptr A pointer to the tile_t to remove.
 */
void pool_remove_tile(const pool_t* pool, const tile_t* tile_ptr);

void
pool_update (pool_t *pool, const grid_t *grid);
/**
 * @brief Checks if a specific tile is a member of this pool.
 * @param pool A constant pointer to the pool to check within.
 * @param tile_ptr A pointer to the tile_t to search for.
 * @return True if the tile is in the pool, false otherwise.
 */
bool pool_contains_tile(const pool_t* pool, const tile_t* tile_ptr);

bool pool_accepts_tile_type(const pool_t *pool, tile_type_t type);

void pool_update_edges (const grid_t *grid, pool_t *pool);

void pool_update_center(pool_t *pool);

int compare_pools_by_score(const void *a, const void *b);

int
pool_find_tile_friendly_neighbor_count (tile_map_t *tile_map,
                                        const tile_t *tile, const grid_t *grid);

int
pool_find_max_tile_neighbors_in_pool (pool_t *pool, const grid_t *grid);

void
pool_calculate_score (pool_t *pool, const grid_t *grid);

void
pool_add_tile (pool_t *pool, const tile_t *tile_ptr, grid_type_e geometry_type);

// --- Modifier Functions ---

/**
 * @brief Sets the modifier value for a pool.
 * @param pool Pointer to the pool.
 * @param modifier Modifier value (can be negative or positive).
 */
void pool_set_modifier(pool_t *pool, float modifier);

/**
 * @brief Adds to the current modifier value for a pool.
 * @param pool Pointer to the pool.
 * @param modifier_delta Value to add to current modifier.
 */
void pool_add_modifier(pool_t *pool, float modifier_delta);

/**
 * @brief Gets the modifier value for a pool.
 * @param pool Pointer to the pool.
 * @return The pool's modifier value.
 */
float pool_get_modifier(const pool_t *pool);

// --- Geometric Property Functions ---

/**
 * @brief Calculates and updates all geometric properties of a pool.
 * @param pool Pointer to the pool.
 * @param geometry_type The grid geometry type for calculations.
 */
void pool_update_geometric_properties(pool_t *pool, grid_type_e geometry_type);

/**
 * @brief Calculates the diameter (maximum distance between any two tiles) of a pool.
 * @param pool Pointer to the pool.
 * @param geometry_type The grid geometry type for calculations.
 * @return The diameter of the pool, or 0 if less than 2 tiles.
 */
int pool_calculate_diameter(const pool_t *pool, grid_type_e geometry_type);

/**
 * @brief Calculates the geometric center of a pool.
 * @param pool Pointer to the pool.
 * @param geometry_type The grid geometry type for calculations.
 * @return The geometric center as a grid cell.
 */
grid_cell_t pool_calculate_center(const pool_t *pool, grid_type_e geometry_type);

/**
 * @brief Calculates the average distance from tiles to pool center.
 * @param pool Pointer to the pool.
 * @param geometry_type The grid geometry type for calculations.
 * @return The average distance from tiles to center.
 */
float pool_calculate_avg_center_distance(const pool_t *pool, grid_type_e geometry_type);

/**
 * @brief Calculates the number of external edges of a pool.
 * @param pool Pointer to the pool.
 * @param geometry_type The grid geometry type for calculations.
 * @return The number of external edges.
 */
int pool_calculate_edge_count(const pool_t *pool, grid_type_e geometry_type);

/**
 * @brief Calculates the compactness score of a pool.
 * @param pool Pointer to the pool.
 * @return The compactness score (edge_count / tile_count ratio).
 */
float pool_calculate_compactness_score(const pool_t *pool);

/**
 * @brief Prints all easily printable properties of a pool.
 * @param pool Pointer to the pool to print.
 */
void pool_print(const pool_t *pool);


#endif // TILE_POOL_H
