#include "../../include/grid/grid_system.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Optimized chunk size for hex grids (8-16 works well)
#define DEFAULT_CHUNK_SIZE 12
#define INITIAL_HASH_TABLE_SIZE 64
#define COORD_POOL_INITIAL_SIZE 32

// Hash function for chunk IDs
static inline size_t hash_chunk_id(chunk_id_t chunk_id, size_t table_size) {
  // Simple hash combining x and y coordinates
  size_t hash = ((size_t)chunk_id.chunk_x * 73856093) ^
                ((size_t)chunk_id.chunk_y * 19349663);
  return hash & (table_size - 1); // table_size must be power of 2
}

// Helper function for chunk ID comparison
static inline bool chunk_id_equals(chunk_id_t a, chunk_id_t b) {
  return a.chunk_x == b.chunk_x && a.chunk_y == b.chunk_y;
}

// Helper function to check if chunk ID is invalid
static inline bool is_invalid_chunk_id(chunk_id_t chunk_id) {
  return chunk_id.chunk_x == INVALID_CHUNK_ID.chunk_x &&
         chunk_id.chunk_y == INVALID_CHUNK_ID.chunk_y;
}

// Get or allocate coordinate array from pool
static grid_cell_t *pool_get_coords(chunk_system_t *system,
                                    size_t needed_size) {
  if (!system)
    return NULL;

  // Look for available entry with sufficient capacity
  coord_pool_entry_t *entry = system->coord_pool;
  while (entry) {
    if (!entry->in_use && entry->capacity >= needed_size) {
      entry->in_use = true;
      return entry->coords;
    }
    entry = entry->next;
  }

  // No suitable entry found, create new one
  coord_pool_entry_t *new_entry = malloc(sizeof(coord_pool_entry_t));
  if (!new_entry)
    return NULL;

  size_t alloc_size = needed_size > COORD_POOL_INITIAL_SIZE
                        ? needed_size
                        : COORD_POOL_INITIAL_SIZE;
  new_entry->coords = malloc(alloc_size * sizeof(grid_cell_t));
  if (!new_entry->coords) {
    free(new_entry);
    return NULL;
  }

  new_entry->capacity = alloc_size;
  new_entry->in_use = true;
  new_entry->next = system->coord_pool;
  system->coord_pool = new_entry;

  return new_entry->coords;
}

// Return coordinate array to pool
static void pool_return_coords(chunk_system_t *system, grid_cell_t *coords) {
  if (!system || !coords)
    return;

  coord_pool_entry_t *entry = system->coord_pool;
  while (entry) {
    if (entry->coords == coords && entry->in_use) {
      entry->in_use = false;
      return;
    }
    entry = entry->next;
  }
}

// Find chunk in hash table
static grid_chunk_t *find_chunk(chunk_system_t *system, chunk_id_t chunk_id) {
  if (!system || !system->hash_table)
    return NULL;

  size_t bucket = hash_chunk_id(chunk_id, system->hash_table_size);
  grid_chunk_t *chunk = system->hash_table[bucket];

  while (chunk) {
    if (chunk_id_equals(chunk->id, chunk_id)) {
      return chunk;
    }
    chunk = chunk->next;
  }

  return NULL;
}

// Create and insert new chunk
static grid_chunk_t *create_chunk(chunk_system_t *system, chunk_id_t chunk_id) {
  if (!system || !system->hash_table)
    return NULL;

  grid_chunk_t *chunk = malloc(sizeof(grid_chunk_t));
  if (!chunk)
    return NULL;

  chunk->id = chunk_id;
  chunk->dirty = true;
  chunk->render_data = NULL;

  // Insert into hash table
  size_t bucket = hash_chunk_id(chunk_id, system->hash_table_size);
  chunk->next = system->hash_table[bucket];
  system->hash_table[bucket] = chunk;
  system->num_chunks++;

  return chunk;
}

// Find or create chunk
static grid_chunk_t *find_or_create_chunk(grid_t *grid, chunk_id_t chunk_id) {
  if (!grid)
    return NULL;

  chunk_system_t *system = &grid->chunk_system;

  grid_chunk_t *chunk = find_chunk(system, chunk_id);
  if (chunk) {
    return chunk;
  }

  return create_chunk(system, chunk_id);
}

// Resize hash table if needed
static void maybe_resize_hash_table(chunk_system_t *system) {
  if (!system || !system->hash_table)
    return;

  // Resize when load factor > 0.75
  if (system->num_chunks <= system->hash_table_size * 3 / 4)
    return;

  size_t old_size = system->hash_table_size;
  size_t new_size = old_size * 2;
  grid_chunk_t **old_table = system->hash_table;

  // Allocate new table
  grid_chunk_t **new_table = calloc(new_size, sizeof(grid_chunk_t *));
  if (!new_table)
    return; // Keep old table on failure

  system->hash_table = new_table;
  system->hash_table_size = new_size;

  // Rehash all chunks
  for (size_t i = 0; i < old_size; i++) {
    grid_chunk_t *chunk = old_table[i];
    while (chunk) {
      grid_chunk_t *next = chunk->next;

      size_t new_bucket = hash_chunk_id(chunk->id, new_size);
      chunk->next = new_table[new_bucket];
      new_table[new_bucket] = chunk;

      chunk = next;
    }
  }

  free(old_table);
}

