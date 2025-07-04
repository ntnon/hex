#include "../../include/tile/tile_map.h"

tile_map_entry_t *
tile_map_create (void)
{
  return NULL;
}

void
tile_map_free (tile_map_entry_t **map_root)
{
  tile_map_entry_t *current_entry, *tmp_entry;
  HASH_ITER (hh, *map_root, current_entry, tmp_entry)
  {
    HASH_DEL (*map_root, current_entry);
    free (current_entry);
  }
}

tile_map_entry_t *
tile_map_find (tile_map_entry_t *map_root, grid_cell_t cell)
{
  tile_map_entry_t *found = NULL;
  HASH_FIND (hh, map_root, &cell, sizeof (grid_cell_t), found);
  return found;
}

void
tile_map_remove (tile_map_entry_t **map_root, grid_cell_t cell)
{
  tile_map_entry_t *entry = tile_map_find (*map_root, cell);
  if (entry)
    {
      HASH_DEL (*map_root, entry);
      free (entry);
    }
}

void
tile_map_foreach (tile_map_entry_t *map_root,
                  void (*fn) (tile_map_entry_t *, void *), void *user_data)
{
  tile_map_entry_t *entry, *tmp;
  HASH_ITER (hh, map_root, entry, tmp) { fn (entry, user_data); }
}

int
tile_map_size (tile_map_entry_t *map_root)
{
  tile_map_entry_t *entry, *tmp;
  int size = 0;
  HASH_ITER (hh, map_root, entry, tmp) { size++; }
  return size;
}

void
tile_map_add (tile_map_entry_t **map_root, tile_t *tile)
{
  // Use tile->cell as the key for lookup
  tile_map_entry_t *existing_entry = tile_map_find (*map_root, tile->cell);
  if (existing_entry)
    {
      // Update the tile pointer in the existing entry
      existing_entry->tile
          = tile; // const-correct if struct uses const tile_t *
    }
  else
    {
      tile_map_entry_t *entry = malloc (sizeof (tile_map_entry_t));
      entry->cell = tile->cell;
      entry->tile = tile; // const-correct if struct uses const tile_t *
      HASH_ADD (hh, *map_root, cell, sizeof (entry->cell), entry);
    }
}
