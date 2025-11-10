#include "../../include/tile/pool.h"
#include "../../include/grid/grid_cell_utils.h"
#include "../../include/grid/grid_geometry.h"
#include "../../include/third_party/uthash.h"
#include "grid/grid_types.h"
#include "third_party/kvec.h"
#include "tile/tile_map.h"
#include <complex.h>
#include <stdio.h>

// LIFECYCLE
pool_t *pool_create() {
    pool_t *pool = malloc(sizeof(pool_t));
    if (!pool)
        return NULL; // Always check for allocation failure.

    pool->id = -1;
    // Since we're not given a tile here, we don't set accepted_tile_types.
    // Create the pool's internal tile map container.
    pool->tiles = tile_map_create();
    pool->highest_n = 0;
    pool->accepted_tile_type = TILE_UNDEFINED; // Default to empty type.
    pool->modifier = 1.0f;                     // Initialize modifier to 0

    // Initialize geometric properties
    pool->diameter = 0;
    pool->edge_count = 0;
    pool->compactness_score = 0.0f;

    // Initialize neighbor cells and tiles
    kv_init(pool->neighbor_cells);
    kv_init(pool->neighbor_tiles);

    return pool;
}

void pool_update_center(pool_t *pool) {
    if (!pool || !pool->tiles || pool->tiles->num_tiles == 0)
        return;

    // Find the center tile based on the average position of all tiles in the
    // pool.
    tile_map_entry_t *entry, *tmp;
    grid_cell_t center_cell = {0};
    int total_x = 0, total_y = 0, count = 0;

    HASH_ITER(hh, pool->tiles->root, entry, tmp) {
        total_x += entry->cell.coord.square.x;
        total_y += entry->cell.coord.square.y;
        count++;
    }

    if (count > 0) {
        center_cell.coord.square.y = total_y / count;
        center_cell.coord.square.x = total_x / count;
        pool->center = center_cell;
    }
}

bool pool_contains_tile(const pool_t *pool, const tile_t *tile_ptr) {
    return tile_map_find(pool->tiles, tile_ptr->cell) != NULL;
}

// --- Modifier Functions ---

void pool_set_modifier(pool_t *pool, float modifier) {
    if (pool) {
        pool->modifier = modifier;
    }
}

void pool_add_modifier(pool_t *pool, float modifier_delta) {
    if (pool) {
        pool->modifier += modifier_delta;
    }
}

float pool_get_modifier(const pool_t *pool) {
    return pool ? pool->modifier : 0.0f;
}

// --- Geometric Property Functions ---

void pool_update_geometric_properties(pool_t *pool, grid_type_e geometry_type) {
    if (!pool)
        return;

    size_t old_tile_count = pool->tiles ? pool->tiles->num_tiles : 0;

    pool->diameter = pool_calculate_diameter(pool, geometry_type);
    pool->edge_count = pool_calculate_edge_count(pool, geometry_type);
    pool->compactness_score = pool_calculate_compactness_score(pool);
}

int pool_calculate_diameter(const pool_t *pool, grid_type_e geometry_type) {
    if (!pool || !pool->tiles || pool->tiles->num_tiles < 2) {
        return 0;
    }

    // Extract cells from pool tiles
    grid_cell_t *cells = malloc(pool->tiles->num_tiles * sizeof(grid_cell_t));
    if (!cells)
        return 0;

    tile_map_entry_t *entry, *tmp;
    size_t i = 0;
    HASH_ITER(hh, pool->tiles->root, entry, tmp) {
        cells[i] = entry->tile->cell;
        i++;
    }

    // Use geometry-agnostic calculation
    int diameter = grid_geometry_calculate_diameter(geometry_type, cells,
                                                    pool->tiles->num_tiles);
    free(cells);
    return diameter;
}

grid_cell_t pool_calculate_center(const pool_t *pool,
                                  grid_type_e geometry_type) {
    grid_cell_t invalid_cell = {0};
    if (!pool || !pool->tiles || pool->tiles->num_tiles == 0) {
        return invalid_cell;
    }

    // Extract cells from pool tiles
    grid_cell_t *cells = malloc(pool->tiles->num_tiles * sizeof(grid_cell_t));
    if (!cells)
        return invalid_cell;

    tile_map_entry_t *entry, *tmp;
    size_t i = 0;
    HASH_ITER(hh, pool->tiles->root, entry, tmp) {
        cells[i] = entry->tile->cell;
        i++;
    }

    // Use geometry-agnostic calculation
    grid_cell_t center = grid_geometry_calculate_center(geometry_type, cells,
                                                        pool->tiles->num_tiles);
    free(cells);
    return center;
}

