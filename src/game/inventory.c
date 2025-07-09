#include "../../include/game/inventory.h"
#include "../../include/grid/grid_types.h"
#include <stdlib.h>

inv_item_t *
create_inv_item (tile_t *tile)
{
  inv_item_t *item = malloc (sizeof (inv_item_t));
  item->tile = tile;
  return item;
}

void
inventory_set_index (inv_t *inv, int index)
{
  if (index >= 0 && index < inv->size)
    {
      inv->selected_index = index;
    }
}

inv_item_t *
inventory_create_item ()
{
  grid_cell_t cell = { .type = GRID_TYPE_HEXAGON, .coord = { 0, 0, 0 } };
  tile_t *tile = tile_create_random_ptr (cell);
  return create_inv_item (tile);
}

inv_item_t **
inventory_create_item_array (int size)
{
  inv_item_t **item_array = malloc (size * sizeof (inv_item_t *));
  for (int i = 0; i < size; i++)
    {
      item_array[i] = inventory_create_item ();
    }
  return item_array;
}

inv_t *
create_inventory (int size)
{
  inv_t *inventory = malloc (sizeof (inv_t));
  inventory->item_array = inventory_create_item_array (size);
  inventory->size = size;
  inventory->selected_index = -1;
  return inventory;
}

void
inventory_render (inv_t *inv)
{
  int screen_width = GetScreenWidth ();
  int screen_height = GetScreenHeight ();

  int slot_size = 64;
  int slot_padding = 12;
  int total_width = inv->size * slot_size + (inv->size - 1) * slot_padding;
  int start_x = (screen_width - total_width) / 2;
  int y = screen_height - slot_size - 24;

  for (int i = 0; i < inv->size; i++)
    {
      Rectangle slot = { start_x + i * (slot_size + slot_padding), y,
                         slot_size, slot_size };
      Color fill = (inv->item_array[i] && inv->item_array[i]->tile)
                       ? tile_get_color (inv->item_array[i]->tile)
                       : LIGHTGRAY;

      DrawRectangleRec (slot, fill);

      if (i == inv->selected_index)
        {
          DrawRectangleLinesEx (slot, 4, GOLD);
        }
      else
        {
          DrawRectangleLinesEx (slot, 2, DARKGRAY);
        }
    }
}

void
inventory_destroy_item (inv_item_t *item)
{
  if (item)
    {
      if (item->tile)
        {
          free (item->tile); // Assuming you have a tile_destroy function
        }
      free (item);
    }
}

void
free_inventory (inv_t *inv)
{
  for (int i = 0; i < inv->size; i++)
    {
      inventory_destroy_item (inv->item_array[i]);
    }
  free (inv->item_array);
  free (inv);
}
