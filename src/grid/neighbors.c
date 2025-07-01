#include "../../include/grid/neighbors.h"
#include <raymath.h>
#include <stdio.h> // for fprintf (or your preferred logging)

// Direction vectors
static const hex hex_directions[6]
    = { { 1, 0, -1 }, { 1, -1, 0 }, { 0, -1, 1 },
        { -1, 0, 1 }, { -1, 1, 0 }, { 0, 1, -1 } };

hex
get_direction_vector (enum hex_grid_direction direction)
{
  if (direction >= 0 && direction < 6)
    {
      return hex_directions[direction];
    }
  // Return a default invalid hex if the direction is out of range
  fprintf (stderr, "get_direction_vector: Invalid direction\n");
  return (hex){ 0, 0, 0 };
}

hex
hex_direction (int direction)
{
  if (direction < 0 || direction >= 6)
    {
      fprintf (stderr, "hex_direction: direction must be 0-5\n");
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
      fprintf (stderr, "hex_diagonal_neighbor: direction must be 0-5\n");
      return hex;
    }
  return hex_add (hex,
                  hex_directions[(direction + (direction % 2 == 0 ? 1 : -1))
                                 % 6]); // Corrected
}

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