void grid_init_chunks(grid_t *grid, int chunk_size) {
  if (!grid)
    return;

  if (chunk_size <= 0) {
    chunk_size = DEFAULT_CHUNK_SIZE;
  }

  chunk_system_t *system = &grid->chunk_system;

  // Clean up existing system
  if (system->hash_table) {
    for (size_t i = 0; i < system->hash_table_size; i++) {
      grid_chunk_t *chunk = system->hash_table[i];
      while (chunk) {
        grid_chunk_t *next = chunk->next;
        free(chunk);
        chunk = next;
      }
    }
    free(system->hash_table);
  }

  // Clean up coordinate pool
  coord_pool_entry_t *entry = system->coord_pool;
  while (entry) {
    coord_pool_entry_t *next = entry->next;
    free(entry->coords);
    free(entry);
    entry = next;
  }

  // Initialize new system
  system->chunk_size = chunk_size;
  system->hash_table_size = INITIAL_HASH_TABLE_SIZE;
  system->hash_table = calloc(system->hash_table_size, sizeof(grid_chunk_t *));
  system->num_chunks = 0;
  system->system_dirty = false;
  system->coord_pool = NULL;

  if (!system->hash_table) {
    printf("ERROR: Failed to allocate chunk hash table\n");
    return;
  }

  printf("Initialized optimized chunk system with size %d\n", chunk_size);
}

void grid_update_chunks(grid_t *grid) {
  if (!grid)
    return;

  chunk_system_t *system = &grid->chunk_system;

  // Only resize hash table if needed
  maybe_resize_hash_table(system);

  system->system_dirty = false;
}

chunk_id_t grid_get_chunk_id(const grid_t *grid, grid_cell_t cell) {
  if (!grid || !grid->vtable->get_chunk_id) {
    return (chunk_id_t){0, 0};
  }

  if (!grid->vtable->is_valid_cell(grid, cell)) {
    return INVALID_CHUNK_ID;
  }

  return grid->vtable->get_chunk_id(grid, cell, grid->chunk_system.chunk_size);
}

void grid_get_chunk_coordinates(const grid_t *grid, chunk_id_t chunk_id,
                                grid_cell_t **out_coords, size_t *out_count) {
  if (!grid || !grid->vtable->get_chunk_coordinates || !out_coords ||
      !out_count) {
    if (out_coords)
      *out_coords = NULL;
    if (out_count)
      *out_count = 0;
    return;
  }

  // Generate coordinates on-demand - no storage in chunk
  grid->vtable->get_chunk_coordinates(
    grid, chunk_id, grid->chunk_system.chunk_size, out_coords, out_count);
}

grid_cell_t *grid_get_chunk_coordinates_pooled(grid_t *grid,
                                               chunk_id_t chunk_id,
                                               size_t *out_count) {
  if (!grid || !out_count) {
    if (out_count)
      *out_count = 0;
    return NULL;
  }

  // First get count
  grid_cell_t *temp_coords = NULL;
  size_t count = 0;
  grid_get_chunk_coordinates(grid, chunk_id, &temp_coords, &count);

  if (count == 0) {
    *out_count = 0;
    return NULL;
  }

  // Get pooled memory
  grid_cell_t *pooled_coords = pool_get_coords(&grid->chunk_system, count);
  if (!pooled_coords) {
    *out_count = 0;
    free(temp_coords);
    return NULL;
  }

  // Copy coordinates
  memcpy(pooled_coords, temp_coords, count * sizeof(grid_cell_t));
  free(temp_coords);

  *out_count = count;
  return pooled_coords;
}

void grid_return_chunk_coordinates_pooled(grid_t *grid, grid_cell_t *coords) {
  if (!grid || !coords)
    return;
  pool_return_coords(&grid->chunk_system, coords);
}

void grid_mark_chunk_dirty(grid_t *grid, chunk_id_t chunk_id) {
  if (!grid)
    return;

  if (is_invalid_chunk_id(chunk_id)) {
    return;
  }

  grid_chunk_t *chunk = find_or_create_chunk(grid, chunk_id);
  if (chunk) {
    chunk->dirty = true;
  }
}

grid_chunk_t **grid_get_all_chunks(const grid_t *grid, size_t *out_count) {
  if (!grid || !out_count) {
    if (out_count)
      *out_count = 0;
    return NULL;
  }

  const chunk_system_t *system = &grid->chunk_system;

  if (system->num_chunks == 0) {
    *out_count = 0;
    return NULL;
  }

  grid_chunk_t **chunk_ptrs =
    malloc(system->num_chunks * sizeof(grid_chunk_t *));
  if (!chunk_ptrs) {
    *out_count = 0;
    return NULL;
  }

  size_t index = 0;
  for (size_t i = 0; i < system->hash_table_size; i++) {
    grid_chunk_t *chunk = system->hash_table[i];
    while (chunk && index < system->num_chunks) {
      chunk_ptrs[index++] = chunk;
      chunk = chunk->next;
    }
  }

  *out_count = index;
  return chunk_ptrs;
}

