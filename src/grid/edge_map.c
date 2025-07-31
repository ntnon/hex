#include "../../include/grid/edge_map.h"
#include "../../include/grid/grid_system.h"
#include "../../include/grid/grid_types.h"
#include <stdio.h>
#include <stdlib.h>

void
canonicalize_edge (grid_edge_t *edge)
{
  // Swap points if needed so a < b lexicographically
  if ((edge->a.x > edge->b.x)
      || (edge->a.x == edge->b.x && edge->a.y > edge->b.y))
    {
      point_t tmp = edge->a;
      edge->a = edge->b;
      edge->b = tmp;
    }
}

edge_map_entry_t *
edge_map_create (void)
{
  return NULL;
}

edge_map_entry_t *
edge_map_create_entry (grid_edge_t edge)
{
  edge_map_entry_t *entry = malloc (sizeof (edge_map_entry_t));
  if (!entry)
    return NULL;

  entry->edge = edge;
  canonicalize_edge (&entry->edge);
  return entry;
}

void
edge_map_insert (edge_map_entry_t **map_root, edge_map_entry_t *entry)
{
  canonicalize_edge (&entry->edge);
  HASH_ADD_KEYPTR (hh, *map_root, &entry->edge, sizeof (grid_edge_t), entry);
}

void
edge_map_clear (edge_map_entry_t **map_root)
{
  HASH_CLEAR (hh, *map_root);
};

void
edge_map_free (edge_map_entry_t **map_root)
{
  edge_map_entry_t *current_entry, *tmp_entry;
  HASH_ITER (hh, *map_root, current_entry, tmp_entry)
  {
    HASH_DEL (*map_root, current_entry);
    free (current_entry);
  }
}

void
corners_to_edges (point_t corners[6], edge_map_entry_t **edge_map_root)
{
  for (int i = 0; i < 6; ++i)
    {
      grid_edge_t e = { corners[i], corners[(i + 1) % 6] };
      canonicalize_edge (&e);

      edge_map_entry_t *found = NULL;
      HASH_FIND (hh, *edge_map_root, &e, sizeof (grid_edge_t), found);
      if (!found)
        {
          edge_map_entry_t *new_entry = edge_map_create_entry (e);
          edge_map_insert (edge_map_root, new_entry);
        }
    }
}

void
get_cell_edges (const grid_t *grid, grid_cell_t cell,
                edge_map_entry_t **edge_map_root)
{
  point_t corners[6];
  grid->vtable->get_corners (grid, cell, corners);
  corners_to_edges (corners, edge_map_root);
}

void
edge_map_merge (edge_map_entry_t **dst_map, edge_map_entry_t *src_map)
{
  edge_map_entry_t *entry, *tmp;
  HASH_ITER (hh, src_map, entry, tmp)
  {
    // Check if already exists in dst_map
    edge_map_entry_t *found = NULL;
    HASH_FIND (hh, *dst_map, &entry->edge, sizeof (grid_edge_t), found);
    if (!found)
      {
        // Create a copy if you want independent ownership
        edge_map_entry_t *copy = malloc (sizeof (edge_map_entry_t));
        copy->edge = entry->edge;
        HASH_ADD (hh, *dst_map, edge, sizeof (grid_edge_t), copy);
      }
    // If you want to overwrite, you could remove and add, or just skip this
    // check
  }
}
