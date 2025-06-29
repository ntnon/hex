#include "tile.h"
#include <stdlib.h>

// tile operations
tile
tile_create (hex hex, tile_type type, int value)
{
  tile tile = { 0 };
  tile.hex = hex;
  tile.type = type;
  tile.value = value;
  // tile.extra_data = NULL;
  return tile;
}

tile
tile_create_empty (hex hex)
{
  return tile_create (hex, TILE_EMPTY, 0);
}

tile_array
tile_array_create (void)
{
  tile_array array = { 0 };
  array.capacity = 16;
  array.data = malloc (array.capacity * sizeof (tile));
  array.count = 0;
  return array;
}

void
tile_array_push (tile_array *array, tile tile)
{
  if (!array)
    return;

  if (array->count >= array->capacity)
    {
      array->capacity *= 2;
      array->data = realloc (array->data, array->capacity * sizeof (tile));
    }
  array->data[array->count++] = tile;
}

void
tile_array_clear (tile_array *tile_array)
{
  if (!tile_array)
    {
      return; // Do nothing if the array is NULL
    }

  tile_array->count = 0; // Reset the count
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
  array->count = 0;
  array->capacity = 0;
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
get_tile_color (tile_type type)
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
  DrawCircle (10, 10, 100, ORANGE);
  point corners[6];
  get_hex_corners (layout, tile.hex, corners);

  Vector2 vertices[6];
  for (int i = 0; i < 6; i++)
    {
      vertices[i] = (Vector2){ corners[i].x, corners[i].y };
    }

  Color tile_color = get_tile_color (tile.type);
  DrawTriangleFan (vertices, 6, tile_color);

  // Draw hex outline
  for (int i = 0; i < 6; i++)
    {
      int next = (i + 1) % 6;
      DrawLineV (vertices[i], vertices[next], BLACK);
    }

  // Draw tile value if non-zero
  if (tile.value != 0)
    {
      point center = hex_to_pixel (layout, tile.hex);
      const char *value_text = TextFormat ("%d", tile.value);
      DrawText (value_text, center.x - 5, center.y - 5, 12, WHITE);
    }
}

void
draw_tile_array (layout layout, tile_array *tiles)
{
  if (!tiles)
    return;

  for (int i = 0; i < tiles->count; i++)
    {
      draw_tile (layout, tiles->data[i]);
    }
}