int pool_calculate_edge_count(const pool_t *pool, grid_type_e geometry_type) {
    if (!pool || !pool->tiles || pool->tiles->num_tiles == 0) {
        return 0;
    }

    // Extract cells from pool tiles
    grid_cell_t *cells = malloc(pool->tiles->num_tiles * sizeof(grid_cell_t));
    if (!cells)
        return 0;

    tile_map_entry_t *entry, *tmp;
    size_t i = 0;
    HASH_ITER(hh, pool->tiles->root, entry, tmp) {
        cells[i] = entry->tile->cell;
        i++;
    }

    // Use geometry-agnostic calculation
    int edge_count = grid_geometry_count_external_edges(geometry_type, cells,
                                                        pool->tiles->num_tiles);
    free(cells);
    return edge_count;
}

float pool_calculate_compactness_score(const pool_t *pool) {
    if (!pool || !pool->tiles || pool->tiles->num_tiles == 0) {
        return 0.0f;
    }

    // Get actual tile count
    int actual_tiles = pool->tiles->num_tiles;

    // Extract coordinates from tile map hash table
    grid_cell_t *coordinates = malloc(actual_tiles * sizeof(grid_cell_t));
    if (!coordinates) {
        return 0.0f;
    }

    int index = 0;
    tile_map_entry_t *entry, *tmp;
    HASH_ITER(hh, pool->tiles->root, entry, tmp) {
        coordinates[index++] = entry->cell;
    }

    // Calculate internal edges (shared between tiles in pool)
    int internal_edges = grid_geometry_count_internal_edges(
      GRID_TYPE_HEXAGON, coordinates, actual_tiles);

    free(coordinates);

    // External edges are already calculated and stored in pool
    int external_edges = pool->edge_count;

    // Total edges = internal + external
    int total_edges = internal_edges + external_edges;

    if (total_edges == 0) {
        return 0.0f;
    }

    // Compactness = internal edges / total edges
    // 1.0 = all edges are internal (impossible), 0.0 = all edges are external
    // Higher values = more compact (more shared edges)
    float score = (float)internal_edges / (float)total_edges;

    return score;
}

void pool_print(const pool_t *pool) {
    if (!pool) {
        return;
    }

    printf("=== Pool Properties ===\n");
    printf("ID: %d\n", pool->id);
    printf("Tile count: %d\n", pool->tiles ? pool->tiles->num_tiles : 0);
    printf("Accepted tile type: %d\n", pool->accepted_tile_type);
    printf("Highest N: %d\n", pool->highest_n);
    printf("Modifier: %.2f\n", pool->modifier);
    printf("--- Geometric Properties ---\n");
    printf("Diameter: %d\n", pool->diameter);
    printf("Edge count: %d\n", pool->edge_count);
    printf("Compactness score: %.3f\n", pool->compactness_score);
    printf("Neighbor cell count: %zu\n", pool->neighbor_cells.n);
    printf("Neighbor tile count: %zu\n", pool->neighbor_tiles.n);
    printf("========================\n");
}

// Helper: Returns true if the tile's type is allowed in the pool.

bool pool_accepts_tile_type(const pool_t *pool, tile_type_t type) {
    if (pool->accepted_tile_type == TILE_UNDEFINED)
        return true; // If no specific type is set, allow all types.

    // Check if the tile's type matches the pool's accepted type.
    return pool->accepted_tile_type == type;
}
void pool_remove_tile(const pool_t *pool, const tile_t *tile_ptr) {
    if (!pool || !tile_ptr)
        return;

    if (!pool_contains_tile(pool, tile_ptr)) {
        printf("Tile not found in pool %d\n", pool->id);
        return;
    }

    // Remove the tile from the pool's internal tile map
    tile_map_remove(pool->tiles, tile_ptr->cell);
}

static bool is_cell_in_neighbor_list(const pool_t *pool, grid_cell_t cell,
                                     grid_type_e geometry_type) {
    for (size_t i = 0; i < kv_size(pool->neighbor_cells); i++) {
        if (grid_geometry_cells_equal(geometry_type,
                                      kv_A(pool->neighbor_cells, i), cell)) {
            return true;
        }
    }
    return false;
}

