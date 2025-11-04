#include "../../include/tile/tile_map.h"

#include "../../include/grid/grid_cell_utils.h"
#include "../../include/grid/grid_geometry.h"
#include <stdio.h>
#include <stdlib.h>

tile_map_t *tile_map_create(void) {
  tile_map_t *map = malloc(sizeof(tile_map_t));
  if (!map) {
    fprintf(stderr, "Out of memory!\n");
    return NULL;
  }
  map->root = NULL;
  map->num_tiles = 0;
  return map;
}

bool tile_map_contains(const tile_map_t *map, grid_cell_t cell) {
  return (tile_map_find((tile_map_t *)map, cell) != NULL);
}

int tile_map_size(const tile_map_t *map) { return map->num_tiles; }

void tile_map_free(tile_map_t *map) {
  if (!map)
    return;
  tile_map_entry_t *current_entry, *tmp_entry;
  HASH_ITER(hh, map->root, current_entry, tmp_entry) {
    HASH_DEL(map->root, current_entry);
    free(current_entry);
  }
  map->num_tiles = 0;
  free(map);
}

tile_map_entry_t *tile_map_find(const tile_map_t *map, grid_cell_t cell) {
  if (!map)
    return NULL;
  tile_map_entry_t *entry;
  for (entry = map->root; entry != NULL; entry = entry->hh.next) {
    if (grid_cells_equal(&entry->cell, &cell))
      return entry;
  }
  return NULL;
}

void tile_map_remove(tile_map_t *map, grid_cell_t cell) {
  if (!map)
    return;
  tile_map_entry_t *entry = tile_map_find((const tile_map_t *)map, cell);
  if (entry) {
    HASH_DEL(map->root, entry);
    map->num_tiles--;
    free(entry);
  }
}

void tile_map_foreach_tile(tile_map_t *map, void (*fn)(tile_t *, void *),
                           void *user_data) {
  if (!map)
    return;
  tile_map_entry_t *entry, *tmp;
  HASH_ITER(hh, map->root, entry, tmp) { fn(entry->tile, user_data); }
}
void tile_map_add(tile_map_t *map, tile_t *tile) {
  if (!map || !tile)
    return;
  tile_map_entry_t *existing_entry =
    tile_map_find((const tile_map_t *)map, tile->cell);
  if (existing_entry) {
    // Update the tile pointer in the existing entry
    existing_entry->tile = tile;
  } else {
    tile_map_entry_t *entry = malloc(sizeof(tile_map_entry_t));
    if (!entry) {
      fprintf(stderr, "Out of memory!\n");
      return;
    }
    entry->cell = tile->cell;
    entry->tile = tile;
    HASH_ADD(hh, map->root, cell, sizeof(entry->cell), entry);
    map->num_tiles++;
  }
}

void tile_map_add_unchecked(tile_map_t *map, tile_t *tile) {
  if (!map || !tile)
    return;

  tile_map_entry_t *entry = malloc(sizeof(tile_map_entry_t));
  if (!entry) {
    fprintf(stderr, "Out of memory!\n");
    return;
  }
  entry->cell = tile->cell;
  entry->tile = tile;
  HASH_ADD(hh, map->root, cell, sizeof(entry->cell), entry);
  map->num_tiles++;
}

bool tile_map_apply_offset(tile_map_t *tile_map, grid_cell_t offset) {
  if (!tile_map)
    return false;

  if (tile_map->num_tiles == 0)
    return true; // Nothing to offset

  // Collect current tiles and their coordinates
  tile_t **tiles = malloc(tile_map->num_tiles * sizeof(tile_t *));
  grid_cell_t *old_coords = malloc(tile_map->num_tiles * sizeof(grid_cell_t));
  grid_cell_t *new_coords = malloc(tile_map->num_tiles * sizeof(grid_cell_t));

  if (!tiles || !old_coords || !new_coords) {
    free(tiles);
    free(old_coords);
    free(new_coords);
    return false;
  }

  // Extract tiles and coordinates
  int count = 0;
  tile_map_entry_t *entry, *tmp;
  HASH_ITER(hh, tile_map->root, entry, tmp) {
    tiles[count] = entry->tile;
    old_coords[count] = entry->tile->cell;

    // Calculate new coordinate with offset using grid geometry functions
    // Assume hex for now - this should be parameterized
    grid_cell_t new_coord =
      grid_geometry_apply_offset(GRID_TYPE_HEXAGON, entry->tile->cell, offset);
    if (new_coord.type == GRID_TYPE_UNKNOWN) {
      free(tiles);
      free(old_coords);
      free(new_coords);
      return false;
    }
    new_coords[count] = new_coord;

    count++;
  }

  // Check for conflicts between new positions
  for (int i = 0; i < count; i++) {
    for (int j = i + 1; j < count; j++) {
      if (grid_geometry_cells_equal(GRID_TYPE_HEXAGON, new_coords[i],
                                    new_coords[j])) {
        free(tiles);
        free(old_coords);
        free(new_coords);
        return false;
      }
    }
  }

  // Remove all tiles from their current positions
  for (int i = 0; i < count; i++) {
    tile_map_remove(tile_map, old_coords[i]);
  }

  // Update tile positions and re-add them
  for (int i = 0; i < count; i++) {
    tiles[i]->cell = new_coords[i];
    tile_map_add(tile_map, tiles[i]);
  }

  free(tiles);
  free(old_coords);
  free(new_coords);
  return true;
}

