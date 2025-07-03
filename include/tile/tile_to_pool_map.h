#include "pool.h"
#include "../third_party/uthash.h"

typedef struct {
    tile_t *tile;
    pool_t *pool;
    UT_hash_handle hh;
} tile_to_map_entry_t;
