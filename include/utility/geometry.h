#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "grid/grid_types.h"

typedef struct {
    float x, y, width, height;
} bounds_t;

/**
 * @brief Check if a point is within the given bounding box
 * @param point The point to check
 * @param bounds The bounding box to check against
 * @return true if point is within bounds, false otherwise
 */
bool point_in_bounds(point_t point, bounds_t bounds);

#endif // GEOMETRY_H
