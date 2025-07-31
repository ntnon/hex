#include "../include/game/inventory.h"
#include "../include/third_party/kvec.h"
// #include "../../include/grid/grid_types.h"
#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>

tile_data_t
inventory_create_item (void)
{
  return tile_data_create_random ();
}

int
inventory_get_size (inventory_t *inv)
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
  if (index >= 0 && index < inventory_get_size (inv))
    {
      inv->selected_index = index;
    }
}

void
inventory_fill (inventory_t *inv)
{
  if (inv == NULL)
    {
      printf ("Inventory is NULL.\n");
      return; // Handle NULL inventory
    }

  for (int i = 0; i < inventory_get_size (inv); i++)
    {
      inv->items.a[i] = inventory_create_item ();
    }
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
  inventory_fill (inventory);

  inventory->selected_index = -1;

  // Fill the inventory with items

  return inventory;
}

void
inventory_destroy_item (inventory_t *inventory, int index)
{
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
  printf ("Inventory freed\n");
  if (!inv)
    return;
  kv_destroy (inv->items); // destroy the inventory items
  free (inv);              // <-- You were missing this
}

// Centralized slot rectangle calculation
Rectangle
inventory_get_slot_rect (inventory_t *inv, int i)
{
  int size = inventory_get_size (inv);
  int screen_width = GetScreenWidth ();
  int screen_height = GetScreenHeight ();
  int slot_size = 64;
  int slot_padding = 12;
  int total_width = size * slot_size + (size - 1) * slot_padding;
  int start_x = (screen_width - total_width) / 2;
  int y = screen_height - slot_size - 24;
  int x = start_x + i * (slot_size + slot_padding);
  return (Rectangle){ x, y, slot_size, slot_size };
}

// Render a single inventory item
void
inventory_render_item (inventory_t *inv, int i, bool selected)
{
  Rectangle slot = inventory_get_slot_rect (inv, i);
  Color fill = tile_get_color (inv->items.a[i]);

  DrawRectangleRec (slot, fill);

  if (selected)
    {
      DrawRectangleLinesEx (slot, 4, GOLD);
    }
  else
    {
      DrawRectangleLinesEx (slot, 2, DARKGRAY);
    }
}

// Render the entire inventory and handle click selection
void
inventory_render (inventory_t *inv)
{
  Vector2 mouse = GetMousePosition ();
  bool mouse_clicked = IsMouseButtonPressed (MOUSE_BUTTON_LEFT);

  for (int i = 0; i < inventory_get_size (inv); i++)
    {
      Rectangle slot = inventory_get_slot_rect (inv, i);

      // Handle click selection
      if (mouse_clicked && CheckCollisionPointRec (mouse, slot))
        {
          inv->selected_index = i;
        }

      inventory_render_item (inv, i, i == inv->selected_index);
    }
}

// Hit test: is mouse over any inventory slot?
bool
inventory_hit_test (inventory_t *inv, Vector2 mouse)
{
  for (int i = 0; i < inventory_get_size (inv); i++)
    {
      Rectangle slot = inventory_get_slot_rect (inv, i);
      if (CheckCollisionPointRec (mouse, slot))
        return true;
    }
  return false;
}

void
inventory_handle_input (inventory_t *inv)
{
  printf ("hi");
}
