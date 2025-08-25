#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "raylib.h"
#include "third_party/clay.h"

/**
 * @brief Check if a point is within the given bounding box
 * @param point The point to check
 * @param bounds The bounding box to check against
 * @return true if point is within bounds, false otherwise
 */
bool point_in_bounds(Vector2 point, Clay_BoundingBox bounds);

#endif // GEOMETRY_H
