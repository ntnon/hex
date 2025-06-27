#include "tile_group.h" // or the appropriate header for tile_type
#include <stdlib.h>

tile_group_array
tile_group_array_create (void)
{
  tile_group_array array;
  array.count = 0;
  array.capacity = TILE_GROUP_ARRAY_INITIAL_CAPACITY;
  array.data = (tile_group **)malloc (array.capacity * sizeof (tile_group *));
  if (array.data == NULL)
    {
      array.capacity = 0;
    }
  return array;
}

void
tile_group_array_free (tile_group_array *array)
{
  if (array->data != NULL)
    {
      for (int i = 0; i < array->count; i++)
        {
          tile_group_free (array->data[i]);
        }
      free (array->data);
    }
}

void
tile_group_free (tile_group *group)
{
  if (!group)
    return;
  if (group->data)
    {
      free (group->data); // Free the array of tile*
      group->data = NULL;
    }
  group->count = 0;
  free (group); // Free the tile_group struct itself if it was dynamically
                // allocated
}
