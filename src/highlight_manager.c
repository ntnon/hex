#include "highlight_manager.h"
#include <stdio.h>
#include <stdlib.h>

#define DEFAULT_HIGHLIGHT_COLOR ORANGE
highlight_manager *
highlight_manager_create (highlight highlight)
{
  // Dynamically allocate memory for the highlight_manager
  highlight_manager *manager = malloc (sizeof (highlight_manager));
  if (!manager)
    {
      fprintf (stderr, "Failed to allocate memory for highlight_manager\n");
      return NULL;
    }

  // Initialize the tile_array
  manager->tile_array = tile_array_create ();
  if (!manager->tile_array)
    {
      fprintf (
          stderr,
          "Failed to allocate memory for tile_array in highlight_manager.\n");
      free (manager); // Free the manager if tile_array creation fails
      return NULL;
    }

  // Initialize the highlight

  manager->highlight = highlight;

  return manager;
}

void
update_highlight (highlight_manager *manager, highlight highlight)
{
  manager->highlight = highlight;
}

void
highlight_manager_set_tile (highlight_manager *manager, tile tile)
{
  clear_highlights (manager);
  tile_array_push (manager->tile_array, tile);
}
// Update the highlight manager with new tiles and a highlight color
void
highlight_manager_set_tile_array (highlight_manager *manager,
                                  tile_array tile_array)
{
  // Clear existing highlights
  clear_highlights (manager);

  // Add tiles from the provided tile_array to the manager's tile_array
  for (int i = 0; i < tile_array.count; i++)
    tile_array_push (manager->tile_array, tile_array.data[i]);
}

// Free the resources used by the highlight manager
void
highlight_manager_free (highlight_manager *manager)
{
  if (!manager)
    return;
  tile_array_free (manager->tile_array);
  free (manager); // Free the manager itself
}

// Clear all highlights from the highlight manager
void
clear_highlights (highlight_manager *manager)
{
  if (!manager || !manager->tile_array)
    {
      return;
    }

  tile_array_clear (manager->tile_array);
}

highlight
highlight_create (Color color)
{
  highlight highlight = { color };
  return highlight;
}
