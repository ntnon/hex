#include "hexgrid.h"
#include "raylib.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Constants
const int EVEN = 1;
const int ODD = -1;

// Predefined orientations
const Orientation layout_pointy = { .f0 = 1.732050808,
                                    .f1 = 0.866025404,
                                    .f2 = 0.0,
                                    .f3 = 1.5,
                                    .b0 = 0.577350269,
                                    .b1 = -0.333333333,
                                    .b2 = 0.0,
                                    .b3 = 0.666666667,
                                    .start_angle = 0.5 };

const Orientation layout_flat = { .f0 = 1.5,
                                  .f1 = 0.0,
                                  .f2 = 0.866025404,
                                  .f3 = 1.732050808,
                                  .b0 = 0.666666667,
                                  .b1 = 0.0,
                                  .b2 = -0.333333333,
                                  .b3 = 0.577350269,
                                  .start_angle = 0.0 };

// Direction vectors
static const Hex hex_directions[6]
    = { { 1, 0, -1 }, { 1, -1, 0 }, { 0, -1, 1 },
        { -1, 0, 1 }, { -1, 1, 0 }, { 0, 1, -1 } };

static const Hex hex_diagonals[6]
    = { { 2, -1, -1 }, { 1, -2, 1 },  { -1, -1, 2 },
        { -2, 1, 1 },  { -1, 2, -1 }, { 1, 1, -2 } };

// Basic hex operations
Hex
hex_create (int q, int r, int s)
{
  if (q + r + s != 0)
    {
      TraceLog (LOG_ERROR, "hex_create: q + r + s must be 0");
    }
  return (Hex){ q, r, s };
}

Hex
hex_add (Hex a, Hex b)
{
  return (Hex){ a.q + b.q, a.r + b.r, a.s + b.s };
}

Hex
hex_subtract (Hex a, Hex b)
{
  return (Hex){ a.q - b.q, a.r - b.r, a.s - b.s };
}

Hex
hex_scale (Hex a, int k)
{
  return (Hex){ a.q * k, a.r * k, a.s * k };
}

Hex
hex_rotate_left (Hex a)
{
  return (Hex){ -a.s, -a.q, -a.r };
}

Hex
hex_rotate_right (Hex a)
{
  return (Hex){ -a.r, -a.s, -a.q };
}

bool
hex_equal (Hex a, Hex b)
{
  return a.q == b.q && a.r == b.r && a.s == b.s;
}

// Directions and neighbors
Hex
hex_direction (int direction)
{
  if (direction < 0 || direction >= 6)
    {
      TraceLog (LOG_ERROR, "hex_direction: direction must be 0-5");
      return (Hex){ 0, 0, 0 };
    }
  return hex_directions[direction];
}

Hex
hex_neighbor (Hex hex, int direction)
{
  return hex_add (hex, hex_direction (direction));
}

Hex
hex_diagonal_neighbor (Hex hex, int direction)
{
  if (direction < 0 || direction >= 6)
    {
      TraceLog (LOG_ERROR, "hex_diagonal_neighbor: direction must be 0-5");
      return hex;
    }
  return hex_add (hex, hex_diagonals[direction]);
}

// Distance and length
int
hex_length (Hex hex)
{
  return (abs (hex.q) + abs (hex.r) + abs (hex.s)) / 2;
}

int
hex_distance (Hex a, Hex b)
{
  return hex_length (hex_subtract (a, b));
}

// Fractional hex operations
FractionalHex
fractional_hex_create (double q, double r, double s)
{
  if (round (q + r + s) != 0)
    {
      TraceLog (LOG_ERROR, "fractional_hex_create: q + r + s must be 0");
    }
  return (FractionalHex){ q, r, s };
}

Hex
hex_round (FractionalHex h)
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
  return (Hex){ qi, ri, si };
}

FractionalHex
hex_lerp (FractionalHex a, FractionalHex b, double t)
{
  return (FractionalHex){ a.q * (1.0 - t) + b.q * t, a.r * (1.0 - t) + b.r * t,
                          a.s * (1.0 - t) + b.s * t };
}

