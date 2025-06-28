#include "highlight_manager.h"
#include <stdio.h>
#include <stdlib.h>

#define DEFAULT_HIGHLIGHT_COLOR ORANGE
highlight_manager
highlight_manager_create ()
{
  highlight_manager manager;

  // Initialize the tile_array directly
  manager.tile_array = tile_array_create ();

  // Initialize the highlight
  manager.highlight.color = DEFAULT_HIGHLIGHT_COLOR; // Default highlight color

  // Initialize count and capacity
  manager.count = 0;
  manager.capacity = HIGHLIGHT_MANAGER_MAX_CAPACITY;

  return manager;
}

// Update the highlight manager with new tiles and a highlight color
void
highlight_manager_update (highlight_manager *manager, tile_array *tile_array,
                          Color highlight_color)
{
  // Clear existing highlights
  clear_highlights (manager);

  // Update the highlight color
  manager->highlight.color = highlight_color;

  // Add tiles from the provided tile_array to the manager's tile_array
  for (int i = 0; i < tile_array->count; i++)
    {
      if (manager->count < manager->capacity)
        {
          tile_array_push (&manager->tile_array, tile_array->data[i]);
          manager->count++;
        }
      else
        {
          // Capacity exceeded, handle as needed (e.g., log a warning)
          break;
        }
    }
}

// Free the resources used by the highlight manager
void
highlight_manager_free (highlight_manager *manager)
{
  // Free the internal tile_array
  tile_array_free (&manager->tile_array);

  // No need to free the manager itself since it's not dynamically allocated
}

// Clear all highlights from the highlight manager
static void
clear_highlights (highlight_manager *manager)
{
  tile_array_clear (&manager->tile_array);
  manager->count = 0;
}
