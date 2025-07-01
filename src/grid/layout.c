#include "../../include/grid/layout.h"
#include <math.h>
#include <stdio.h> // for fprintf

// layout operations
point
point_create (double x, double y)
{
  return (point){ x, y };
}

layout
layout_create (orientation orientation, point size, point origin)
{
  return (layout){ orientation, size, origin };
}

fractional_hex
fractional_hex_create (double q, double r, double s)
{
  if (round (q + r + s) != 0)
    {
      fprintf (stderr, "fractional_hex_create: q + r + s must sum to 0\n");
      return (fractional_hex){ 0, 0, 0 }; // or handle the error appropriately
    }
  return (fractional_hex){ q, r, s };
}

point
hex_to_pixel (layout layout, hex h)
{
  orientation M = layout.orientation;
  point size = layout.size;
  point origin = layout.origin;
  double x = (M.f0 * h.q + M.f1 * h.r) * size.x;
  double y = (M.f2 * h.q + M.f3 * h.r) * size.y;
  return (point){ x + origin.x + 0.000001,
                  y + origin.y + 0.000001 }; // Small offset to prevent gaps
}

fractional_hex
pixel_to_hex_fractional (layout layout, point p)
{
  orientation M = layout.orientation;
  point size = layout.size;
  point origin = layout.origin;
  point pt = { (p.x - origin.x) / size.x, (p.y - origin.y) / size.y };
  double q = M.b0 * pt.x + M.b1 * pt.y;
  double r = M.b2 * pt.x + M.b3 * pt.y;
  return (fractional_hex){ q, r, -q - r };
}

hex
hex_round (fractional_hex h)
{
  int qi = (int)round (h.q);
  int ri = (int)round (h.r);
  int si = (int)round (h.s);
  double q_diff = fabs (qi - h.q);
  double r_diff = fabs (ri - h.r);
  double s_diff = fabs (si - h.s);

  if (q_diff > r_diff && q_diff > s_diff)
    {
      qi = -ri - si;
    }
  else if (r_diff > s_diff)
    {
      ri = -qi - si;
    }
  else
    {
      si = -qi - ri;
    }
  return (hex){ qi, ri, si };
}

hex
pixel_to_hex_rounded (layout layout, point p)
{
  return hex_round (pixel_to_hex_fractional (layout, p));
}

point
hex_corner_offset (layout layout, int corner)
{
  orientation M = layout.orientation;
  point size = layout.size;
  double angle = 2.0 * M_PI * (M.start_angle + corner)
                 / 6.0; // Corrected to work with get_hex_corners
  return (point){ size.x * cos (angle), size.y * sin (angle) };
}

void
get_hex_corners (layout layout, hex h, point corners[6])
{
  point center = hex_to_pixel (layout, h);
  for (int i = 0; i < 6; i++)
    {
      point offset = hex_corner_offset (layout, i);
      corners[i] = (point){ center.x + offset.x, center.y + offset.y };
    }
}

hex_array
polygon_corners (layout layout, hex h)
{
  hex_array corners = hex_array_create ();
  point center = hex_to_pixel (layout, h);

  for (int i = 0; i < 6; i++)
    {
      point offset = hex_corner_offset (layout, i);
      // Store corner points as hex coordinates for simplicity
      // In practice, you might want a pointArray type instead
      hex corner_as_hex
          = { (int)(center.x + offset.x), (int)(center.y + offset.y), 0 };
      hex_array_push (&corners, corner_as_hex);
    }
  return corners;
}

fractional_hex
hex_lerp (fractional_hex a, fractional_hex b, double t)
{
  return (fractional_hex){ a.q * (1.0 - t) + b.q * t,
                           a.r * (1.0 - t) + b.r * t,
                           a.s * (1.0 - t) + b.s * t };
}
