#include "../render/color.h"
#include "../tile/tile.h"
#include "../types.h"
#include "raylib.h"

Color
to_raylib_color (const color_t color);

color_t from_raylib_color(const color_t color);

color_t
tile_get_color (const tile_data_t tile_data);

Rectangle rect_to_ray_rectangle(const rect_t rect);
