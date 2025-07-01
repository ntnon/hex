#include "utility.h"

// Random number generation
int
random_in_range (int min, int max)
{
  return min + rand () % (max - min + 1);
}

double
random_float_in_range (double min, double max)
{
  return min + (double)rand () / RAND_MAX * (max - min);
}

void
seed_random ()
{
  srand (time (NULL));
}

// Math utilities
int
clamp (int value, int min, int max)
{
  if (value < min)
    return min;
  if (value > max)
    return max;
  return value;
}

double
lerp (double a, double b, double t)
{
  return a + t * (b - a);
}
