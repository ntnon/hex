#include "../../include/tile/tile_map.h"
#include "../../include/grid/grid_cell_utils.h"
#include <stdio.h>
#include <stdlib.h>

tile_map_t *
tile_map_create (void)
{
  tile_map_t *map = malloc (sizeof (tile_map_t));
  if (!map)
    {
      fprintf (stderr, "Out of memory!\n");
      return NULL;
    }
  map->root = NULL;
  map->num_tiles = 0;
  return map;
}

bool
tile_map_contains (tile_map_t *map, grid_cell_t cell)
{
  return (tile_map_find (map, cell) != NULL);
}

void
tile_map_free (tile_map_t *map)
{
  if (!map)
    return;
  tile_map_entry_t *current_entry, *tmp_entry;
  HASH_ITER (hh, map->root, current_entry, tmp_entry)
  {
    HASH_DEL (map->root, current_entry);
    free (current_entry);
  }
  map->num_tiles = 0;
  free (map);
}

tile_map_entry_t *
tile_map_find (tile_map_t *map, grid_cell_t cell)
{
  if (!map)
    return NULL;
  tile_map_entry_t *entry;
  for (entry = map->root; entry != NULL; entry = entry->hh.next)
    {
      if (grid_cells_equal (&entry->cell, &cell))
        return entry;
    }
  return NULL;
}

void
tile_map_remove (tile_map_t *map, grid_cell_t cell)
{
  if (!map)
    return;
  tile_map_entry_t *entry = tile_map_find (map, cell);
  if (entry)
    {
      HASH_DEL (map->root, entry);
      map->num_tiles--;
      free (entry);
    }
}

void
tile_map_foreach_tile (tile_map_t *map, void (*fn) (tile_t *, void *),
                       void *user_data)
{
  if (!map)
    return;
  tile_map_entry_t *entry, *tmp;
  HASH_ITER (hh, map->root, entry, tmp) { fn (entry->tile, user_data); }
}
void
tile_map_add (tile_map_t *map, tile_t *tile)
{
  if (!map || !tile)
    return;
  tile_map_entry_t *existing_entry = tile_map_find (map, tile->cell);
  if (existing_entry)
    {
      // Update the tile pointer in the existing entry
      existing_entry->tile = tile;
    }
  else
    {
      tile_map_entry_t *entry = malloc (sizeof (tile_map_entry_t));
      if (!entry)
        {
          fprintf (stderr, "Out of memory!\n");
          return;
        }
      entry->cell = tile->cell;
      entry->tile = tile;
      HASH_ADD (hh, map->root, cell, sizeof (entry->cell), entry);
      map->num_tiles++;
    }
}