// Line drawing
HexArray
hex_linedraw (Hex a, Hex b)
{
  int N = hex_distance (a, b);
  FractionalHex a_nudge = { a.q + 1e-06, a.r + 1e-06, a.s - 2e-06 };
  FractionalHex b_nudge = { b.q + 1e-06, b.r + 1e-06, b.s - 2e-06 };

  HexArray results = hex_array_create ();
  double step = 1.0 / fmax (N, 1);

  for (int i = 0; i <= N; i++)
    {
      FractionalHex lerp_result = hex_lerp (a_nudge, b_nudge, step * i);
      hex_array_push (&results, hex_round (lerp_result));
    }
  return results;
}

// Layout operations
Point
point_create (double x, double y)
{
  return (Point){ x, y };
}

Layout
layout_create (Orientation orientation, Point size, Point origin)
{
  return (Layout){ orientation, size, origin };
}

Point
hex_to_pixel (Layout layout, Hex h)
{
  Orientation M = layout.orientation;
  Point size = layout.size;
  Point origin = layout.origin;
  double x = (M.f0 * h.q + M.f1 * h.r) * size.x;
  double y = (M.f2 * h.q + M.f3 * h.r) * size.y;
  return (Point){ x + origin.x, y + origin.y };
}

FractionalHex
pixel_to_hex_fractional (Layout layout, Point p)
{
  Orientation M = layout.orientation;
  Point size = layout.size;
  Point origin = layout.origin;
  Point pt = { (p.x - origin.x) / size.x, (p.y - origin.y) / size.y };
  double q = M.b0 * pt.x + M.b1 * pt.y;
  double r = M.b2 * pt.x + M.b3 * pt.y;
  return (FractionalHex){ q, r, -q - r };
}

Hex
pixel_to_hex_rounded (Layout layout, Point p)
{
  return hex_round (pixel_to_hex_fractional (layout, p));
}

Point
hex_corner_offset (Layout layout, int corner)
{
  Orientation M = layout.orientation;
  Point size = layout.size;
  double angle = 2.0 * M_PI * (M.start_angle - corner) / 6.0;
  return (Point){ size.x * cos (angle), size.y * sin (angle) };
}

void
get_hex_corners (Layout layout, Hex h, Point corners[6])
{
  Point center = hex_to_pixel (layout, h);
  for (int i = 0; i < 6; i++)
    {
      Point offset = hex_corner_offset (layout, i);
      corners[i] = point_create (center.x + offset.x, center.y + offset.y);
    }
}

HexArray
polygon_corners (Layout layout, Hex h)
{
  HexArray corners = hex_array_create ();
  Point center = hex_to_pixel (layout, h);

  for (int i = 0; i < 6; i++)
    {
      Point offset = hex_corner_offset (layout, i);
      // Store corner points as hex coordinates for simplicity
      // In practice, you might want a PointArray type instead
      Hex corner_as_hex
          = { (int)(center.x + offset.x), (int)(center.y + offset.y), 0 };
      hex_array_push (&corners, corner_as_hex);
    }
  return corners;
}

// Offset coordinate conversions
OffsetCoord
qoffset_from_cube (int offset, Hex h)
{
  if (offset != EVEN && offset != ODD)
    {
      TraceLog (LOG_ERROR, "offset must be EVEN (+1) or ODD (-1)");
    }
  int col = h.q;
  int row = h.r + (int)((h.q + offset * (h.q & 1)) / 2);
  return (OffsetCoord){ col, row };
}

Hex
qoffset_to_cube (int offset, OffsetCoord h)
{
  if (offset != EVEN && offset != ODD)
    {
      TraceLog (LOG_ERROR, "offset must be EVEN (+1) or ODD (-1)");
    }
  int q = h.col;
  int r = h.row - (int)((h.col + offset * (h.col & 1)) / 2);
  int s = -q - r;
  return (Hex){ q, r, s };
}