bool tile_map_rotate(tile_map_t *tile_map, grid_cell_t center,
                     int rotation_steps) {
  if (!tile_map)
    return false;

  if (tile_map->num_tiles == 0)
    return true; // Nothing to rotate

  // Collect current tile coordinates
  grid_cell_t *current_coords =
    malloc(tile_map->num_tiles * sizeof(grid_cell_t));
  tile_t **tiles = malloc(tile_map->num_tiles * sizeof(tile_t *));

  if (!current_coords || !tiles) {
    free(current_coords);
    free(tiles);
    return false;
  }

  // Extract coordinates and tiles
  int count = 0;
  tile_map_entry_t *entry, *tmp;
  HASH_ITER(hh, tile_map->root, entry, tmp) {
    current_coords[count] = entry->tile->cell;
    tiles[count] = entry->tile;
    count++;
  }

  // Rotate each coordinate individually
  grid_cell_t *rotated_coords = malloc(count * sizeof(grid_cell_t));
  if (!rotated_coords) {
    free(current_coords);
    free(tiles);
    return false;
  }

  // Rotate each tile's position around the center
  for (int i = 0; i < count; i++) {
    // Calculate offset from center
    grid_cell_t offset = grid_geometry_calculate_offset(
      GRID_TYPE_HEXAGON, center, current_coords[i]);
    // Rotate the offset
    grid_cell_t rotated_offset;
    if (!grid_geometry_rotate_cell(GRID_TYPE_HEXAGON, offset, rotation_steps,
                                   &rotated_offset)) {
      free(current_coords);
      free(tiles);
      free(rotated_coords);
      return false;
    }
    // Apply rotated offset back to center
    rotated_coords[i] =
      grid_geometry_apply_offset(GRID_TYPE_HEXAGON, center, rotated_offset);
  }

  // Check for conflicts between rotated positions
  for (int i = 0; i < count; i++) {
    for (int j = i + 1; j < count; j++) {
      if (grid_geometry_cells_equal(GRID_TYPE_HEXAGON, rotated_coords[i],
                                    rotated_coords[j])) {
        free(current_coords);
        free(tiles);
        free(rotated_coords);
        return false;
      }
    }
  }

  // Remove all tiles from their current positions
  for (int i = 0; i < count; i++) {
    tile_map_remove(tile_map, current_coords[i]);
  }

  // Update tile positions and re-add them
  for (int i = 0; i < count; i++) {
    tiles[i]->cell = rotated_coords[i];
    tile_map_add(tile_map, tiles[i]);
  }

  free(current_coords);
  free(tiles);
  free(rotated_coords);
  return true;
}

bool tile_map_merge(tile_map_t *dest, const tile_map_t *source) {
  if (!dest || !source)
    return false;

  if (source->num_tiles == 0)
    return true; // Nothing to merge

  // Check for conflicts with existing tiles in destination
  tile_map_entry_t *entry, *tmp;
  HASH_ITER(hh, source->root, entry, tmp) {
    if (tile_map_contains(dest, entry->tile->cell)) {
      return false; // Conflict found
    }
  }

  // Create copies of source tiles and add to destination
  HASH_ITER(hh, source->root, entry, tmp) {
    // Create a copy of the tile
    tile_t *tile_copy = malloc(sizeof(tile_t));
    if (!tile_copy) {
      return false;
    }

    *tile_copy = *entry->tile; // Copy tile data including position
    tile_map_add(dest, tile_copy);
  }

  return true;
}

tile_map_t *tile_map_clone(const tile_map_t *source) {
  if (!source)
    return NULL;

  tile_map_t *clone = tile_map_create();
  if (!clone)
    return NULL;

  if (source->num_tiles == 0)
    return clone; // Empty clone

  // Iterate through source tiles and create copies
  tile_map_entry_t *entry, *tmp;
  HASH_ITER(hh, source->root, entry, tmp) {
    // Create a copy of the tile
    tile_t *tile_copy = malloc(sizeof(tile_t));
    if (!tile_copy) {
      tile_map_free(clone);
      return NULL;
    }

    *tile_copy = *entry->tile; // Copy tile data including position
    tile_map_add(clone, tile_copy);
  }

  return clone;
}

