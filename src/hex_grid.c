#include "hex_grid.h"
#include "raylib.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Constants
const int EVEN = 1;
const int ODD = -1;

// Predefined orientations
const orientation layout_pointy = { .f0 = 1.732050808,
                                    .f1 = 0.866025404,
                                    .f2 = 0.0,
                                    .f3 = 1.5,
                                    .b0 = 0.577350269,
                                    .b1 = -0.333333333,
                                    .b2 = 0.0,
                                    .b3 = 0.666666667,
                                    .start_angle = 0.5 };

const orientation layout_flat = { .f0 = 1.5,
                                  .f1 = 0.0,
                                  .f2 = 0.866025404,
                                  .f3 = 1.732050808,
                                  .b0 = 0.666666667,
                                  .b1 = 0.0,
                                  .b2 = -0.333333333,
                                  .b3 = 0.577350269,
                                  .start_angle = 0.0 };

// Direction vectors
static const hex hex_directions[6]
    = { { 1, 0, -1 }, { 1, -1, 0 }, { 0, -1, 1 },
        { -1, 0, 1 }, { -1, 1, 0 }, { 0, 1, -1 } };

static const hex hex_diagonals[6]
    = { { 2, -1, -1 }, { 1, -2, 1 },  { -1, -1, 2 },
        { -2, 1, 1 },  { -1, 2, -1 }, { 1, 1, -2 } };

// Function to get the direction vector for a given direction
hex
get_direction_vector (enum hex_grid_direction direction)
{
  if (direction >= 0 && direction < 6)
    {
      return hex_directions[direction];
    }
  // Return a default invalid hex if the direction is out of range
  return (hex){ 0, 0, 0 };
}

// Basic hex operations
hex
hex_create (int q, int r, int s)
{
  if (q + r + s != 0)
    {
      TraceLog (LOG_ERROR, "hex_create: q + r + s must be 0");
    }
  return (hex){ q, r, s };
}

hex
hex_add (hex a, hex b)
{
  return (hex){ a.q + b.q, a.r + b.r, a.s + b.s };
}

hex
hex_subtract (hex a, hex b)
{
  return (hex){ a.q - b.q, a.r - b.r, a.s - b.s };
}

hex
hex_scale (hex a, int k)
{
  return (hex){ a.q * k, a.r * k, a.s * k };
}

hex
hex_rotate_left (hex a)
{
  return (hex){ -a.s, -a.q, -a.r };
}

hex
hex_rotate_right (hex a)
{
  return (hex){ -a.r, -a.s, -a.q };
}

bool
hex_equal (hex a, hex b)
{
  return a.q == b.q && a.r == b.r && a.s == b.s;
}

// Directions and neighbors
hex
hex_direction (int direction)
{
  if (direction < 0 || direction >= 6)
    {
      TraceLog (LOG_ERROR, "hex_direction: direction must be 0-5");
      return (hex){ 0, 0, 0 };
    }
  return hex_directions[direction];
}

hex
hex_neighbor (hex hex, int direction)
{
  return hex_add (hex, hex_direction (direction));
}

hex
hex_diagonal_neighbor (hex hex, int direction)
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
hex_length (hex hex)
{
  return (abs (hex.q) + abs (hex.r) + abs (hex.s)) / 2;
}

int
hex_distance (hex a, hex b)
{
  return hex_length (hex_subtract (a, b));
}

