
#include "utility/geometry.h"

bool point_in_bounds(Vector2 point, Clay_BoundingBox bounds) {
  return (point.x >= bounds.x && point.x <= bounds.x + bounds.width &&
          point.y >= bounds.y && point.y <= bounds.y + bounds.height);
}
