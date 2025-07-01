#ifndef UTILITY_H
#define UTILITY_H

#include <stdlib.h>
#include <time.h>

// Random number generation
int random_in_range(int min, int max);
double random_float_in_range(double min, double max);
void seed_random();

// Math utilities
int clamp(int value, int min, int max);
double lerp(double a, double b, double t);

#endif // UTILITY_H
