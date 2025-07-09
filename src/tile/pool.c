#include "../../include/tile/pool.h"
#include "../../include/grid/grid_cell_utils.h"
#include "../../include/grid/grid_system.h"
#include "../../include/third_party/uthash.h"
#include <stdio.h>

// LIFECYCLE
pool_t *
pool_create ()
{
  pool_t *pool = malloc (sizeof (pool_t));
  if (!pool)
    return NULL; // Always check for allocation failure.

  pool->id = -1;
  // Since we're not given a tile here, we don't set accepted_tile_types.
  pool->num_accepted_tile_types = 0;
  // Create the pool's internal tile map container.
  pool->tiles = tile_map_create ();
  pool->edges = edge_map_create ();
  pool->is_mixed = false;
  pool->highest_n = 0;

  return pool;
}

bool
pool_contains_tile (const pool_t *pool, const tile_t *tile_ptr)
{
  return tile_map_find (pool->tiles, tile_ptr->cell) != NULL;
}

// Helper: Returns true if the tile's type is allowed in the pool.
static bool
pool_tile_type_allowed (pool_t *pool, const tile_t *tile)
{
  // If no accepted type has been recorded, allow it.
  if (pool->num_accepted_tile_types == 0)
    return true;

  for (size_t i = 0; i < pool->num_accepted_tile_types; i++)
    {
      if (pool->accepted_tile_types[i] == tile->type)
        return true;
    }
  return false;
}

// Helper: Updates the pool's accepted tile types with the new tile's type.
static void
pool_update_accepted_tile_types (pool_t *pool, const tile_t *tile)
{
  if (pool->num_accepted_tile_types == 0)
    {
      pool->accepted_tile_types[0] = tile->type;
      pool->num_accepted_tile_types = 1;
    }
  else
    {
      bool found = false;
      for (size_t i = 0; i < pool->num_accepted_tile_types; i++)
        {
          if (pool->accepted_tile_types[i] == tile->type)
            {
              found = true;
              break;
            }
        }
      if (!found)
        {
          if (pool->num_accepted_tile_types < MAX_ACCEPTED_TILE_TYPES)
            {
              pool->accepted_tile_types[pool->num_accepted_tile_types++]
                  = tile->type;
            }
          else
            {
              printf ("Error: Maximum number of accepted tile types reached "
                      "in pool %d.\n",
                      pool->id);
            }
          pool->is_mixed = true;
        }
    }
}

// Main function: Adds a tile to a pool if it passes validations.
bool
pool_add_tile_to_pool (pool_t *pool, const tile_t *tile)
{

  if (!pool || !tile)
    return false;

  if (pool_contains_tile (pool, tile))
    {
      printf ("Tile already exists in pool %d\n", pool->id);
      return false;
    }

  if (pool->num_accepted_tile_types != 0
      && !pool_tile_type_allowed (pool, tile))
    {
      printf ("Tile type %d not allowed in pool %d\n", tile->type, pool->id);
      return false;
    }

  // Add the tile to the pool's internal tile map.
  tile_map_add (pool->tiles, (tile_t *)tile);

  // Update the accepted tile types in the pool.
  pool_update_accepted_tile_types (pool, tile);

  return true;
}

void
pool_free (pool_t *pool)
{
  tile_map_free (pool->tiles);
  edge_map_free (&pool->edges);
  free (pool);
};

void
pool_add_tile (pool_t *pool, const tile_t *tile_ptr)
{
  tile_map_add (pool->tiles, (tile_t *)tile_ptr);
}

// UTILITY
int
pool_score (const pool_t *pool)
{
  // Example: prioritize size, then lower ID as tiebreaker
  // You can adjust weights as needed
  return 100000 * pool->tiles->num_tiles - pool->id;
}
int
compare_pools_by_score (const void *a, const void *b)
{
  const pool_t *pool_a = *(const pool_t **)a;
  const pool_t *pool_b = *(const pool_t **)b;
  int score_a = pool_score (pool_a);
  int score_b = pool_score (pool_b);
  return score_b - score_a;
}

bool
pool_accepts_tile_type (const pool_t *pool, tile_type_t type)
{
  for (size_t i = 0; i < pool->num_accepted_tile_types; ++i)
    {
      if (pool->accepted_tile_types[i] == type)
        return true;
    }
  return false;
}
// UPDATE POOL STATE

void
pool_update_edges (const grid_t *grid, pool_t *pool)
{
  edge_map_entry_t *collected_edges = edge_map_create ();

  tile_map_entry_t *tile_entry, *tmp;
  HASH_ITER (hh, pool->tiles->root, tile_entry, tmp)
  {
    get_cell_edges (grid, tile_entry->cell, &collected_edges);
  }
  edge_map_free (&pool->edges);  // Free old edges if needed
  pool->edges = collected_edges; // Assign the new set
};

void
pool_add_accepted_tile_type (pool_t *pool, tile_type_t type)
{
  // Check for duplicate
  for (size_t i = 0; i < pool->num_accepted_tile_types; ++i)
    {
      if (pool->accepted_tile_types[i] == type)
        return; // Already present, do not add
    }
  // Add if not present and within bounds
  if (pool->num_accepted_tile_types < MAX_ACCEPTED_TILE_TYPES)
    {
      pool->accepted_tile_types[pool->num_accepted_tile_types++] = type;
    }
  else
    {
      // Optionally handle error: too many types
      printf ("Error: Maximum number of accepted tile types reached.\n");
    }
  pool->is_mixed = true;
}

int
pool_find_tile_friendly_neighbor_count (tile_map_t *tile_map,
                                        const tile_t *tile, const grid_t *grid)
{
  int num_neighbors = grid->vtable->num_neighbors;
  grid_cell_t neighbor_cells[num_neighbors];
  grid->vtable->get_neighbor_cells (tile->cell, neighbor_cells);

  char key_buffer[32];
  grid_cell_to_string (&tile->cell, key_buffer, sizeof (key_buffer));

  int neighbor_count = 0;
  for (int i = 0; i < num_neighbors; ++i)
    {
      grid_cell_to_string (&neighbor_cells[i], key_buffer,
                           sizeof (key_buffer));

      bool entry = tile_map_contains (tile_map, neighbor_cells[i]);
      if (entry)
        {
          neighbor_count++;
          grid_cell_to_string (&neighbor_cells[i], key_buffer,
                               sizeof (key_buffer));
        }
      else
        {
          grid_cell_to_string (&neighbor_cells[i], key_buffer,
                               sizeof (key_buffer));
        }
    }
  return neighbor_count;
}

int
pool_find_max_tile_neighbors_in_pool (pool_t *pool, const grid_t *grid)
{

  if (!pool->tiles)
    {
      printf ("Pool tiles not found\n");
      return 0;
    }

  int best_score = 0;
  tile_map_entry_t *entry, *tmp;
  HASH_ITER (hh, pool->tiles->root, entry, tmp)
  {
    int score = pool_find_tile_friendly_neighbor_count (pool->tiles,
                                                        entry->tile, grid);
    if (score > best_score)
      {
        best_score = score;
      }
  }

  return best_score;
}

void
pool_calculate (pool_t *pool, const grid_t *grid)
{
  pool->highest_n = pool_find_max_tile_neighbors_in_pool (pool, grid);
}

void
pool_update (pool_t *pool, const grid_t *grid)
{
  pool_calculate (pool, grid);
}