bool tile_map_find_overlaps(const tile_map_t *map1, const tile_map_t *map2,
                            grid_cell_t **out_overlaps, size_t *out_count) {
  if (!map1 || !map2 || !out_overlaps || !out_count) {
    if (out_overlaps)
      *out_overlaps = NULL;
    if (out_count)
      *out_count = 0;
    return false;
  }

  if (map1->num_tiles == 0 || map2->num_tiles == 0) {
    *out_overlaps = NULL;
    *out_count = 0;
    return true; // No overlaps possible
  }

  // Use the smaller map for iteration to optimize performance
  const tile_map_t *smaller_map =
    (map1->num_tiles <= map2->num_tiles) ? map1 : map2;
  const tile_map_t *larger_map =
    (map1->num_tiles <= map2->num_tiles) ? map2 : map1;

  // Allocate array for potential overlaps (max size is smaller map's tile
  // count)
  grid_cell_t *overlaps = malloc(smaller_map->num_tiles * sizeof(grid_cell_t));
  if (!overlaps) {
    *out_overlaps = NULL;
    *out_count = 0;
    return false;
  }

  size_t overlap_count = 0;

  // Iterate through smaller map and check if coordinates exist in larger map
  tile_map_entry_t *entry, *tmp;
  HASH_ITER(hh, smaller_map->root, entry, tmp) {
    if (tile_map_contains(larger_map, entry->tile->cell)) {
      overlaps[overlap_count] = entry->tile->cell;
      overlap_count++;
    }
  }

  if (overlap_count == 0) {
    free(overlaps);
    *out_overlaps = NULL;
    *out_count = 0;
    return true;
  }

  // Resize array to exact size needed
  if (overlap_count < smaller_map->num_tiles) {
    grid_cell_t *resized =
      realloc(overlaps, overlap_count * sizeof(grid_cell_t));
    if (resized) {
      overlaps = resized;
    }
  }

  *out_overlaps = overlaps;
  *out_count = overlap_count;
  return true;
}

bool tile_map_find_merge_conflicts(const tile_map_t *source,
                                   const tile_map_t *dest, grid_cell_t offset,
                                   grid_cell_t **out_conflicts,
                                   size_t *out_count) {
  if (!source || !dest || !out_conflicts || !out_count) {
    if (out_conflicts)
      *out_conflicts = NULL;
    if (out_count)
      *out_count = 0;
    return false;
  }

  if (source->num_tiles == 0) {
    *out_conflicts = NULL;
    *out_count = 0;
    return true; // No conflicts possible
  }

  // Allocate array for potential conflicts (max size is source tile count)
  grid_cell_t *conflicts = malloc(source->num_tiles * sizeof(grid_cell_t));
  if (!conflicts) {
    *out_conflicts = NULL;
    *out_count = 0;
    return false;
  }

  size_t conflict_count = 0;

  // Check each source tile position after applying offset
  tile_map_entry_t *entry, *tmp;
  HASH_ITER(hh, source->root, entry, tmp) {
    grid_cell_t target_pos =
      grid_geometry_apply_offset(GRID_TYPE_HEXAGON, entry->tile->cell, offset);

    // Check if position is occupied in destination
    if (tile_map_contains(dest, target_pos)) {
      conflicts[conflict_count] = target_pos;
      conflict_count++;
    }
  }

  if (conflict_count == 0) {
    free(conflicts);
    *out_conflicts = NULL;
    *out_count = 0;
    return true;
  }

  // Resize array to exact size needed
  if (conflict_count < source->num_tiles) {
    grid_cell_t *resized =
      realloc(conflicts, conflict_count * sizeof(grid_cell_t));
    if (resized) {
      conflicts = resized;
    }
  }

  *out_conflicts = conflicts;
  *out_count = conflict_count;
  return true;
}

bool tile_map_can_merge_with_offset(const tile_map_t *source,
                                    const tile_map_t *dest,
                                    grid_cell_t offset) {
  if (!source || !dest)
    return false;

  if (source->num_tiles == 0)
    return true; // Empty source can always be merged

  // Check each source tile position after applying offset
  tile_map_entry_t *entry, *tmp;
  HASH_ITER(hh, source->root, entry, tmp) {
    grid_cell_t target_pos =
      grid_geometry_apply_offset(GRID_TYPE_HEXAGON, entry->tile->cell, offset);

    // Check if position is occupied in destination
    if (tile_map_contains(dest, target_pos)) {
      return false; // Found a conflict
    }
  }

  return true; // No conflicts found
}


