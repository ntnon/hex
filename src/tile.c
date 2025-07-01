#include "tile.h"
#include <stdio.h>
#include <stdlib.h>

// tile operations
tile
tile_create (hex hex, tile_type type, int value)
{
  tile tile = { 0 };
  tile.hex = hex;
  tile.type = type;
  tile.value = value;
  return tile;
}

tile
tile_create_empty (hex hex)
{
  return tile_create (hex, TILE_EMPTY, 0);
}

tile_array *
tile_array_create (void)
{
  // Dynamically allocate memory for the tile_array structure
  tile_array *array = malloc (sizeof (tile_array));
  if (!array)
    {
      printf (stderr, "Failed to allocate memory for tile_array.\n");
      return NULL;
    }

  // Initialize the fields of the tile_array
  array->capacity = 16;
  array->data = malloc (array->capacity * sizeof (tile));
  if (!array->data)
    {
      fprintf (stderr, "Failed to allocate memory for tile_array data.\n");
      free (array); // Free the structure itself if data allocation fails
      return NULL;
    }
  array->count = 0;

  return array;
}

void
tile_array_push (tile_array *array, tile tile)
{
  if (!array)
    return;

  if (array->count >= array->capacity)
    {
      void *new_data
          = realloc (array->data, array->capacity * 2 * sizeof (tile));
      if (!new_data)
        {
          fprintf (stderr, "Failed to reallocate memory for tile_array.\n");
          return;
        }
      array->data = new_data;
      array->capacity *= 2;
    }
  array->data[array->count++] = tile;
}

void
tile_array_free (tile_array *array)
{
  if (!array)
    return;
  if (array->data)
    {
      free (array->data);
      array->data = NULL;
    }
  free (array); // Free the structure itself
}

void
tile_array_clear (tile_array *array)
{
  if (!array)
    {
      return; // Do nothing if the array is NULL
    }

  // Reset the count to 0, effectively "clearing" the array
  array->count = 0;
}

tile *
find_tile_by_coord (tile_array *array, hex hex)
{
  if (!array)
    return NULL;

  for (int i = 0; i < array->count; i++)
    {
      if (hex_equal (array->data[i].hex, hex))
        {
          return &array->data[i];
        }
    }
  return NULL;
}

// tile properties
Color
get_tile_type_color (tile_type type)
{
  switch (type)
    {
    case TILE_EMPTY:
      return LIGHTGRAY;
    case TILE_MAGENTA:
      return MAGENTA;
    case TILE_CYAN:
      return SKYBLUE;
    case TILE_YELLOW:
      return YELLOW;
    default:
      return WHITE;
    }
}

// tile rendering
void
draw_tile (layout layout, tile tile)
{
  // Use draw_hex to draw the hexagon with the tile's color
  Color tile_color = get_tile_type_color (tile.type);
  draw_hex (layout, tile.hex, 0.9f,
            tile_color); // Scale is 1.0 for normal size

  // Draw tile value if non-zero
  if (tile.value != 0)
    {
      point center = hex_to_pixel (layout, tile.hex);
      const char *value_text = TextFormat ("%d", tile.value);
      DrawText (value_text, center.x - 3.5, center.y - 5, 6, BLACK);
    }
}

void
draw_tile_array (layout layout, tile_array *tiles)
{
  if (!tiles)
    return;

  for (int i = 0; i < tiles->count; i++)
    {
      if (tiles->data[i].type == TILE_EMPTY)
        continue; // skip drawing empty tiles
      draw_tile (layout, tiles->data[i]);
    }
}

tile
tile_create_random (hex hex)
{
  int value = (rand () % 5) + 1;
  tile_type type = (tile_type)(rand () % TILE_TYPE_COUNT);
  return tile_create (hex, type, value);
}

const char *
get_tile_type_name (tile_type type)
{
  switch (type)
    {
    case TILE_EMPTY:
      return "Empty";
    case TILE_MAGENTA:
      return "Magenta";
    case TILE_CYAN:
      return "Cyan";
    case TILE_YELLOW:
      return "Yellow";
    default:
      return "Unknown";
    }
}

tile *
get_tile_by_hex (const tile_array *tile_array, hex hex)
{
  if (!tile_array)
    {
      return NULL; // Return NULL if board or tile_array is NULL
    }

  for (int i = 0; i < tile_array->count; i++)
    {
      if (hex_equal (tile_array->data[i].hex, hex))
        {
          return &tile_array->data[i];
        }
    }
  return NULL;
}

int
get_tile_index (const tile_array *tile_array, tile *tile)
{
  if (!tile_array || !tile)
    {
      return -1; // Return -1 if board or tile is NULL
    }

  for (int i = 0; i < tile_array->count; i++)
    {
      if (&tile_array->data[i] == tile)
        {
          return i;
        }
    }
  return -1;
}

tile *
get_tile_by_index (const tile_array *tile_array, int index)
{
  if (!tile_array || index < 0 || index >= tile_array->count)
    {
      return NULL; // Return NULL if board or tile_array is NULL
    }

  return &tile_array->data[index];
}

void
tile_array_set_tile (tile_array *tile_array, int index, tile tile)
{
  if (!tile_array)
    {
      printf ("Tile array is NULL\n");
      return;
    }
  tile_array->data[index] = tile;
}

void
tile_array_cycle_tile (tile_array *tile_array, int index)
{
  if (!tile_array)
    {
      printf ("Tile array is NULL\n");
      return;
    }
  tile_array->data[index].type
      = (tile_array->data[index].type + 1) % TILE_TYPE_COUNT;
}

void
tile_cycle (tile *tile)
{
  if (!tile)
    {
      printf ("Tile is NULL\n");
      return;
    }

  // Cycle the tile type
  tile->type = (tile->type + 1) % TILE_TYPE_COUNT;
}
