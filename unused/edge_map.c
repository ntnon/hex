#include "../../include/grid/edge_map.h"
#include <stdlib.h>
#include <string.h>

// --- Core edge map operations ---

void edge_map_canonicalize(grid_edge_t *edge) {
  if (!edge)
    return;

  // Ensure a < b lexicographically (x first, then y)
  if ((edge->a.x > edge->b.x) ||
      (edge->a.x == edge->b.x && edge->a.y > edge->b.y)) {
    point_t tmp = edge->a;
    edge->a = edge->b;
    edge->b = tmp;
  }
}

edge_map_entry_t *edge_map_create_entry(grid_edge_t edge) {
  edge_map_entry_t *entry = malloc(sizeof(edge_map_entry_t));
  if (!entry)
    return NULL;

  entry->edge = edge;
  edge_map_canonicalize(&entry->edge);
  return entry;
}

void edge_map_insert(edge_map_entry_t **map_root, edge_map_entry_t *entry) {
  if (!map_root || !entry)
    return;

  edge_map_canonicalize(&entry->edge);
  HASH_ADD(hh, *map_root, edge, sizeof(grid_edge_t), entry);
}

bool edge_map_add_edge(edge_map_entry_t **map_root, grid_edge_t edge) {
  if (!map_root)
    return false;

  edge_map_canonicalize(&edge);

  // Check if already exists
  edge_map_entry_t *found = NULL;
  HASH_FIND(hh, *map_root, &edge, sizeof(grid_edge_t), found);
  if (found) {
    return false; // Already exists
  }

  // Create and add new entry
  edge_map_entry_t *entry = edge_map_create_entry(edge);
  if (!entry)
    return false;

  edge_map_insert(map_root, entry);
  return true;
}

bool edge_map_contains(edge_map_entry_t *map_root, grid_edge_t edge) {
  edge_map_canonicalize(&edge);

  edge_map_entry_t *found = NULL;
  HASH_FIND(hh, map_root, &edge, sizeof(grid_edge_t), found);
  return (found != NULL);
}

bool edge_map_remove_edge(edge_map_entry_t **map_root, grid_edge_t edge) {
  if (!map_root)
    return false;

  edge_map_canonicalize(&edge);

  edge_map_entry_t *found = NULL;
  HASH_FIND(hh, *map_root, &edge, sizeof(grid_edge_t), found);
  if (found) {
    HASH_DEL(*map_root, found);
    free(found);
    return true;
  }
  return false;
}

void edge_map_clear(edge_map_entry_t **map_root) {
  if (!map_root)
    return;
  HASH_CLEAR(hh, *map_root);
}

void edge_map_free(edge_map_entry_t **map_root) {
  if (!map_root)
    return;

  edge_map_entry_t *current, *tmp;
  HASH_ITER(hh, *map_root, current, tmp) {
    HASH_DEL(*map_root, current);
    free(current);
  }
}

void edge_map_merge(edge_map_entry_t **dst_map, edge_map_entry_t *src_map) {
  if (!dst_map)
    return;

  edge_map_entry_t *entry, *tmp;
  HASH_ITER(hh, src_map, entry, tmp) {
    edge_map_add_edge(dst_map, entry->edge);
  }
}

size_t edge_map_count(edge_map_entry_t *map_root) {
  return HASH_COUNT(map_root);
}

// --- Grid geometry integration ---

void edge_map_add_cell_edges(grid_type_e grid_type, const layout_t *layout,
                             grid_cell_t cell, edge_map_entry_t **map_root) {
  if (!layout || !map_root)
    return;

  // Get number of corners for this grid type
  int corner_count = grid_geometry_get_corner_count(grid_type);
  if (corner_count <= 0)
    return;

  // Allocate space for corners
  point_t *corners = malloc(corner_count * sizeof(point_t));
  if (!corners)
    return;

  // Get the corners
  grid_geometry_get_corners(grid_type, layout, cell, corners);

  // Add edges between consecutive corners
  edge_map_add_corners(corners, corner_count, map_root);

  free(corners);
}

void edge_map_add_corners(point_t *corners, int corner_count,
                          edge_map_entry_t **map_root) {
  if (!corners || !map_root || corner_count < 2)
    return;

  // Create edges between consecutive corners
  for (int i = 0; i < corner_count; i++) {
    grid_edge_t edge = {.a = corners[i], .b = corners[(i + 1) % corner_count]};
    edge_map_add_edge(map_root, edge);
  }
}