void grid_cleanup_chunks(grid_t *grid) {
  if (!grid)
    return;

  chunk_system_t *system = &grid->chunk_system;

  // Clean up hash table
  if (system->hash_table) {
    for (size_t i = 0; i < system->hash_table_size; i++) {
      grid_chunk_t *chunk = system->hash_table[i];
      while (chunk) {
        grid_chunk_t *next = chunk->next;
        free(chunk);
        chunk = next;
      }
    }
    free(system->hash_table);
  }

  // Clean up coordinate pool
  coord_pool_entry_t *entry = system->coord_pool;
  while (entry) {
    coord_pool_entry_t *next = entry->next;
    free(entry->coords);
    free(entry);
    entry = next;
  }

  // Reset system
  system->hash_table = NULL;
  system->hash_table_size = 0;
  system->num_chunks = 0;
  system->chunk_size = 0;
  system->system_dirty = true;
  system->coord_pool = NULL;
}

void grid_test_chunk_performance(grid_t *grid) {
  if (!grid) {
    printf("ERROR: Cannot test chunks on null grid\n");
    return;
  }

  printf("\n=== Chunk System Performance Test ===\n");

  // Initialize chunks if not already done
  if (!grid->chunk_system.hash_table) {
    grid_init_chunks(grid, 12);
  }

  chunk_system_t *system = &grid->chunk_system;
  printf("Chunk size: %d\n", system->chunk_size);
  printf("Hash table size: %zu\n", system->hash_table_size);

  // Test chunk creation and lookup performance
  clock_t start = clock();
  size_t test_chunks_created = 0;

  // Create chunks for a reasonable test area
  for (int q = -30; q <= 30; q += 6) {
    for (int r = -30; r <= 30; r += 6) {
      int s = -q - r;
      grid_cell_t test_cell = {.type = GRID_TYPE_HEXAGON,
                               .coord.hex = {q, r, s}};

      if (grid->vtable->is_valid_cell(grid, test_cell)) {
        chunk_id_t chunk_id = grid_get_chunk_id(grid, test_cell);
        if (!is_invalid_chunk_id(chunk_id)) {
          grid_chunk_t *chunk = find_or_create_chunk(grid, chunk_id);
          if (chunk) {
            test_chunks_created++;
          }
        }
      }
    }
  }

  clock_t end = clock();
  double creation_time = ((double)(end - start)) / CLOCKS_PER_SEC;

  printf("Created/found %zu chunks in %.4f seconds\n", test_chunks_created,
         creation_time);
  printf("Average time per chunk: %.6f seconds\n",
         creation_time / test_chunks_created);

  // Test coordinate generation performance
  start = clock();
  size_t total_coords = 0;
  size_t coord_tests = 0;

  for (size_t i = 0; i < system->hash_table_size; i++) {
    grid_chunk_t *chunk = system->hash_table[i];
    while (chunk && coord_tests < 50) { // Limit tests to prevent long runtime
      grid_cell_t *coords;
      size_t count;
      grid_get_chunk_coordinates(grid, chunk->id, &coords, &count);

      if (coords) {
        total_coords += count;
        free(coords);
        coord_tests++;
      }
      chunk = chunk->next;
    }
  }

  end = clock();
  double coord_time = ((double)(end - start)) / CLOCKS_PER_SEC;

  printf("Generated %zu coordinates across %zu chunks in %.4f seconds\n",
         total_coords, coord_tests, coord_time);

  if (coord_tests > 0) {
    printf("Average coordinates per chunk: %.1f\n",
           (double)total_coords / coord_tests);
    printf("Average time per coordinate: %.8f seconds\n",
           coord_time / total_coords);
  }

  // Test pooled coordinate performance
  start = clock();
  size_t pooled_tests = 0;

  for (size_t i = 0; i < system->hash_table_size && pooled_tests < 20; i++) {
    grid_chunk_t *chunk = system->hash_table[i];
    if (chunk) {
      size_t count;
      grid_cell_t *pooled_coords =
        grid_get_chunk_coordinates_pooled(grid, chunk->id, &count);

      if (pooled_coords) {
        grid_return_chunk_coordinates_pooled(grid, pooled_coords);
        pooled_tests++;
      }
    }
  }

  end = clock();
  double pool_time = ((double)(end - start)) / CLOCKS_PER_SEC;

  printf("Pooled coordinate tests: %zu in %.4f seconds\n", pooled_tests,
         pool_time);

  // Display memory usage info
  size_t pool_entries = 0;
  size_t pool_memory = 0;
  coord_pool_entry_t *entry = system->coord_pool;
  while (entry) {
    pool_entries++;
    pool_memory += entry->capacity * sizeof(grid_cell_t);
    entry = entry->next;
  }

  printf("Memory pool: %zu entries, ~%zu bytes allocated\n", pool_entries,
         pool_memory);
  printf("Active chunks: %zu (load factor: %.2f)\n", system->num_chunks,
         (double)system->num_chunks / system->hash_table_size);

  printf("=== Test Complete ===\n\n");
}
