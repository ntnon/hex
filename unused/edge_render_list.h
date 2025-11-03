#ifndef EDGE_RENDER_LIST_H
#define EDGE_RENDER_LIST_H

#include "third_party/uthash.h"
#include "raylib.h"
#include <stddef.h>
#include <stdbool.h>

/**
 * @brief Represents a single edge to be rendered
 */
typedef struct {
    Vector2 start;
    Vector2 end;
    Color color;
    float thickness;
} render_edge_t;

/**
 * @brief Represents a vertex/corner circle to be rendered
 */
typedef struct {
    Vector2 position;
    Color color;
    float radius;
} render_vertex_t;

/**
 * @brief Hash table entry for edge deduplication
 */
typedef struct edge_hash_entry {
    char key[64];              /* String key: "x1,y1,x2,y2" */
    render_edge_t edge;        /* The edge data */
    UT_hash_handle hh;         /* UTHash handle */
} edge_hash_entry_t;

/**
 * @brief Hash table entry for vertex deduplication
 */
typedef struct vertex_hash_entry {
    char key[32];              /* String key: "x,y" */
    render_vertex_t vertex;    /* The vertex data */
    UT_hash_handle hh;         /* UTHash handle */
} vertex_hash_entry_t;

/**
 * @brief Edge render list container with deduplication
 */
typedef struct {
    edge_hash_entry_t *edge_hash;     /* Hash table for edges */
    vertex_hash_entry_t *vertex_hash; /* Hash table for vertices */
    
    render_edge_t *edges;             /* Final array of unique edges */
    render_vertex_t *vertices;        /* Final array of unique vertices */
    
    size_t num_edges;
    size_t num_vertices;
    size_t edge_capacity;
    size_t vertex_capacity;
    
    bool needs_rebuild;               /* Flag to track if arrays need rebuilding */
} edge_render_list_t;

/* Function declarations */

/**
 * @brief Creates a new edge render list
 * @return Pointer to new edge render list, or NULL on failure
 */
edge_render_list_t *edge_render_list_create(void);

/**
 * @brief Frees an edge render list and all its data
 * @param list The list to free
 */
void edge_render_list_free(edge_render_list_t *list);

/**
 * @brief Clears all edges and vertices from the list
 * @param list The list to clear
 */
void edge_render_list_clear(edge_render_list_t *list);

/**
 * @brief Adds an edge to the list (with automatic deduplication)
 * @param list The list to add to
 * @param start Start point of the edge
 * @param end End point of the edge
 * @param color Color of the edge
 * @param thickness Thickness of the edge line
 * @return True if edge was added (new), false if it was a duplicate
 */
bool edge_render_list_add_edge(edge_render_list_t *list, Vector2 start, Vector2 end, 
                               Color color, float thickness);

/**
 * @brief Adds a vertex to the list (with automatic deduplication)
 * @param list The list to add to
 * @param position Position of the vertex
 * @param color Color of the vertex circle
 * @param radius Radius of the vertex circle
 * @return True if vertex was added (new), false if it was a duplicate
 */
bool edge_render_list_add_vertex(edge_render_list_t *list, Vector2 position, 
                                 Color color, float radius);

/**
 * @brief Rebuilds the final arrays from hash tables (call before rendering)
 * @param list The list to rebuild
 * @return True if rebuild was successful, false on memory allocation failure
 */
bool edge_render_list_rebuild_arrays(edge_render_list_t *list);

/**
 * @brief Gets the array of edges ready for rendering
 * @param list The list to get edges from
 * @param out_count Pointer to store the number of edges
 * @return Pointer to edge array, or NULL if needs rebuild
 */
const render_edge_t *edge_render_list_get_edges(edge_render_list_t *list, size_t *out_count);

/**
 * @brief Gets the array of vertices ready for rendering
 * @param list The list to get vertices from
 * @param out_count Pointer to store the number of vertices
 * @return Pointer to vertex array, or NULL if needs rebuild
 */
const render_vertex_t *edge_render_list_get_vertices(edge_render_list_t *list, size_t *out_count);

#endif // EDGE_RENDER_LIST_H