#ifndef HEX_NEIGHBORS_H
#define HEX_NEIGHBORS_H

#include "core.h"

// Enum for named directions
typedef enum hex_grid_direction {
    NORTH,
    NORTHEAST,
    SOUTHEAST,
    SOUTH,
    SOUTHWEST,
    NORTHWEST,
    NONE,
    HEX_GRID_DIRECTION_COUNT
} hex_grid_direction;

// Function to get the direction vector for a given direction
hex get_direction_vector(hex_grid_direction direction);

// Directions and neighbors
hex hex_direction(int direction);
hex hex_neighbor(hex hex, int direction);
hex hex_diagonal_neighbor(hex hex, int direction);

// Distance and length
int hex_length(hex hex);
int hex_distance(hex a, hex b);

#endif
