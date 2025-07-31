#ifndef GRID_CELL_COLLECTION_H
#define GRID_CELL_COLLECTION_H

#include "grid_system.h"
#include "../third_party/uthash.h"

typedef struct {
    grid_edge_t edge;
    UT_hash_handle hh;
} edge_map_entry_t;

void canonicalize_edge(grid_edge_t *edge);
edge_map_entry_t *edge_map_create(void);
edge_map_entry_t *edge_map_create_entry(grid_edge_t edge);
void edge_map_add(edge_map_entry_t **root, edge_map_entry_t *entry);
void edge_map_free(edge_map_entry_t **root);
void edge_map_insert(edge_map_entry_t **root, edge_map_entry_t *entry);


void
edge_map_merge (edge_map_entry_t **dst_map, edge_map_entry_t *src_map);


/**
 * @brief Adds all unique edges of a hexagonal cell to an edge map.
 *
 * Given an array of 6 corner points for a hex cell, this function computes
 * each of the 6 edges (as pairs of consecutive corners), canonicalizes each
 * edge, and inserts it into the provided edge hash table if it does not
 * already exist. This ensures that each unique edge is only stored once,
 * even if shared by multiple cells.
 *
 * @param corners         Array of 6 corner points defining the cell.
 * @param edge_map_root   Pointer to the root of the edge hash table.
 */
void
 corners_to_edges(point_t corners[6], edge_map_entry_t **edge_map_root);

 void
 edge_map_clear (edge_map_entry_t **map_root);

//utility

void get_cell_edges(const grid_t *grid, grid_cell_t cell, edge_map_entry_t **edges);
void collect_all_edges(const grid_t *grid, edge_map_entry_t **edges);

#endif