void edge_map_add_cells_edges(grid_type_e grid_type, const layout_t *layout,
                              grid_cell_t *cells, size_t cell_count,
                              edge_map_entry_t **map_root) {
  if (!layout || !cells || !map_root)
    return;

  for (size_t i = 0; i < cell_count; i++) {
    edge_map_add_cell_edges(grid_type, layout, cells[i], map_root);
  }
}

size_t edge_map_find_external_edges(grid_type_e grid_type,
                                    const layout_t *layout, grid_cell_t *cells,
                                    size_t cell_count,
                                    edge_map_entry_t **external_edges) {
  if (!layout || !cells || !external_edges)
    return 0;

  // Clear output map
  edge_map_free(external_edges);

  // Count occurrences of each edge
  typedef struct {
    grid_edge_t edge;
    int count;
    UT_hash_handle hh;
  } edge_count_entry_t;

  edge_count_entry_t *edge_counts = NULL;

  // Get corner count for this grid type
  int corner_count = grid_geometry_get_corner_count(grid_type);
  if (corner_count <= 0) {
    return 0;
  }

  point_t *corners = malloc(corner_count * sizeof(point_t));
  if (!corners)
    return 0;

  // Count edge occurrences
  for (size_t i = 0; i < cell_count; i++) {
    grid_geometry_get_corners(grid_type, layout, cells[i], corners);

    for (int j = 0; j < corner_count; j++) {
      grid_edge_t edge = {.a = corners[j],
                          .b = corners[(j + 1) % corner_count]};
      edge_map_canonicalize(&edge);

      edge_count_entry_t *found = NULL;
      HASH_FIND(hh, edge_counts, &edge, sizeof(grid_edge_t), found);
      if (found) {
        found->count++;
      } else {
        edge_count_entry_t *entry = malloc(sizeof(edge_count_entry_t));
        entry->edge = edge;
        entry->count = 1;
        HASH_ADD(hh, edge_counts, edge, sizeof(grid_edge_t), entry);
      }
    }
  }

  free(corners);

  // Extract edges that appear only once (external)
  size_t external_count = 0;
  edge_count_entry_t *entry, *tmp;
  HASH_ITER(hh, edge_counts, entry, tmp) {
    if (entry->count == 1) {
      edge_map_add_edge(external_edges, entry->edge);
      external_count++;
    }
    HASH_DEL(edge_counts, entry);
    free(entry);
  }

  return external_count;
}

size_t edge_map_find_internal_edges(grid_type_e grid_type,
                                    const layout_t *layout, grid_cell_t *cells,
                                    size_t cell_count,
                                    edge_map_entry_t **internal_edges) {
  if (!layout || !cells || !internal_edges)
    return 0;

  // Clear output map
  edge_map_free(internal_edges);

  // Count occurrences of each edge
  typedef struct {
    grid_edge_t edge;
    int count;
    UT_hash_handle hh;
  } edge_count_entry_t;

  edge_count_entry_t *edge_counts = NULL;

  // Get corner count for this grid type
  int corner_count = grid_geometry_get_corner_count(grid_type);
  if (corner_count <= 0) {
    return 0;
  }

  point_t *corners = malloc(corner_count * sizeof(point_t));
  if (!corners)
    return 0;

  // Count edge occurrences
  for (size_t i = 0; i < cell_count; i++) {
    grid_geometry_get_corners(grid_type, layout, cells[i], corners);

    for (int j = 0; j < corner_count; j++) {
      grid_edge_t edge = {.a = corners[j],
                          .b = corners[(j + 1) % corner_count]};
      edge_map_canonicalize(&edge);

      edge_count_entry_t *found = NULL;
      HASH_FIND(hh, edge_counts, &edge, sizeof(grid_edge_t), found);
      if (found) {
        found->count++;
      } else {
        edge_count_entry_t *entry = malloc(sizeof(edge_count_entry_t));
        entry->edge = edge;
        entry->count = 1;
        HASH_ADD(hh, edge_counts, edge, sizeof(grid_edge_t), entry);
      }
    }
  }

  free(corners);

  // Extract edges that appear more than once (internal/shared)
  size_t internal_count = 0;
  edge_count_entry_t *entry, *tmp;
  HASH_ITER(hh, edge_counts, entry, tmp) {
    if (entry->count > 1) {
      edge_map_add_edge(internal_edges, entry->edge);
      internal_count++;
    }
    HASH_DEL(edge_counts, entry);
    free(entry);
  }

  return internal_count;
}
