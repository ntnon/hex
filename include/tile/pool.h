/**************************************************************************//**
 * @file tile_pool.h
 * @brief Defines structures and functions for managing tile pools (groups of tiles).
 *****************************************************************************/

#ifndef TILE_POOL_H
#define TILE_POOL_H

#include "../third_party/uthash.h"
#include "../grid/grid_types.h" // For grid_cell_t
#include "../tile/tile_map.h"
#include "../grid/edge_map.h"
#include "tile.h"
#include "raylib.h"
// --- Enums ---
/**
 * @brief Enum representing the type or color of a tile pool.
 */
 #define MAX_ACCEPTED_TILE_TYPES 8

 typedef struct tile_pool {
     int id;
     bool is_mixed;
     Color color;
     edge_map_entry_t *edges;
     tile_map_entry_t *tiles;
     tile_type_t accepted_tile_types[MAX_ACCEPTED_TILE_TYPES]; // <-- new field
     size_t num_accepted_tile_types;                          // <-- new field
 } pool_t;

// --- Pool Lifecycle Functions ---

/**
 * @brief Creates and initializes a new tile pool.
 * @param id A unique identifier for the pool.
 * @param type The type of pool (e.g., POOL_MAGENTA).
 * @param color The associated color for the pool.
 * @return A pointer to the newly created pool_t, or NULL on failure.
 */

 pool_t * pool_create(int id, const tile_t *tile);

int pool_score(const pool_t *pool);

void pool_add_accepted_tile_type(pool_t* pool, tile_type_t type);

/**
 * @brief Frees all memory associated with a tile pool, including its members.
 * @param pool A pointer to the pool_t to be freed.
 */
void pool_free(pool_t* pool);

// --- Pool Membership Functions ---

/**
 * @brief Adds a tile to a pool.
 * @param pool A pointer to the pool to add the tile to.
 * @param tile_ptr A pointer to the tile_t to add.
 */
void pool_add_tile(pool_t* pool, const tile_t* tile_ptr);

/**
 * @brief Removes a tile from a pool.
 * @param pool A pointer to the pool to remove the tile from.
 * @param tile_ptr A pointer to the tile_t to remove.
 */
void pool_remove_tile(const pool_t* pool, const tile_t* tile_ptr);

/**
 * @brief Checks if a specific tile is a member of this pool.
 * @param pool A constant pointer to the pool to check within.
 * @param tile_ptr A pointer to the tile_t to search for.
 * @return True if the tile is in the pool, false otherwise.
 */
bool pool_contains_tile(const pool_t* pool, const tile_t* tile_ptr);


bool pool_contains_tile(const pool_t* pool, const tile_t* tile_ptr);

void pool_update_edges (const grid_t *grid, pool_t *pool);

bool pool_accepts_tile_type(const pool_t *pool, tile_type_t type);

int compare_pools_by_score(const void *a, const void *b);

#endif // TILE_POOL_H
