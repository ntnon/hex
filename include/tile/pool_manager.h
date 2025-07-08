#include "pool_map.h"
#include "tile.h"
#include "tile_to_pool_map.h"

typedef struct {
    pool_map_t *pool_map;
} pool_manager_t;

pool_manager_t *pool_manager_create(void);
pool_t *
pool_manager_create_pool (pool_manager_t *pm);
void pool_manager_free(pool_manager_t *pm);
void pool_manager_clear(pool_manager_t *pm);

size_t pool_map_filter_by_tile_type(
    pool_t **pool_candidates,
    size_t num_candidates,
    tile_type_t tile_type,
    pool_t **out_pools,
    size_t max_out_pools
);
void
pool_manager_add_tile_to_pool (pool_manager_t *pm, pool_t *pool, tile_t *tile);

int pool_manager_get_unique_id (pool_manager_t *pm);

void
pool_manager_add_pool (pool_manager_t *pm, pool_t *pool);

//returns the merged pool
pool_t* pool_manager_merge_pools (pool_manager_t *pm,
                               tile_to_pool_entry_t **tile_to_pool_entries,
                               pool_t **pools_to_merge, size_t num_pools
                               );

                               pool_t * pool_manager_find_lowest_pool(pool_manager_t *pm);
