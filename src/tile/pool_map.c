#include "../../include/tile/pool_map.h"
#include <stdio.h>

pool_map_entry_t *
pool_map_create (void)
{
  return NULL;
}

void
pool_map_free (pool_map_entry_t **map_root)
{
  pool_map_entry_t *el, *tmp;
  HASH_ITER (hh, *map_root, el, tmp)
  {
    HASH_DEL (*map_root, el);
    free (el);
  }
  *map_root = NULL;
}

void
pool_map_add (pool_map_entry_t **map_root, int id, pool_t *pool)
{
  pool_map_entry_t *existing = find_pool_by_id (*map_root, id);
  if (existing)
    {
      // Remove and free the existing entry
      HASH_DEL (*map_root, existing);
      free (existing);
    }
  pool_map_entry_t *entry = malloc (sizeof (pool_map_entry_t));
  entry->id = id;
  entry->pool = pool;
  HASH_ADD_INT (*map_root, id, entry);
}

pool_map_entry_t *
find_pool_by_id (pool_map_entry_t *map_root, int id)
{
  pool_map_entry_t *entry = NULL;
  HASH_FIND_INT (map_root, &id, entry);
  // Remove this debug print - it's normal for entries not to exist
  // if (!entry)
  //   printf ("Entry with ID %d not found.\n", id);
  return entry;
}

void
pool_map_remove (pool_map_entry_t **map_root, int id)
{
  // Find the entry by ID
  pool_map_entry_t *entry_to_remove = NULL;
  HASH_FIND_INT (*map_root, &id, entry_to_remove);

  // If the entry exists, remove it
  if (entry_to_remove)
    {
      HASH_DEL (*map_root, entry_to_remove);
      free (entry_to_remove);
    }
}