OffsetCoord
roffset_from_cube (int offset, Hex h)
{
  if (offset != EVEN && offset != ODD)
    {
      TraceLog (LOG_ERROR, "offset must be EVEN (+1) or ODD (-1)");
    }
  int col = h.q + (int)((h.r + offset * (h.r & 1)) / 2);
  int row = h.r;
  return (OffsetCoord){ col, row };
}

Hex
roffset_to_cube (int offset, OffsetCoord h)
{
  if (offset != EVEN && offset != ODD)
    {
      TraceLog (LOG_ERROR, "offset must be EVEN (+1) or ODD (-1)");
    }
  int q = h.col - (int)((h.row + offset * (h.row & 1)) / 2);
  int r = h.row;
  int s = -q - r;
  return (Hex){ q, r, s };
}

// Doubled coordinate conversions
DoubledCoord
qdoubled_from_cube (Hex h)
{
  int col = h.q;
  int row = 2 * h.r + h.q;
  return (DoubledCoord){ col, row };
}

Hex
qdoubled_to_cube (DoubledCoord h)
{
  int q = h.col;
  int r = (int)((h.row - h.col) / 2);
  int s = -q - r;
  return (Hex){ q, r, s };
}

DoubledCoord
rdoubled_from_cube (Hex h)
{
  int col = 2 * h.q + h.r;
  int row = h.r;
  return (DoubledCoord){ col, row };
}

Hex
rdoubled_to_cube (DoubledCoord h)
{
  int q = (int)((h.col - h.row) / 2);
  int r = h.row;
  int s = -q - r;
  return (Hex){ q, r, s };
}

// Dynamic array operations
HexArray
hex_array_create (void)
{
  HexArray array = { 0 };
  array.capacity = 16;
  array.data = malloc (array.capacity * sizeof (Hex));
  array.count = 0;
  return array;
}

void
hex_array_push (HexArray *array, Hex hex)
{
  if (!array)
    return;

  if (array->count >= array->capacity)
    {
      array->capacity *= 2;
      array->data = realloc (array->data, array->capacity * sizeof (Hex));
    }
  array->data[array->count++] = hex;
}

void
hex_array_free (HexArray *array)
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

// Utility functions
void
print_hex (Hex h)
{
  TraceLog (LOG_INFO, "Hex(%d, %d, %d)", h.q, h.r, h.s);
}

void
print_point (Point p)
{
  TraceLog (LOG_INFO, "Point(%.2f, %.2f)", p.x, p.y);
}

// Edge drawing functions
void
draw_hex_edge (Layout layout, Hex hex1, Hex hex2, float thickness, Color color)
{
  if (!are_hexes_adjacent (hex1, hex2))
    {
      TraceLog (LOG_WARNING, "draw_hex_edge: hexes are not adjacent");
      return;
    }

  Point p1 = hex_to_pixel (layout, hex1);
  Point p2 = hex_to_pixel (layout, hex2);
  Vector2 start = { p1.x, p1.y };
  Vector2 end = { p2.x, p2.y };
  DrawLineEx (start, end, thickness, color);
}

void
get_shared_edge_points (Layout layout, Hex hex1, Hex hex2, Point *edge_start,
                        Point *edge_end)
{
  if (!are_hexes_adjacent (hex1, hex2))
    {
      *edge_start = *edge_end = point_create (0, 0);
      return;
    }

  int direction = get_edge_direction (hex1, hex2);
  if (direction == -1)
    {
      *edge_start = *edge_end = point_create (0, 0);
      return;
    }

  Point corners1[6];
  get_hex_corners (layout, hex1, corners1);

  // The shared edge consists of two consecutive corners of hex1
  *edge_start = corners1[direction];
  *edge_end = corners1[(direction + 1) % 6];
}

void
draw_hex_shared_edge (Layout layout, Hex hex1, Hex hex2, float thickness,
                      Color color)
{
  if (!are_hexes_adjacent (hex1, hex2))
    {
      TraceLog (LOG_WARNING, "draw_hex_shared_edge: hexes are not adjacent");
      return;
    }

  Point edge_start, edge_end;
  get_shared_edge_points (layout, hex1, hex2, &edge_start, &edge_end);

  Vector2 start = { edge_start.x, edge_start.y };
  Vector2 end = { edge_end.x, edge_end.y };
  DrawLineEx (start, end, thickness, color);
}

