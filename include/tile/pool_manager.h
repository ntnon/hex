#ifndef pool_manager_H
#define pool_manager_H

#include "pool.h"
#include "../third_party/uthash.h"
#include "grid/grid_geometry.h"

// --- Pool Hash Table Entry ---
// This struct is managed by UTHash.
typedef struct pool_manager_entry {
    int id;            // Key: pool ID
    pool_t *pool;      // Value: pointer to the pool
    UT_hash_handle hh; // uthash handle for linking entries
} pool_manager_entry_t;

// --- Pool Map Container ---
// Encapsulates the hash table and associated metadata.
typedef struct pool_manager {
    pool_manager_entry_t *root; // Root hash table pointer
    size_t num_pools;          // Number of pools in the map
    int next_id;
} pool_manager_t;

// Create and initialize a new pool map.
pool_manager_t *pool_manager_create(void);

pool_t* pool_manager_create_pool(pool_manager_t *map);

// Free the entire pool map.
void pool_manager_free(pool_manager_t *map);

// Add a pool to the map, keyed by its id.
void pool_manager_add(pool_manager_t *map, pool_t *pool);

// Remove a pool from the map given its id.
void pool_manager_remove(pool_manager_t *map, int id);

// Find an entry in the pool map by pool id.
pool_manager_entry_t *pool_manager_find_by_id(pool_manager_t *map, int pool_id);

// Direct pool access functions (better ergonomics)
pool_t *pool_manager_get_pool(pool_manager_t *map, int pool_id);
pool_t *pool_manager_get_pool_by_tile(pool_manager_t *map, tile_t* tile);

// Optionally, if you need to map from a tile (or tile id) to a pool, add:
pool_manager_entry_t *pool_manager_find_by_tile(pool_manager_t *map, tile_t* tile);

bool pool_manager_contains_tile(pool_manager_t *map, tile_t* tile);

// --- New Pool Management Functions ---

/**
 * @brief Merges source pool into target pool, moving all tiles and updating neighbors.
 * @param manager The pool manager.
 * @param target_id ID of the pool to merge into.
 * @param source_id ID of the pool to merge from (will be removed).
 * @param geometry_type Grid geometry for neighbor calculations.
 * @param board_tiles All tiles on the board for neighbor lookups.
 */
void pool_manager_merge_pools(pool_manager_t *manager, int target_id, int source_id,
                              grid_type_e geometry_type, tile_map_t *board_tiles);

/**
 * @brief Finds all pools compatible with the given tile based on its neighbors.
 * @param manager The pool manager.
 * @param tile The tile to find compatible pools for.
 * @param neighbor_tiles Array of neighbor tiles.
 * @param neighbor_count Number of neighbors.
 * @param out_pool_ids Output array for compatible pool IDs.
 * @param out_count Output for number of compatible pools found.
 */
void pool_manager_find_compatible_pools(pool_manager_t *manager, tile_t *tile,
                                        tile_t **neighbor_tiles, int neighbor_count,
                                        uint32_t *out_pool_ids, size_t *out_count);

/**
 * @brief Assigns a tile to the appropriate pool or creates a new one.
 * @param manager The pool manager.
 * @param tile The tile to assign.
 * @param geometry_type Grid geometry for neighbor calculations.
 * @param board_tiles All tiles on the board.
 * @return The pool the tile was assigned to, or NULL if singleton.
 */
pool_t *pool_manager_assign_tile(pool_manager_t *manager, tile_t *tile,
                                 grid_type_e geometry_type, tile_map_t *board_tiles);

/**
 * @brief Updates neighbor information for all pools affected by recent changes.
 * @param manager The pool manager.
 * @param affected_cells Array of cells where changes occurred.
 * @param num_affected Number of affected cells.
 * @param geometry_type Grid geometry for neighbor calculations.
 * @param board_tiles All tiles on the board.
 */
void pool_manager_update_affected_pools(pool_manager_t *manager, grid_cell_t *affected_cells,
                                        size_t num_affected, grid_type_e geometry_type,
                                        tile_map_t *board_tiles);

/**
 * @brief Gets all neighboring pools for a given cell.
 * @param manager The pool manager.
 * @param cell The cell to check neighbors for.
 * @param geometry_type Grid geometry for neighbor calculations.
 * @param board_tiles All tiles on the board.
 * @param out_pool_ids Output array for neighbor pool IDs.
 * @param out_count Output for number of neighbor pools found.
 */
void pool_manager_get_neighbor_pools(pool_manager_t *manager, grid_cell_t cell,
                                     grid_type_e geometry_type, tile_map_t *board_tiles,
                                     uint32_t *out_pool_ids, size_t *out_count);

#endif /* pool_manager_H */
