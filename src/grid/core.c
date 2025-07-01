#include "../../include/grid/core.h"
#include <stdlib.h> // for malloc/free

hex
hex_create (int q, int r, int s)
{
  if (q + r + s != 0)
    {
      // Handle the error appropriately, e.g., return an invalid hex or log an
      // error.
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
