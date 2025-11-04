#include "../../include/tile/pool_manager.h"
#include <stdio.h>

pool_manager_t *pool_manager_create(void) {
    pool_manager_t *map = malloc(sizeof(pool_manager_t));
    if (!map) {
        fprintf(stderr, "Out of memory!\n");
        return NULL;
    }
    map->root = NULL;
    map->num_pools = 0;
    map->next_id = 1; // Start from 1 since 0 means "no pool"
    return map;
}

void pool_manager_free(pool_manager_t *map) {
    if (!map)
        return;
    pool_manager_entry_t *el, *tmp;
    HASH_ITER(hh, map->root, el, tmp) {
        HASH_DEL(map->root, el);
        free(el);
    }
    map->num_pools = 0;
    map->next_id = 1; // Reset to 1 since 0 means "no pool"
    free(map);
}

pool_t *pool_manager_create_pool(pool_manager_t *map) {
    if (!map)
        return NULL;

    // Create a new pool; assumes pool_create returns a pool with id = -1.
    pool_t *pool = pool_create();
    if (!pool)
        return NULL;

    // Add it to the map (this function will assign a unique id).
    pool_manager_add(map, pool);

    // Return the registered pool to the caller.
    return pool;
}

void pool_manager_add(pool_manager_t *map, pool_t *pool) {
    if (!map || !pool)
        return;

    // Remove any existing entry with the same pool id to prevent duplicates.
    pool_manager_entry_t *existing = pool_manager_find_by_id(map, pool->id);
    if (existing) {
        fprintf(stderr, "ERROR: Duplicate pool ID %d\n", pool->id);
        HASH_DEL(map->root, existing);
        free(existing);
        map->num_pools--;
    }

    // Assign a new unique ID to the pool.
    pool->id = map->next_id++;

    pool_manager_entry_t *entry = malloc(sizeof(pool_manager_entry_t));
    if (!entry) {
        fprintf(stderr, "Out of memory!\n");
        return;
    }

    entry->id = pool->id;
    entry->pool = pool;
    HASH_ADD_INT(map->root, id, entry);
    map->num_pools++;
}

pool_manager_entry_t *pool_manager_find_by_tile(pool_manager_t *map,
                                                tile_t *tile) {
    pool_manager_entry_t *entry = NULL;
    for (entry = map->root; entry != NULL; entry = entry->hh.next) {
        if (pool_contains_tile(entry->pool, tile))
            return entry;
    }
    return NULL;
}

bool pool_manager_contains_tile(pool_manager_t *map, tile_t *tile) {
    if (!map || !tile)
        return false;
    pool_manager_entry_t *entry = pool_manager_find_by_tile(map, tile);
    return entry != NULL;
}

void pool_manager_remove(pool_manager_t *map, int id) {
    if (!map)
        return;
    pool_manager_entry_t *entry_to_remove = NULL;
    HASH_FIND_INT(map->root, &id, entry_to_remove);
    if (entry_to_remove) {
        HASH_DEL(map->root, entry_to_remove);
        free(entry_to_remove);
        map->num_pools--;
    }
}

pool_manager_entry_t *pool_manager_find_by_id(pool_manager_t *map,
                                              int pool_id) {
    pool_manager_entry_t *entry = NULL;
    HASH_FIND_INT(map->root, &pool_id, entry);
    return entry;
}

// Direct pool access functions (better ergonomics)
pool_t *pool_manager_get_pool(pool_manager_t *map, int pool_id) {
    pool_manager_entry_t *entry = pool_manager_find_by_id(map, pool_id);
    return entry ? entry->pool : NULL;
}

pool_t *pool_manager_get_pool_by_tile(pool_manager_t *map, tile_t *tile) {
    pool_manager_entry_t *entry = pool_manager_find_by_tile(map, tile);
    return entry ? entry->pool : NULL;
}

// --- New Pool Management Functions ---

