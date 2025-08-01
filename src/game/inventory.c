#include "../include/game/inventory.h"
#include "../include/third_party/kvec.h"
// #include "../../include/grid/grid_types.h"
#include <stdio.h>
#include <stdlib.h>

tile_data_t
inventory_create_item (void)
{
  return tile_data_create_random ();
}

int
inventory_get_size (const inventory_t *inv)
{
  return inv->items.m;
}

// Set the selected index in the inventory
void
inventory_set_index (inventory_t *inv, int index)
{
  if (inv == NULL)
    {
      printf ("Inventory is NULL.\n");
      return; // Handle NULL inventory
    }
  if (index >= 0 && index < inventory_get_size (inv)
      && index != inv->selected_index)
    {
      inv->selected_index = index;
      return;
    }
  if (index == inv->selected_index)
    {
      inv->selected_index = -1;
      return;
    }
}

void
inventory_fill (inventory_t *inv, int size)
{
  if (inv == NULL)
    {
      printf ("Inventory is NULL.\n");
      return; // Handle NULL inventory
    }

  for (int i = 0; i < size; i++)
    {
      kv_push (tile_data_t, inv->items, inventory_create_item ());
    }

  printf ("inventory filled, %d items\n", size);
}

// Create the inventory
inventory_t *
inventory_create (int size)
{
  if (size <= 0)
    {
      printf ("Invalid inventory size.\n");
      return NULL; // Handle invalid size
    }

  inventory_t *inventory = malloc (sizeof (inventory_t));
  if (inventory == NULL)
    {
      printf ("Failed to allocate memory for inventory.\n");
      return NULL; // Handle memory allocation failure
    }

  kv_init (inventory->items);

  inventory->selected_index = -1;

  return inventory;
}

void
inventory_use_selected (inventory_t *inv)
{
  int index = inv->selected_index;
  if (!inv || index < 0 || index >= inventory_get_size (inv))
    return;

  // Remove the item
  kv_remove_at (tile_data_t, inv->items, index);

  // Adjust selected_index
  inv->selected_index = -1;
}

void
free_inventory (inventory_t *inv)
{

  if (!inv)
    {
      return;
    }
  kv_destroy (inv->items); // destroy the inventory items
  printf ("Inventory freed\n");
}
