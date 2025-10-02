#include "../../include/renderer/edge_render_list.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Tolerance for floating point comparison
#define FLOAT_TOLERANCE 0.001f

// Helper function to create edge key for hashing
static void make_edge_key(char *key, Vector2 start, Vector2 end) {
  // Normalize edge direction (smaller point first) for consistent hashing
  if (start.x < end.x ||
      (fabs(start.x - end.x) < FLOAT_TOLERANCE && start.y < end.y)) {
    snprintf(key, 64, "%.3f,%.3f,%.3f,%.3f", start.x, start.y, end.x, end.y);
  } else {
    snprintf(key, 64, "%.3f,%.3f,%.3f,%.3f", end.x, end.y, start.x, start.y);
  }
}

// Helper function to create vertex key for hashing
static void make_vertex_key(char *key, Vector2 position) {
  snprintf(key, 32, "%.3f,%.3f", position.x, position.y);
}

edge_render_list_t *edge_render_list_create(void) {
  edge_render_list_t *list = malloc(sizeof(edge_render_list_t));
  if (!list) {
    fprintf(stderr, "Failed to allocate edge render list\n");
    return NULL;
  }

  list->edge_hash = NULL;
  list->vertex_hash = NULL;
  list->edges = NULL;
  list->vertices = NULL;
  list->num_edges = 0;
  list->num_vertices = 0;
  list->edge_capacity = 0;
  list->vertex_capacity = 0;
  list->needs_rebuild = false;

  return list;
}

void edge_render_list_free(edge_render_list_t *list) {
  if (!list)
    return;

  // Free hash tables
  edge_hash_entry_t *edge_entry, *edge_tmp;
  HASH_ITER(hh, list->edge_hash, edge_entry, edge_tmp) {
    HASH_DEL(list->edge_hash, edge_entry);
    free(edge_entry);
  }

  vertex_hash_entry_t *vertex_entry, *vertex_tmp;
  HASH_ITER(hh, list->vertex_hash, vertex_entry, vertex_tmp) {
    HASH_DEL(list->vertex_hash, vertex_entry);
    free(vertex_entry);
  }

  // Free arrays
  free(list->edges);
  free(list->vertices);

  free(list);
}

void edge_render_list_clear(edge_render_list_t *list) {
  if (!list)
    return;

  // Clear hash tables
  edge_hash_entry_t *edge_entry, *edge_tmp;
  HASH_ITER(hh, list->edge_hash, edge_entry, edge_tmp) {
    HASH_DEL(list->edge_hash, edge_entry);
    free(edge_entry);
  }
  list->edge_hash = NULL;

  vertex_hash_entry_t *vertex_entry, *vertex_tmp;
  HASH_ITER(hh, list->vertex_hash, vertex_entry, vertex_tmp) {
    HASH_DEL(list->vertex_hash, vertex_entry);
    free(vertex_entry);
  }
  list->vertex_hash = NULL;

  // Reset counts
  list->num_edges = 0;
  list->num_vertices = 0;
  list->needs_rebuild = true;
}

bool edge_render_list_add_edge(edge_render_list_t *list, Vector2 start,
                               Vector2 end, Color color, float thickness) {
  if (!list)
    return false;

  char key[64];
  make_edge_key(key, start, end);

  // Check if edge already exists
  edge_hash_entry_t *existing = NULL;
  HASH_FIND_STR(list->edge_hash, key, existing);
  if (existing) {
    return false; // Duplicate edge
  }

  // Create new edge entry
  edge_hash_entry_t *entry = malloc(sizeof(edge_hash_entry_t));
  if (!entry) {
    fprintf(stderr, "Failed to allocate edge hash entry\n");
    return false;
  }

  strcpy(entry->key, key);
  entry->edge.start = start;
  entry->edge.end = end;
  entry->edge.color = color;
  entry->edge.thickness = thickness;

  // Add to hash table
  HASH_ADD_STR(list->edge_hash, key, entry);
  list->needs_rebuild = true;

  return true; // New edge added
}

bool edge_render_list_add_vertex(edge_render_list_t *list, Vector2 position,
                                 Color color, float radius) {
  if (!list)
    return false;

  char key[32];
  make_vertex_key(key, position);

  // Check if vertex already exists
  vertex_hash_entry_t *existing = NULL;
  HASH_FIND_STR(list->vertex_hash, key, existing);
  if (existing) {
    return false; // Duplicate vertex
  }

  // Create new vertex entry
  vertex_hash_entry_t *entry = malloc(sizeof(vertex_hash_entry_t));
  if (!entry) {
    fprintf(stderr, "Failed to allocate vertex hash entry\n");
    return false;
  }

  strcpy(entry->key, key);
  entry->vertex.position = position;
  entry->vertex.color = color;
  entry->vertex.radius = radius;

  // Add to hash table
  HASH_ADD_STR(list->vertex_hash, key, entry);
  list->needs_rebuild = true;

  return true; // New vertex added
}

bool edge_render_list_rebuild_arrays(edge_render_list_t *list) {
  if (!list)
    return false;
  if (!list->needs_rebuild)
    return true; // Already up to date

  // Count entries
  size_t edge_count = HASH_COUNT(list->edge_hash);
  size_t vertex_count = HASH_COUNT(list->vertex_hash);

  // Reallocate arrays if needed
  if (edge_count > list->edge_capacity) {
    list->edge_capacity = edge_count * 2; // Grow with some headroom
    list->edges =
      realloc(list->edges, list->edge_capacity * sizeof(render_edge_t));
    if (!list->edges && list->edge_capacity > 0) {
      fprintf(stderr, "Failed to reallocate edge array\n");
      return false;
    }
  }

  if (vertex_count > list->vertex_capacity) {
    list->vertex_capacity = vertex_count * 2; // Grow with some headroom
    list->vertices =
      realloc(list->vertices, list->vertex_capacity * sizeof(render_vertex_t));
    if (!list->vertices && list->vertex_capacity > 0) {
      fprintf(stderr, "Failed to reallocate vertex array\n");
      return false;
    }
  }

  // Copy edges from hash table to array
  list->num_edges = 0;
  edge_hash_entry_t *edge_entry, *edge_tmp;
  HASH_ITER(hh, list->edge_hash, edge_entry, edge_tmp) {
    list->edges[list->num_edges] = edge_entry->edge;
    list->num_edges++;
  }

  // Copy vertices from hash table to array
  list->num_vertices = 0;
  vertex_hash_entry_t *vertex_entry, *vertex_tmp;
  HASH_ITER(hh, list->vertex_hash, vertex_entry, vertex_tmp) {
    list->vertices[list->num_vertices] = vertex_entry->vertex;
    list->num_vertices++;
  }

  list->needs_rebuild = false;
  return true;
}

const render_edge_t *edge_render_list_get_edges(edge_render_list_t *list,
                                                size_t *out_count) {
  if (!list || !out_count) {
    if (out_count)
      *out_count = 0;
    return NULL;
  }

  if (list->needs_rebuild) {
    if (!edge_render_list_rebuild_arrays(list)) {
      *out_count = 0;
      return NULL;
    }
  }

  *out_count = list->num_edges;
  return list->edges;
}

const render_vertex_t *edge_render_list_get_vertices(edge_render_list_t *list,
                                                     size_t *out_count) {
  if (!list || !out_count) {
    if (out_count)
      *out_count = 0;
    return NULL;
  }

  if (list->needs_rebuild) {
    if (!edge_render_list_rebuild_arrays(list)) {
      *out_count = 0;
      return NULL;
    }
  }

  *out_count = list->num_vertices;
  return list->vertices;
}
