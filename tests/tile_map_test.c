#include "../include/tile/tile_map.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
tile_map_entry_t *
make_entry (int type, int q, int r, int s, int value)
{
  tile_t *tile = malloc (sizeof (tile_t));
  tile->cell.type = type;
  tile->cell.coord.hex.q = q;
  tile->cell.coord.hex.r = r;
  tile->cell.coord.hex.s = s;
  tile->value = value;

  tile_map_entry_t *entry = malloc (sizeof (tile_map_entry_t));
  entry->cell = tile->cell;
  entry->tile = tile;
  return entry;
}

void
print_entry (tile_map_entry_t *entry, void *user_data)
{
  printf ("Tile at (type=%d, q=%d, r=%d, s=%d) with value %d\n",
          entry->cell.type, entry->cell.coord.hex.q, entry->cell.coord.hex.r,
          entry->cell.coord.hex.s, entry->tile->value);
}

int
main (void)
{
  tile_map_entry_t *map = tile_map_create ();

  // Add tiles
  tile_map_add (&map, make_entry (1, 0, 0, 0, 10));
  tile_map_add (&map, make_entry (1, 1, 0, -1, 20));
  tile_map_add (&map, make_entry (1, 0, 1, -1, 30));
  assert (tile_map_size (map) == 3);

  // Find a tile
  tile_t search_tile;
  search_tile.cell.type = 1;
  search_tile.cell.coord.hex.q = 1;
  search_tile.cell.coord.hex.r = 0;
  search_tile.cell.coord.hex.s = -1;
  tile_map_entry_t *found = tile_map_find (map, &search_tile);
  assert (found && found->tile->value == 20);

  // Iterate and print
  tile_map_foreach (map, print_entry, NULL);

  // Remove a tile
  found = tile_map_find (map, &search_tile);
  tile_map_remove (&map, found);
  assert (tile_map_size (map) == 2);

  // Clear and free
  tile_map_clear (&map);
  assert (tile_map_size (map) == 0);
  tile_map_free (&map);

  printf ("All tile_map tests passed!\n");
  return 0;
}