void pool_manager_merge_pools(pool_manager_t *manager, int target_id,
                              int source_id, grid_type_e geometry_type,
                              tile_map_t *board_tiles) {
    if (!manager || target_id == source_id)
        return;

    pool_t *target_pool = pool_manager_get_pool(manager, target_id);
    pool_t *source_pool = pool_manager_get_pool(manager, source_id);

    if (!target_pool || !source_pool)
        return;

    // Move all tiles from source to target
    tile_map_entry_t *tile_entry, *tmp;
    HASH_ITER(hh, source_pool->tiles->root, tile_entry, tmp) {
        tile_t *tile_to_move = tile_entry->tile;
        tile_to_move->pool_id = target_id;

        // Remove from source pool's tile map
        tile_map_remove(source_pool->tiles, tile_to_move->cell);

        // Add to target pool
        pool_add_tile(target_pool, tile_to_move, geometry_type, board_tiles);
    }

    // Remove the source pool
    pool_manager_remove(manager, source_id);
}

void pool_manager_find_compatible_pools(pool_manager_t *manager, tile_t *tile,
                                        tile_t **neighbor_tiles,
                                        int neighbor_count,
                                        uint32_t *out_pool_ids,
                                        size_t *out_count) {
    if (!manager || !tile || !out_pool_ids || !out_count)
        return;

    *out_count = 0;

    for (int i = 0; i < neighbor_count; i++) {
        if (!neighbor_tiles[i])
            continue;

        // Check if neighbor is same type
        if (neighbor_tiles[i]->data.type != tile->data.type)
            continue;

        // Skip singletons (pool_id == 0)
        if (neighbor_tiles[i]->pool_id == 0)
            continue;

        // Check if we already have this pool ID
        bool already_added = false;
        for (size_t j = 0; j < *out_count; j++) {
            if (out_pool_ids[j] == neighbor_tiles[i]->pool_id) {
                already_added = true;
                break;
            }
        }

        if (!already_added) {
            out_pool_ids[(*out_count)++] = neighbor_tiles[i]->pool_id;
        }
    }
}

pool_t *pool_manager_assign_tile(pool_manager_t *manager, tile_t *tile,
                                 grid_type_e geometry_type,
                                 tile_map_t *board_tiles) {
    if (!manager || !tile || !board_tiles)
        return NULL;

    // Get neighbors
    int neighbor_count = grid_geometry_get_neighbor_count(geometry_type);
    grid_cell_t neighbor_cells[neighbor_count];
    grid_geometry_get_all_neighbors(geometry_type, tile->cell, neighbor_cells);

    // Convert to tile pointers
    tile_t *neighbor_tiles[neighbor_count];
    bool has_same_type_neighbors = false;
    for (int i = 0; i < neighbor_count; i++) {
        tile_map_entry_t *entry = tile_map_find(board_tiles, neighbor_cells[i]);
        neighbor_tiles[i] = entry ? entry->tile : NULL;
        if (neighbor_tiles[i] &&
            neighbor_tiles[i]->data.type == tile->data.type) {
            has_same_type_neighbors = true;
        }
    }

// Find compatible pools
#define MAX_COMPATIBLE_POOLS 20
    uint32_t compatible_pool_ids[MAX_COMPATIBLE_POOLS];
    size_t num_compatible_pools = 0;
    pool_manager_find_compatible_pools(manager, tile, neighbor_tiles,
                                       neighbor_count, compatible_pool_ids,
                                       &num_compatible_pools);

    pool_t *target_pool = NULL;

    if (num_compatible_pools == 0) {
        if (has_same_type_neighbors) {
            // Create new pool for this tile and any singleton neighbors
            target_pool = pool_manager_create_pool(manager);
            target_pool->accepted_tile_type = tile->data.type;
            tile->pool_id = target_pool->id;

            // Add singleton neighbors to the new pool
            for (int i = 0; i < neighbor_count; i++) {
                if (neighbor_tiles[i] &&
                    neighbor_tiles[i]->data.type == tile->data.type &&
                    neighbor_tiles[i]->pool_id == 0) {
                    neighbor_tiles[i]->pool_id = target_pool->id;
                    pool_add_tile(target_pool, neighbor_tiles[i], geometry_type,
                                  board_tiles);
                }
            }
        } else {
            // No same-type neighbors - remain singleton
            tile->pool_id = 0;
            return NULL;
        }
    } else if (num_compatible_pools == 1) {
        // Join the single compatible pool
        target_pool = pool_manager_get_pool(manager, compatible_pool_ids[0]);
        tile->pool_id = target_pool->id;
    } else {
        // Multiple pools - use first as target and merge others into it
        target_pool = pool_manager_get_pool(manager, compatible_pool_ids[0]);
        tile->pool_id = target_pool->id;

        for (size_t i = 1; i < num_compatible_pools; i++) {
            pool_manager_merge_pools(manager, compatible_pool_ids[0],
                                     compatible_pool_ids[i], geometry_type,
                                     board_tiles);
        }
    }

    // Add tile to the pool
    if (target_pool) {
        pool_add_tile(target_pool, tile, geometry_type, board_tiles);

        // Add any remaining singleton neighbors
        for (int i = 0; i < neighbor_count; i++) {
            if (neighbor_tiles[i] &&
                neighbor_tiles[i]->data.type == tile->data.type &&
                neighbor_tiles[i]->pool_id == 0) {
                neighbor_tiles[i]->pool_id = target_pool->id;
                pool_add_tile(target_pool, neighbor_tiles[i], geometry_type,
                              board_tiles);
            }
        }
    }

    return target_pool;
}

