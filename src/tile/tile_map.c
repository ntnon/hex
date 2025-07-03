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

void
tile_map_clear (tile_map_entry_t **map_root)
{
  HASH_CLEAR (hh, *map_root);
}

void
tile_map_remove (tile_map_entry_t **map_root,
                 tile_map_entry_t *entry_to_remove)
{
  HASH_DEL (*map_root, entry_to_remove);
  free (entry_to_remove);
}

tile_map_entry_t *
tile_map_find (tile_map_entry_t *map_root, tile_t *search_tile)
{
  tile_map_entry_t *found_entry
      = NULL; // Declare a pointer to store the found entry

  HASH_FIND (hh, map_root, &search_tile->cell, sizeof (grid_cell_t),
             found_entry);
  return found_entry;
};

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
tile_map_replace (tile_map_entry_t **map_root,
                  tile_map_entry_t *entry_to_replace,
                  tile_map_entry_t *entry_to_add)
{
  HASH_DEL (*map_root, entry_to_replace);
  HASH_ADD (hh, *map_root, cell, sizeof (entry_to_add->cell), entry_to_add);
  free (entry_to_replace);
}

void
tile_map_add (tile_map_entry_t **map_root, tile_map_entry_t *entry)
{
  tile_map_entry_t *existing_entry = tile_map_find (*map_root, entry->tile);
  if (existing_entry)
    {
      tile_map_replace (map_root, existing_entry, entry);
    }
  else
    {
      HASH_ADD (hh, *map_root, cell, sizeof (entry->cell), entry);
    }
}
