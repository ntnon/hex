#ifndef EDGE_MAP_H
#define EDGE_MAP_H

#include "grid_geometry.h"
#include "grid_types.h"
#include "../third_party/uthash.h"

/**
 * @brief Entry in an edge hash map.
 * 
 * Edges are stored in canonical form (smaller point first) to ensure
 * consistent hashing regardless of edge direction.
 */
typedef struct edge_map_entry {
    grid_edge_t edge;
    UT_hash_handle hh;
} edge_map_entry_t;

/**
 * @brief Canonicalizes an edge by ensuring point a < point b lexicographically.
 * @param edge The edge to canonicalize (modified in place).
 */
void edge_map_canonicalize(grid_edge_t *edge);

/**
 * @brief Creates a new edge map entry with a canonicalized edge.
 * @param edge The edge to store (will be canonicalized).
 * @return Newly allocated edge map entry, or NULL on failure.
 */
edge_map_entry_t *edge_map_create_entry(grid_edge_t edge);

/**
 * @brief Inserts an edge map entry into the hash map.
 * @param map_root Pointer to the root of the edge map.
 * @param entry The entry to insert (edge will be canonicalized).
 */
void edge_map_insert(edge_map_entry_t **map_root, edge_map_entry_t *entry);

/**
 * @brief Adds an edge to the map if it doesn't already exist.
 * @param map_root Pointer to the root of the edge map.
 * @param edge The edge to add (will be canonicalized).
 * @return true if edge was added, false if it already existed.
 */
bool edge_map_add_edge(edge_map_entry_t **map_root, grid_edge_t edge);

/**
 * @brief Checks if an edge exists in the map.
 * @param map_root Root of the edge map.
 * @param edge The edge to check (will be canonicalized for comparison).
 * @return true if edge exists in map.
 */
bool edge_map_contains(edge_map_entry_t *map_root, grid_edge_t edge);

/**
 * @brief Removes an edge from the map.
 * @param map_root Pointer to the root of the edge map.
 * @param edge The edge to remove (will be canonicalized for lookup).
 * @return true if edge was removed, false if not found.
 */
bool edge_map_remove_edge(edge_map_entry_t **map_root, grid_edge_t edge);

/**
 * @brief Clears all entries from the map without freeing memory.
 * @param map_root Pointer to the root of the edge map.
 */
void edge_map_clear(edge_map_entry_t **map_root);

/**
 * @brief Frees all memory associated with the edge map.
 * @param map_root Pointer to the root of the edge map.
 */
void edge_map_free(edge_map_entry_t **map_root);

/**
 * @brief Merges edges from source map into destination map.
 * @param dst_map Pointer to destination edge map root.
 * @param src_map Source edge map to merge from.
 */
void edge_map_merge(edge_map_entry_t **dst_map, edge_map_entry_t *src_map);

/**
 * @brief Gets the number of edges in the map.
 * @param map_root Root of the edge map.
 * @return Number of edges in the map.
 */
size_t edge_map_count(edge_map_entry_t *map_root);

// --- Grid geometry integration ---

/**
 * @brief Adds all edges of a cell to the edge map.
 * 
 * Uses the grid geometry system to get cell corners and creates edges
 * between consecutive corners.
 * 
 * @param grid_type The type of grid.
 * @param layout The layout for coordinate conversion.
 * @param cell The cell whose edges to add.
 * @param map_root Pointer to the root of the edge map.
 */
void edge_map_add_cell_edges(grid_type_e grid_type, const layout_t *layout,
                              grid_cell_t cell, edge_map_entry_t **map_root);

/**
 * @brief Creates edges from an array of corner points.
 * 
 * Creates edges between consecutive corners (including last to first).
 * 
 * @param corners Array of corner points.
 * @param corner_count Number of corners.
 * @param map_root Pointer to the root of the edge map.
 */
void edge_map_add_corners(point_t *corners, int corner_count,
                           edge_map_entry_t **map_root);

/**
 * @brief Gets all edges of multiple cells.
 * 
 * Useful for getting the boundary edges of a collection of cells.
 * 
 * @param grid_type The type of grid.
 * @param layout The layout for coordinate conversion.
 * @param cells Array of cells.
 * @param cell_count Number of cells.
 * @param map_root Pointer to the root of the edge map.
 */
void edge_map_add_cells_edges(grid_type_e grid_type, const layout_t *layout,
                               grid_cell_t *cells, size_t cell_count,
                               edge_map_entry_t **map_root);

/**
 * @brief Finds external edges from a collection of cells.
 * 
 * External edges are those that appear only once (not shared between cells).
 * 
 * @param grid_type The type of grid.
 * @param layout The layout for coordinate conversion.
 * @param cells Array of cells.
 * @param cell_count Number of cells.
 * @param external_edges Pointer to store the external edges map.
 * @return Number of external edges found.
 */
size_t edge_map_find_external_edges(grid_type_e grid_type, const layout_t *layout,
                                     grid_cell_t *cells, size_t cell_count,
                                     edge_map_entry_t **external_edges);

/**
 * @brief Finds internal (shared) edges from a collection of cells.
 * 
 * Internal edges are those that appear more than once (shared between cells).
 * 
 * @param grid_type The type of grid.
 * @param layout The layout for coordinate conversion.
 * @param cells Array of cells.
 * @param cell_count Number of cells.
 * @param internal_edges Pointer to store the internal edges map.
 * @return Number of internal edges found.
 */
size_t edge_map_find_internal_edges(grid_type_e grid_type, const layout_t *layout,
                                     grid_cell_t *cells, size_t cell_count,
                                     edge_map_entry_t **internal_edges);

#endif // EDGE_MAP_H