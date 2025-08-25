#ifndef GRID_TYPES_H
#define GRID_TYPES_H

#include <stdbool.h>

/**
 * @brief An enum to identify the grid tessellation type
 */
typedef enum {
    GRID_TYPE_SQUARE,
    GRID_TYPE_HEXAGON,
    GRID_TYPE_TRIANGLE,
    GRID_TYPE_UNKNOWN
} grid_type_e;

/**
 * @brief Coordinate system definitions for each grid type
 */

/**
 * @brief Simple cartesian coordinates for square grids
 */
typedef struct {
    int x;
    int y;
} square_coord_t;

/**
 * @brief Cube coordinates for hexagonal grids
 */
typedef struct {
    int q;
    int r;
    int s;
} hex_coord_t;

/**
 * @brief Example coordinate system for triangular grids
 */
typedef struct {
    int u;
    int v;
    int w;
} triangle_coord_t;

/**
 * @brief Generic grid cell structure
 * A generic cell that can hold coordinates for any grid type.
 * The 'type' field is a discriminator for the union.
 */
typedef struct {
    grid_type_e type;
    union {
        square_coord_t square;
        hex_coord_t hex;
        triangle_coord_t triangle;
    } coord;
} grid_cell_t;

/**
 * @brief Layout and Pixel Conversion structures
 */

/**
 * @brief Represents a 2D point, typically for pixel coordinates
 */
typedef struct {
    double x;
    double y;
} point_t;

typedef struct {
    point_t a;
    point_t b;
} grid_edge_t;

/**
 * @brief Defines the matrix and angle for converting grid coordinates to pixel space
 */
typedef struct {
    double f0, f1, f2, f3;  /* Forward matrix (grid to pixel) */
    double b0, b1, b2, b3;  /* Backward matrix (pixel to grid) */
    double start_angle;     /* Orientation angle in multiples of 60 degrees for hexes */
} orientation_t;

/**
 * @brief Holds all layout information for a grid needed for coordinate conversion
 */
typedef struct {
    orientation_t orientation;
    point_t size;           /* Size of a single cell (e.g., width and height) */
    point_t origin;         /* Pixel offset for the grid's origin (0,0) */
    double scale;           /* Scale multiplier for the entire grid (1.0 = normal, 2.0 = double size) */
} layout_t;


#endif // GRID_TYPES_H