void
draw_hex_edges_highlight (Layout layout, Hex center_hex, int direction_mask,
                          float thickness, Color color)
{
  for (int dir = 0; dir < 6; dir++)
    {
      if (direction_mask & (1 << dir))
        {
          Hex neighbor = hex_neighbor (center_hex, dir);
          draw_hex_edge (layout, center_hex, neighbor, thickness, color);
        }
    }
}

void
draw_hex_shared_edges_highlight (Layout layout, Hex center_hex,
                                 int direction_mask, float thickness,
                                 Color color)
{
  for (int dir = 0; dir < 6; dir++)
    {
      if (direction_mask & (1 << dir))
        {
          Hex neighbor = hex_neighbor (center_hex, dir);
          draw_hex_shared_edge (layout, center_hex, neighbor, thickness,
                                color);
        }
    }
}

bool
are_hexes_adjacent (Hex hex1, Hex hex2)
{
  int distance = hex_distance (hex1, hex2);
  return distance == 1;
}

int
get_edge_direction (Hex from_hex, Hex to_hex)
{
  if (!are_hexes_adjacent (from_hex, to_hex))
    {
      return -1; // Not adjacent
    }

  Hex diff = hex_subtract (to_hex, from_hex);
  for (int dir = 0; dir < 6; dir++)
    {
      if (hex_equal (diff, hex_direction (dir)))
        {
          return dir;
        }
    }
  return -1; // Should not reach here if hexes are adjacent
}

// Advanced edge pattern functions
void
draw_hex_ring_edges (Layout layout, Hex center, int radius, float thickness,
                     Color color)
{
  if (radius <= 0)
    return;

  // Get all hexes in the ring
  HexArray ring_hexes = hex_array_create ();

  // Start at one hex in the ring
  Hex current = center;
  for (int i = 0; i < radius; i++)
    {
      current = hex_neighbor (current, 4); // Move left
    }

  // Walk around the ring
  for (int dir = 0; dir < 6; dir++)
    {
      for (int i = 0; i < radius; i++)
        {
          hex_array_push (&ring_hexes, current);
          current = hex_neighbor (current, dir);
        }
    }

  // Draw edges between adjacent hexes in the ring
  for (int i = 0; i < ring_hexes.count; i++)
    {
      int next_i = (i + 1) % ring_hexes.count;
      draw_hex_edge (layout, ring_hexes.data[i], ring_hexes.data[next_i],
                     thickness, color);
    }

  hex_array_free (&ring_hexes);
}

void
draw_hex_path_edges (Layout layout, HexArray path, float thickness,
                     Color color)
{
  if (path.count < 2)
    return;

  for (int i = 0; i < path.count - 1; i++)
    {
      draw_hex_edge (layout, path.data[i], path.data[i + 1], thickness, color);
    }
}

void
draw_hex_border_edges (Layout layout, HexArray hexes, float thickness,
                       Color color)
{
  if (hexes.count == 0)
    return;

  // For each hex in the array, check each of its 6 edges
  for (int i = 0; i < hexes.count; i++)
    {
      Hex current = hexes.data[i];

      for (int dir = 0; dir < 6; dir++)
        {
          Hex neighbor = hex_neighbor (current, dir);

          // Check if the neighbor is NOT in our hex array
          bool neighbor_in_array = false;
          for (int j = 0; j < hexes.count; j++)
            {
              if (hex_equal (neighbor, hexes.data[j]))
                {
                  neighbor_in_array = true;
                  break;
                }
            }

          // If neighbor is not in array, this is a border edge
          if (!neighbor_in_array)
            {
              // Draw the edge segment for this border
              Point corners[6];
              get_hex_corners (layout, current, corners);

              // Draw the edge between corner[dir] and corner[(dir+1)%6]
              Vector2 start = { corners[dir].x, corners[dir].y };
              Vector2 end
                  = { corners[(dir + 1) % 6].x, corners[(dir + 1) % 6].y };
              DrawLineEx (start, end, thickness, color);
            }
        }
    }
}