// Fractional hex operations
fractional_hex
fractional_hex_create (double q, double r, double s)
{
  if (round (q + r + s) != 0)
    {
      TraceLog (LOG_ERROR, "fractional_hex_create: q + r + s must be 0");
    }
  return (fractional_hex){ q, r, s };
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

fractional_hex
hex_lerp (fractional_hex a, fractional_hex b, double t)
{
  return (fractional_hex){ a.q * (1.0 - t) + b.q * t,
                           a.r * (1.0 - t) + b.r * t,
                           a.s * (1.0 - t) + b.s * t };
}

// Line drawing
hex_array
hex_linedraw (hex a, hex b)
{
  int N = hex_distance (a, b);
  fractional_hex a_nudge = { a.q + 1e-06, a.r + 1e-06, a.s - 2e-06 };
  fractional_hex b_nudge = { b.q + 1e-06, b.r + 1e-06, b.s - 2e-06 };

  hex_array results = hex_array_create ();
  double step = 1.0 / fmax (N, 1);

  for (int i = 0; i <= N; i++)
    {
      fractional_hex lerp_result = hex_lerp (a_nudge, b_nudge, step * i);
      hex_array_push (&results, hex_round (lerp_result));
    }
  return results;
}

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

point
hex_to_pixel (layout layout, hex h)
{
  orientation M = layout.orientation;
  point size = layout.size;
  point origin = layout.origin;
  double x = (M.f0 * h.q + M.f1 * h.r) * size.x;
  double y = (M.f2 * h.q + M.f3 * h.r) * size.y;
  return (point){ x + origin.x, y + origin.y };
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
pixel_to_hex_rounded (layout layout, point p)
{
  return hex_round (pixel_to_hex_fractional (layout, p));
}

point
hex_corner_offset (layout layout, int corner)
{
  orientation M = layout.orientation;
  point size = layout.size;
  double angle = 2.0 * M_PI * (M.start_angle - corner) / 6.0;
  return (point){ size.x * cos (angle), size.y * sin (angle) };
}

void
get_hex_corners (layout layout, hex h, point corners[6])
{
  point center = hex_to_pixel (layout, h);
  for (int i = 0; i < 6; i++)
    {
      point offset = hex_corner_offset (layout, i);
      corners[i] = point_create (center.x + offset.x, center.y + offset.y);
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

// Offset coordinate conversions
offset_coord
qoffset_from_cube (int offset, hex h)
{
  if (offset != EVEN && offset != ODD)
    {
      TraceLog (LOG_ERROR, "offset must be EVEN (+1) or ODD (-1)");
    }
  int col = h.q;
  int row = h.r + (int)((h.q + offset * (h.q & 1)) / 2);
  return (offset_coord){ col, row };
}

hex
qoffset_to_cube (int offset, offset_coord h)
{
  if (offset != EVEN && offset != ODD)
    {
      TraceLog (LOG_ERROR, "offset must be EVEN (+1) or ODD (-1)");
    }
  int q = h.col;
  int r = h.row - (int)((h.col + offset * (h.col & 1)) / 2);
  int s = -q - r;
  return (hex){ q, r, s };
}

offset_coord
roffset_from_cube (int offset, hex h)
{
  if (offset != EVEN && offset != ODD)
    {
      TraceLog (LOG_ERROR, "offset must be EVEN (+1) or ODD (-1)");
    }
  int col = h.q + (int)((h.r + offset * (h.r & 1)) / 2);
  int row = h.r;
  return (offset_coord){ col, row };
}

hex
roffset_to_cube (int offset, offset_coord h)
{
  if (offset != EVEN && offset != ODD)
    {
      TraceLog (LOG_ERROR, "offset must be EVEN (+1) or ODD (-1)");
    }
  int q = h.col - (int)((h.row + offset * (h.row & 1)) / 2);
  int r = h.row;
  int s = -q - r;
  return (hex){ q, r, s };
}

// Doubled coordinate conversions
doubled_coord
qdoubled_from_cube (hex h)
{
  int col = h.q;
  int row = 2 * h.r + h.q;
  return (doubled_coord){ col, row };
}

hex
qdoubled_to_cube (doubled_coord h)
{
  int q = h.col;
  int r = (int)((h.row - h.col) / 2);
  int s = -q - r;
  return (hex){ q, r, s };
}

doubled_coord
rdoubled_from_cube (hex h)
{
  int col = 2 * h.q + h.r;
  int row = h.r;
  return (doubled_coord){ col, row };
}

hex
rdoubled_to_cube (doubled_coord h)
{
  int q = (int)((h.col - h.row) / 2);
  int r = h.row;
  int s = -q - r;
  return (hex){ q, r, s };
}

// Dynamic array operations
hex_array
hex_array_create (void)
{
  hex_array array = { 0 };
  array.capacity = 16;
  array.data = malloc (array.capacity * sizeof (hex));
  array.count = 0;
  return array;
}

void
hex_array_push (hex_array *array, hex hex)
{
  if (!array)
    return;

  if (array->count >= array->capacity)
    {
      array->capacity *= 2;
      array->data = realloc (array->data, array->capacity * sizeof (hex));
    }
  array->data[array->count++] = hex;
}

void
hex_array_free (hex_array *array)
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
print_hex (hex h)
{
  TraceLog (LOG_INFO, "hex(%d, %d, %d)", h.q, h.r, h.s);
}

void
print_point (point p)
{
  TraceLog (LOG_INFO, "point(%.2f, %.2f)", p.x, p.y);
}

// Edge drawing functions
void
draw_hex_edge (layout layout, hex hex1, hex hex2, float thickness, Color color)
{
  if (!are_hexes_adjacent (hex1, hex2))
    {
      TraceLog (LOG_WARNING, "draw_hex_edge: hexes are not adjacent");
      return;
    }

  point p1 = hex_to_pixel (layout, hex1);
  point p2 = hex_to_pixel (layout, hex2);
  Vector2 start = { p1.x, p1.y };
  Vector2 end = { p2.x, p2.y };
  DrawLineEx (start, end, thickness, color);
}

void
get_shared_edge_points (layout layout, hex hex1, hex hex2, point *edge_start,
                        point *edge_end)
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

  point corners1[6];
  get_hex_corners (layout, hex1, corners1);

  // The shared edge consists of two consecutive corners of hex1
  *edge_start = corners1[direction];
  *edge_end = corners1[(direction + 1) % 6];
}

void
draw_hex_shared_edge (layout layout, hex hex1, hex hex2, float thickness,
                      Color color)
{
  if (!are_hexes_adjacent (hex1, hex2))
    {
      TraceLog (LOG_WARNING, "draw_hex_shared_edge: hexes are not adjacent");
      return;
    }

  point edge_start, edge_end;
  get_shared_edge_points (layout, hex1, hex2, &edge_start, &edge_end);

  Vector2 start = { edge_start.x, edge_start.y };
  Vector2 end = { edge_end.x, edge_end.y };
  DrawLineEx (start, end, thickness, color);
}

void
draw_hex_edges_highlight (layout layout, hex center_hex, int direction_mask,
                          float thickness, Color color)
{
  for (int dir = 0; dir < 6; dir++)
    {
      if (direction_mask & (1 << dir))
        {
          hex neighbor = hex_neighbor (center_hex, dir);
          draw_hex_edge (layout, center_hex, neighbor, thickness, color);
        }
    }
}

void
draw_hex_shared_edges_highlight (layout layout, hex center_hex,
                                 int direction_mask, float thickness,
                                 Color color)
{
  for (int dir = 0; dir < 6; dir++)
    {
      if (direction_mask & (1 << dir))
        {
          hex neighbor = hex_neighbor (center_hex, dir);
          draw_hex_shared_edge (layout, center_hex, neighbor, thickness,
                                color);
        }
    }
}

bool
are_hexes_adjacent (hex hex1, hex hex2)
{
  int distance = hex_distance (hex1, hex2);
  return distance == 1;
}

int
get_edge_direction (hex from_hex, hex to_hex)
{
  if (!are_hexes_adjacent (from_hex, to_hex))
    {
      return -1; // Not adjacent
    }

  hex diff = hex_subtract (to_hex, from_hex);
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
draw_hex_ring_edges (layout layout, hex center, int radius, float thickness,
                     Color color)
{
  if (radius <= 0)
    return;

  // Get all hexes in the ring
  hex_array ring_hexes = hex_array_create ();

  // Start at one hex in the ring
  hex current = center;
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
draw_hex_path_edges (layout layout, hex_array path, float thickness,
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
draw_hex_border_edges (layout layout, hex_array hexes, float thickness,
                       Color color)
{
  if (hexes.count == 0)
    return;

  // For each hex in the array, check each of its 6 edges
  for (int i = 0; i < hexes.count; i++)
    {
      hex current = hexes.data[i];

      for (int dir = 0; dir < 6; dir++)
        {
          hex neighbor = hex_neighbor (current, dir);

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
              point corners[6];
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
hex_array
generate_hex_grid_radius (hex center, int radius)
{
  hex_array grid = hex_array_create ();

  // Add all hexes within radius distance from center
  for (int q = -radius; q <= radius; q++)
    {
      int r1 = fmax (-radius, -q - radius);
      int r2 = fmin (radius, -q + radius);
      for (int r = r1; r <= r2; r++)
        {
          int s = -q - r;
          hex hex = hex_add (center, hex_create (q, r, s));
          hex_array_push (&grid, hex);
        }
    }

  return grid;
}

hex_array
generate_hex_grid_rectangle (int width, int height)
{
  hex_array grid = hex_array_create ();

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

hex_array
generate_hex_grid_parallelogram (int q1, int q2, int r1, int r2)
{
  hex_array grid = hex_array_create ();

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
void
draw_hex (layout layout, hex h, float scale, Color color)
{
  point center_pt = hex_to_pixel (layout, h);
  Vector2 center = { center_pt.x, center_pt.y };

  point corners[6];
  get_hex_corners (layout, h, corners);

  Vector2 vertices[6];
  for (int i = 0; i < 6; i++)
    {
      // Scale each corner from the center
      float dx = (corners[i].x - center.x) * scale;
      float dy = (corners[i].y - center.y) * scale;
      vertices[i].x = center.x + dx;
      vertices[i].y = center.y + dy;
    }

  // Draw filled hexagon
  DrawTriangleFan (vertices, 6, color);

  // Optionally, draw the outline
  for (int i = 0; i < 6; i++)
    {
      int next = (i + 1) % 6;
      DrawLineV (vertices[i], vertices[next], BLACK);
    }
}

// Test functions (simplified versions of the C++ tests)
static void
test_hex_arithmetic (void)
{
  hex result = hex_add (hex_create (1, -3, 2), hex_create (3, -7, 4));
  hex expected = hex_create (4, -10, 6);
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
  hex result = hex_direction (2);
  hex expected = hex_create (0, -1, 1);
  if (!hex_equal (result, expected))
    {
      TraceLog (LOG_ERROR, "FAIL hex_direction");
    }
}

static void
test_hex_neighbor (void)
{
  hex result = hex_neighbor (hex_create (1, -2, 1), 2);
  hex expected = hex_create (1, -3, 2);
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
  hex h = hex_create (3, 4, -7);
  layout flat = layout_create (layout_flat, point_create (10.0, 15.0),
                               point_create (35.0, 71.0));
  point pixel = hex_to_pixel (flat, h);
  hex result = pixel_to_hex_rounded (flat, pixel);
  if (!hex_equal (h, result))
    {
      TraceLog (LOG_ERROR, "FAIL layout flat");
    }

  layout pointy = layout_create (layout_pointy, point_create (10.0, 15.0),
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
  TraceLog (LOG_INFO, "hex grid tests completed");
}
