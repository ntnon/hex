#include "pool_map.h"
#include "tile.h"

typedef struct {
    pool_map_entry_t *pool_map;
    size_t num_pools;
    int next_pool_id;

} pool_manager_t;

pool_manager_t *pool_manager_create(void);
void pool_manager_free(pool_manager_t *pm);
void pool_manager_clear(pool_manager_t *pm);

size_t pool_map_filter_by_tile_type(
    pool_t **pool_candidates,
    size_t num_candidates,
    tile_type_t tile_type,
    pool_t **out_pools,
    size_t max_out_pools
);


pool_t *
pool_manager_create_pool (pool_manager_t *pm, tile_t *tile);

pool_t *
pool_manager_create_pool_with_tile (pool_manager_t *pm, const tile_t *tile);

pool_t *
pool_manager_find_pool(pool_manager_t *pm, pool_t *pool);

pool_map_entry_t *pool_manager_find_entry_by_pool(pool_manager_t *pm, pool_t *pool);

void pool_manager_add_pool(pool_manager_t *pm, pool_t *pool);
void pool_manager_remove_pool(pool_manager_t *pm, pool_t *pool);

pool_t *pool_manager_merge_pools(pool_manager_t *pm, pool_t **pools, int count);

int
pool_manager_get_unique_id(pool_manager_t *pm);

void pool_manager_assign_tile_to_best_neighbor_pool(
    pool_manager_t *pm,
    tile_t *tile,
    tile_t **neighbor_tiles,
    size_t num_neighbor_tiles
);

size_t
pool_manager_find_neighbor_pools_for_tile (pool_manager_t *pm,
                                           tile_t **neighbor_tiles,
                                           size_t num_neighbor_tiles,
                                           pool_t **out_pools,
                                           size_t max_pools);

pool_t *
pool_manager_find_best_neighbor_pool_for_tile (
    pool_manager_t *pm, tile_t *tile, pool_t **pool_candidates, size_t num_pool_candidates);

/**
 * @brief Identifies the best pool for a tile based on its neighbors.
 * @param pm The pool manager.
 * @param tile The tile to find the best pool for.
 */
void pool_manager_find_best_pool(pool_manager_t *pm, tile_map_entry_t *tile);

void pool_manager_assign_tile_to_pool(pool_manager_t *pm, pool_t *pool, const tile_t *tile);

void pool_manager_add_tile_to_pool(pool_manager_t *pm, pool_t *pool, const tile_t *tile);


size_t
filter_pools_by_tile_type (pool_t **pool_candidates, size_t num_candidates,
                           tile_type_t tile_type, pool_t **out_pools,
                           size_t max_out_pools);