// Grid generation functions
HexArray
generate_hex_grid_radius (Hex center, int radius)
{
  HexArray grid = hex_array_create ();

  // Add all hexes within radius distance from center
  for (int q = -radius; q <= radius; q++)
    {
      int r1 = fmax (-radius, -q - radius);
      int r2 = fmin (radius, -q + radius);
      for (int r = r1; r <= r2; r++)
        {
          int s = -q - r;
          Hex hex = hex_add (center, hex_create (q, r, s));
          hex_array_push (&grid, hex);
        }
    }

  return grid;
}

HexArray
generate_hex_grid_rectangle (int width, int height)
{
  HexArray grid = hex_array_create ();

  for (int r = 0; r < height; r++)
    {
      int r_offset = (int)floor (r / 2.0);
      for (int q = -r_offset; q < width - r_offset; q++)
        {
          int s = -q - r;
          hex_array_push (&grid, hex_create (q, r, s));
        }
    }

  return grid;
}

HexArray
generate_hex_grid_parallelogram (int q1, int q2, int r1, int r2)
{
  HexArray grid = hex_array_create ();

  for (int q = q1; q <= q2; q++)
    {
      for (int r = r1; r <= r2; r++)
        {
          int s = -q - r;
          hex_array_push (&grid, hex_create (q, r, s));
        }
    }

  return grid;
}

// Test functions (simplified versions of the C++ tests)
static void
test_hex_arithmetic (void)
{
  Hex result = hex_add (hex_create (1, -3, 2), hex_create (3, -7, 4));
  Hex expected = hex_create (4, -10, 6);
  if (!hex_equal (result, expected))
    {
      TraceLog (LOG_ERROR, "FAIL hex_add");
    }

  result = hex_subtract (hex_create (1, -3, 2), hex_create (3, -7, 4));
  expected = hex_create (-2, 4, -2);
  if (!hex_equal (result, expected))
    {
      TraceLog (LOG_ERROR, "FAIL hex_subtract");
    }
}

static void
test_hex_direction (void)
{
  Hex result = hex_direction (2);
  Hex expected = hex_create (0, -1, 1);
  if (!hex_equal (result, expected))
    {
      TraceLog (LOG_ERROR, "FAIL hex_direction");
    }
}

static void
test_hex_neighbor (void)
{
  Hex result = hex_neighbor (hex_create (1, -2, 1), 2);
  Hex expected = hex_create (1, -3, 2);
  if (!hex_equal (result, expected))
    {
      TraceLog (LOG_ERROR, "FAIL hex_neighbor");
    }
}

static void
test_hex_distance (void)
{
  int result = hex_distance (hex_create (3, -7, 4), hex_create (0, 0, 0));
  if (result != 7)
    {
      TraceLog (LOG_ERROR, "FAIL hex_distance");
    }
}

static void
test_layout (void)
{
  Hex h = hex_create (3, 4, -7);
  Layout flat = layout_create (layout_flat, point_create (10.0, 15.0),
                               point_create (35.0, 71.0));
  Point pixel = hex_to_pixel (flat, h);
  Hex result = pixel_to_hex_rounded (flat, pixel);
  if (!hex_equal (h, result))
    {
      TraceLog (LOG_ERROR, "FAIL layout flat");
    }

  Layout pointy = layout_create (layout_pointy, point_create (10.0, 15.0),
                                 point_create (35.0, 71.0));
  pixel = hex_to_pixel (pointy, h);
  result = pixel_to_hex_rounded (pointy, pixel);
  if (!hex_equal (h, result))
    {
      TraceLog (LOG_ERROR, "FAIL layout pointy");
    }
}

void
test_all (void)
{
  TraceLog (LOG_INFO, "Running hex grid tests...");
  test_hex_arithmetic ();
  test_hex_direction ();
  test_hex_neighbor ();
  test_hex_distance ();
  test_layout ();
  TraceLog (LOG_INFO, "Hex grid tests completed");
}
