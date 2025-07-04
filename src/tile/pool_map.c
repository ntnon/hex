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
pool_map_add (pool_map_entry_t **map_root, pool_map_entry_t *entry_to_add)
{
  pool_map_entry_t *existing = NULL;
  HASH_FIND_INT (*map_root, &entry_to_add->id, existing);
  if (existing)
    {
      printf ("Entry with ID %d already exists.\n", entry_to_add->id);
      HASH_DEL (*map_root, existing);
      free (existing);
    }
  HASH_ADD_INT (*map_root, id, entry_to_add);
}

pool_map_entry_t *
find_pool_by_id (pool_map_entry_t *map_root, int id)
{
  pool_map_entry_t *entry = NULL;
  HASH_FIND_INT (map_root, &id, entry);
  if (!entry)
    printf ("Entry with ID %d not found.\n", id);
  return entry;
}

void pool_map_remove (pool_map_entry_t **map_root,
                      pool_map_entry_t *entry_to_remove);
void pool_map_clear (pool_map_entry_t **map_root);

void
pool_map_merge (pool_map_entry_t **destination, pool_map_entry_t *source)
{
  pool_map_entry_t *entry, *tmp;
  HASH_ITER (hh, source, entry, tmp)
  {
    HASH_DEL (source, entry);
    pool_map_add (destination, entry);
  }
  // No need to free source, as all entries have been moved
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
pool_map_remove (pool_map_entry_t **map_root,
                 pool_map_entry_t *entry_to_remove)
{
  HASH_DEL (*map_root, entry_to_remove);
  free (entry_to_remove);
}