void pool_manager_update_affected_pools(pool_manager_t *manager,
                                        grid_cell_t *affected_cells,
                                        size_t num_affected,
                                        grid_type_e geometry_type,
                                        tile_map_t *board_tiles) {
    if (!manager || !affected_cells || num_affected == 0 || !board_tiles)
        return;

// Track which pools need updates (using a simple array since pool count is
// usually small)
#define MAX_POOLS_TO_UPDATE 100
    int pools_to_update[MAX_POOLS_TO_UPDATE];
    size_t num_pools_to_update = 0;

    // For each affected cell, find all neighboring pools
    for (size_t i = 0; i < num_affected; i++) {
        int neighbor_count = grid_geometry_get_neighbor_count(geometry_type);
        grid_cell_t neighbor_cells[neighbor_count];
        grid_geometry_get_all_neighbors(geometry_type, affected_cells[i],
                                        neighbor_cells);

        for (int j = 0; j < neighbor_count; j++) {
            tile_map_entry_t *entry =
              tile_map_find(board_tiles, neighbor_cells[j]);
            if (entry && entry->tile && entry->tile->pool_id > 0) {
                // Check if we already have this pool ID
                bool already_added = false;
                for (size_t k = 0; k < num_pools_to_update; k++) {
                    if (pools_to_update[k] == (int)entry->tile->pool_id) {
                        already_added = true;
                        break;
                    }
                }

                if (!already_added &&
                    num_pools_to_update < MAX_POOLS_TO_UPDATE) {
                    pools_to_update[num_pools_to_update++] =
                      entry->tile->pool_id;
                }
            }
        }
    }

    // Update neighbors for all affected pools
    for (size_t i = 0; i < num_pools_to_update; i++) {
        pool_t *pool = pool_manager_get_pool(manager, pools_to_update[i]);
        if (pool) {
            pool_update_neighbors(pool, board_tiles, geometry_type);
        }
    }
}

void pool_manager_get_neighbor_pools(pool_manager_t *manager, grid_cell_t cell,
                                     grid_type_e geometry_type,
                                     tile_map_t *board_tiles,
                                     uint32_t *out_pool_ids,
                                     size_t *out_count) {
    if (!manager || !board_tiles || !out_pool_ids || !out_count)
        return;

    *out_count = 0;

    int neighbor_count = grid_geometry_get_neighbor_count(geometry_type);
    grid_cell_t neighbor_cells[neighbor_count];
    grid_geometry_get_all_neighbors(geometry_type, cell, neighbor_cells);

    for (int i = 0; i < neighbor_count; i++) {
        tile_map_entry_t *entry = tile_map_find(board_tiles, neighbor_cells[i]);
        if (entry && entry->tile && entry->tile->pool_id > 0) {
            // Check for duplicates
            bool is_duplicate = false;
            for (size_t j = 0; j < *out_count; j++) {
                if (out_pool_ids[j] == entry->tile->pool_id) {
                    is_duplicate = true;
                    break;
                }
            }
            if (!is_duplicate) {
                out_pool_ids[(*out_count)++] = entry->tile->pool_id;
            }
        }
    }
}
