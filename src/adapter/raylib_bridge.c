#include "adapter/raylib_bridge.h"

color_t
tile_get_color (const tile_data_t tile_data)
{
  switch (tile_data.type)
    {
    case TILE_MAGENTA:
      return M_MAGENTA;
    case TILE_CYAN:
      return M_SKYBLUE;
    case TILE_YELLOW:
      return M_GREEN;
    case TILE_EMPTY:
      return M_BLACK;
    default:
      return M_LIGHTGRAY; // Or any color you want for empty/unknown
    }
}

Color
to_raylib_color (const color_t color)
{
  return (Color){ color.r, color.g, color.b, color.a };
}

color_t
froraylib_color (const Color color)
{
  return (color_t){ color.r, color.g, color.b, color.a };
}

Rectangle
rect_to_ray_rectangle (const rect_t rect)
{
  Rectangle ray_rect = { rect.x, rect.y, rect.width, rect.height };
  return ray_rect;
}
