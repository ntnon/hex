#include "../../include/tile/pool.h"
#include "../../include/grid/grid_cell_utils.h"
#include "../../include/grid/grid_system.h"
#include "../../include/third_party/uthash.h"
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
  pool->edges = edge_map_create();
  pool->highest_n = 0;
  pool->accepted_tile_type = TILE_UNDEFINED; // Default to empty type.

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
    center_cell.coord.square.x = total_x / count;
    center_cell.coord.square.y = total_y / count;
    pool->center = center_cell;
  }
}

bool pool_contains_tile(const pool_t *pool, const tile_t *tile_ptr) {
  return tile_map_find(pool->tiles, tile_ptr->cell) != NULL;
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

// Main function: Adds a tile to a pool if it passes validations.
bool pool_add_tile_to_pool(pool_t *pool, const tile_t *tile) {

  if (!pool || !tile)
    return false;

  if (pool_contains_tile(pool, tile)) {
    printf("Tile already exists in pool %d\n", pool->id);
    return false;
  }

  if (!pool_accepts_tile_type(pool, tile->data.type)) {
    printf("Tile type %d not allowed in pool %d\n", tile->data.type, pool->id);
    return false;
  }

  // Add the tile to the pool's internal tile map.
  tile_map_add(pool->tiles, (tile_t *)tile);

  if (pool->accepted_tile_type == TILE_UNDEFINED) {
    // If the pool has no accepted tile type, set it to the tile's type.
    pool->accepted_tile_type = tile->data.type;
  }

  return true;
}

void pool_free(pool_t *pool) {
  tile_map_free(pool->tiles);
  edge_map_free(&pool->edges);
  free(pool);
};

void pool_add_tile(pool_t *pool, const tile_t *tile_ptr) {
  tile_map_add(pool->tiles, (tile_t *)tile_ptr);
}

// UTILITY
int pool_score(const pool_t *pool) {
  // Example: prioritize size, then lower ID as tiebreaker
  // You can adjust weights as needed
  return 100000 * pool->tiles->num_tiles - pool->id;
}
int compare_pools_by_score(const void *a, const void *b) {
  const pool_t *pool_a = *(const pool_t **)a;
  const pool_t *pool_b = *(const pool_t **)b;
  int score_a = pool_score(pool_a);
  int score_b = pool_score(pool_b);
  return score_b - score_a;
}

void pool_update_edges(const grid_t *grid, pool_t *pool) {
  edge_map_entry_t *collected_edges = edge_map_create();

  tile_map_entry_t *tile_entry, *tmp;
  HASH_ITER(hh, pool->tiles->root, tile_entry, tmp) {
    get_cell_edges(grid, tile_entry->cell, &collected_edges);
  }
  edge_map_free(&pool->edges);   // Free old edges if needed
  pool->edges = collected_edges; // Assign the new set
};

int pool_find_tile_friendly_neighbor_count(tile_map_t *tile_map,
                                           const tile_t *tile,
                                           const grid_t *grid) {
  int num_neighbors = grid->vtable->num_neighbors;
  grid_cell_t neighbor_cells[num_neighbors];
  grid->vtable->get_neighbor_cells(tile->cell, neighbor_cells);

  int neighbor_count = 0;
  for (int i = 0; i < num_neighbors; ++i) {
    bool entry = tile_map_contains(tile_map, neighbor_cells[i]);
    if (entry) {
      neighbor_count++;
    }
  }
  return neighbor_count;
}

int pool_find_max_tile_neighbors_in_pool(pool_t *pool, const grid_t *grid) {

  if (!pool->tiles) {
    printf("Pool tiles not found\n");
    return 0;
  }

  int best_score = 0;
  tile_map_entry_t *entry, *tmp;
  HASH_ITER(hh, pool->tiles->root, entry, tmp) {
    int score =
      pool_find_tile_friendly_neighbor_count(pool->tiles, entry->tile, grid);
    if (score > best_score) {
      best_score = score;
    }
  }

  return best_score;
}

void pool_calculate_score(pool_t *pool, const grid_t *grid) {
  pool->highest_n = pool_find_max_tile_neighbors_in_pool(pool, grid);
}

void pool_update(pool_t *pool, const grid_t *grid) {
  pool_calculate_score(pool, grid);
}