void pool_update_neighbors(pool_t *pool, const tile_map_t *board_tiles,
                           grid_type_e geometry_type) {
    if (!pool || !board_tiles)
        return;

    // --- 1. Update neighbor cells ---
    kv_destroy(pool->neighbor_cells);
    kv_init(pool->neighbor_cells);

    tile_map_entry_t *entry, *tmp;
    HASH_ITER(hh, pool->tiles->root, entry, tmp) {
        int num_neighbors = grid_geometry_get_neighbor_count(geometry_type);
        grid_cell_t neighbors[num_neighbors];
        grid_geometry_get_all_neighbors(geometry_type, entry->tile->cell,
                                        neighbors);

        for (int i = 0; i < num_neighbors; i++) {
            grid_cell_t n = neighbors[i];

            // Skip if already in pool or already added to neighbor_cells
            if (tile_map_contains(pool->tiles, n))
                continue;

            bool duplicate = false;
            for (size_t j = 0; j < kv_size(pool->neighbor_cells); j++) {
                if (grid_geometry_cells_equal(
                      geometry_type, kv_A(pool->neighbor_cells, j), n)) {
                    duplicate = true;
                    break;
                }
            }

            if (!duplicate) {
                kv_push(grid_cell_t, pool->neighbor_cells, n);
            }
        }
    }

    // --- 2. Update neighbor tiles ---
    kv_destroy(pool->neighbor_tiles);
    kv_init(pool->neighbor_tiles);

    for (size_t i = 0; i < kv_size(pool->neighbor_cells); i++) {
        grid_cell_t cell = kv_A(pool->neighbor_cells, i);

        tile_map_entry_t *neighbor_entry = tile_map_find(board_tiles, cell);
        if (neighbor_entry) {
            kv_push(tile_t *, pool->neighbor_tiles, neighbor_entry->tile);
        }
    }

    printf("Pool %d: %zu neighbor cells, %zu neighbor tiles\n", pool->id,
           kv_size(pool->neighbor_cells), kv_size(pool->neighbor_tiles));
}

// Main function: Adds a tile to a pool if it passes validations.
bool pool_add_tile(pool_t *pool, const tile_t *tile, grid_type_e geometry_type,
                   const tile_map_t *board_tiles) {

    if (!pool || !tile)
        return false;

    if (pool_contains_tile(pool, tile)) {
        return false;
    }

    if (!pool_accepts_tile_type(pool, tile->data.type)) {
        return false;
    }

    // Add the tile to the pool's internal tile map.
    tile_map_add(pool->tiles, (tile_t *)tile);

    if (pool->accepted_tile_type == TILE_UNDEFINED) {
        // If the pool has no accepted tile type, set it to the tile's type.
        pool->accepted_tile_type = tile->data.type;
    }

    // Update geometric properties after adding tile
    pool_update_geometric_properties(pool, geometry_type);

    // Update neighbors after adding tile
    pool_update_neighbors(pool, board_tiles, geometry_type);
    return true;
}

void pool_free(pool_t *pool) {
    tile_map_free(pool->tiles);
    kv_destroy(pool->neighbor_cells);
    kv_destroy(pool->neighbor_tiles);
    free(pool);
};

// UTILITY
int pool_compatibility_score(const pool_t *pool) {
    // Example: prioritize size, then lower ID as tiebreaker
    // You can adjust weights as needed
    return 100000 * pool->tiles->num_tiles - pool->id;
}
int compare_pools_by_score(const void *a, const void *b) {
    const pool_t *pool_a = *(const pool_t **)a;
    const pool_t *pool_b = *(const pool_t **)b;
    int score_a = pool_compatibility_score(pool_a);
    int score_b = pool_compatibility_score(pool_b);
    return score_b - score_a;
}

int pool_find_tile_friendly_neighbor_count(tile_map_t *tile_map,
                                           const tile_t *tile,
                                           grid_type_e grid_type) {
    int num_neighbors = grid_geometry_get_neighbor_count(grid_type);
    grid_cell_t neighbor_cells[num_neighbors];
    grid_geometry_get_all_neighbors(grid_type, tile->cell, neighbor_cells);

    int neighbor_count = 0;
    for (int i = 0; i < num_neighbors; ++i) {
        bool entry = tile_map_contains(tile_map, neighbor_cells[i]);
        if (entry) {
            neighbor_count++;
        }
    }
    return neighbor_count;
}

int pool_find_max_tile_neighbors_in_pool(pool_t *pool, grid_type_e grid_type) {

    if (!pool->tiles) {
        printf("Pool tiles not found\n");
        return 0;
    }

    int best_score = 0;
    tile_map_entry_t *entry, *tmp;
    HASH_ITER(hh, pool->tiles->root, entry, tmp) {
        int score = pool_find_tile_friendly_neighbor_count(
          pool->tiles, entry->tile, grid_type);
        if (score > best_score) {
            best_score = score;
        }
    }

    return best_score;
}

void pool_calculate_score(pool_t *pool, grid_type_e grid_type) {
    pool->highest_n = pool_find_max_tile_neighbors_in_pool(pool, grid_type);
}

void pool_update(pool_t *pool, grid_type_e grid_type) {
    pool_calculate_score(pool, grid_type);
}

static void add_tile_value(tile_t *tile, void *user_data) {
    int *total_value = (int *)user_data;
    *total_value += tile->data.value;
}

int pool_tile_score(const pool_t *pool) {
    if (!pool || !pool->tiles) {
        return 0;
    }

    // int total_value = 0;
    // tile_map_foreach_tile(pool->tiles, add_tile_value, &total_value);
    // return total_value;
    return pool->tiles->num_tiles;
}
