#include "pool_map.h"
typedef struct {
    grid_t *grid;
    pool_map_entry_t *pool_map;
    tile_to_pool_entry_t *tile_to_pool_map;

}pool_manager_t;

pool_manager_t *pool_manager_create(void);
void pool_manager_free(pool_manager_t *pm);

pool_t *pool_manager_create_pool(pool_manager_t *pm, ...);
void pool_manager_add_tile(pool_manager_t *pm, pool_t *pool, tile_t *tile);
void pool_manager_remove_tile(pool_manager_t *pm, tile_t *tile);
pool_t *pool_manager_get_tile_pool(pool_manager_t *pm, tile_t *tile);
pool_t *pool_manager_merge_pools(pool_manager_t *pm, pool_t **pools, int count);
// ...etc...
