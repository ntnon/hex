#ifndef HEX_LAYOUT_H
#define HEX_LAYOUT_H

#include "core.h"
#include "raylib.h" // For Color, etc.

// layout for hex-to-pixel conversion
typedef struct {
    orientation orientation;
    point size;
    point origin;
} layout;

// layout operations
point point_create(double x, double y);
layout layout_create(orientation orientation, point size, point origin);
point hex_to_pixel(layout layout, hex h);
fractional_hex pixel_to_hex_fractional(layout layout, point p);
hex hex_round(fractional_hex h);
hex pixel_to_hex_rounded(layout layout, point p);
point hex_corner_offset(layout layout, int corner);
void get_hex_corners(layout layout, hex h, point corners[6]);
hex_array polygon_corners(layout layout, hex h);
fractional_hex hex_lerp(fractional_hex a, fractional_hex b, double t);

#endif
