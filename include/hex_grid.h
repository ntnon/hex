#ifndef HEXGRID_H
#define HEXGRID_H

#include "raylib.h"
#include <stdbool.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define MAX_HEX_ARRAY 1000

// point structure
typedef struct {
    double x;
    double y;
} point;

// hex coordinates (cube coordinates)
typedef struct {
    int q;
    int r;
    int s;
} hex;

// Fractional hex coordinates
typedef struct {
    double q;
    double r;
    double s;
} fractional_hex;

// Offset coordinates
typedef struct {
    int col;
    int row;
} offset_coord;

// Doubled coordinates
typedef struct {
    int col;
    int row;
} doubled_coord;

// orientation for layout
typedef struct {
    double f0, f1, f2, f3;  // forward matrix
    double b0, b1, b2, b3;  // backward matrix
    double start_angle;     // in multiples of 60°
} orientation;

// layout for hex-to-pixel conversion
typedef struct {
    orientation orientation;
    point size;
    point origin;
} layout;

// Dynamic array for hex coordinates
typedef struct {
    hex* data;
    int count;
    int capacity;
} hex_array;


// Enum for named directions
typedef enum hexGridDirection {
    NORTH,
    NORTHEAST,
    SOUTHEAST,
    SOUTH,
    SOUTHWEST,
    NORTHWEST,
    NONE,
    HEX_GRID_DIRECTION_COUNT
} hexGridDirection;

// Function to get the direction vector for a given direction
hex get_direction_vector(hexGridDirection direction);

// Constants for offset coordinates
extern const int EVEN;
extern const int ODD;

// Predefined orientations
extern const orientation layout_pointy;
extern const orientation layout_flat;

// Basic hex operations
hex hex_create(int q, int r, int s);
hex hex_add(hex a, hex b);
hex hex_subtract(hex a, hex b);
hex hex_scale(hex a, int k);
hex hex_rotate_left(hex a);
hex hex_rotate_right(hex a);
bool hex_equal(hex a, hex b);
hex hex_offset(hex a, int direction, int offset);

// Directions and neighbors
hex hex_direction(int direction);
hex hex_neighbor(hex hex, int direction);
hex hex_diagonal_neighbor(hex hex, int direction);

// Distance and length
int hex_length(hex hex);
int hex_distance(hex a, hex b);

// Fractional hex operations
fractional_hex fractional_hex_create(double q, double r, double s);
hex hex_round(fractional_hex h);
fractional_hex hex_lerp(fractional_hex a, fractional_hex b, double t);

// Line drawing
hex_array hex_linedraw(hex a, hex b);

// hex drawing
void draw_hex(layout layout, hex h, float scale, Color color);

// layout operations
point point_create(double x, double y);
layout layout_create(orientation orientation, point size, point origin);
point hex_to_pixel(layout layout, hex h);
fractional_hex pixel_to_hex_fractional(layout layout, point p);
hex pixel_to_hex_rounded(layout layout, point p);
point hex_corner_offset(layout layout, int corner);
void get_hex_corners(layout layout, hex h, point corners[6]);
hex_array polygon_corners(layout layout, hex h);

// Offset coordinate conversions
offset_coord qoffset_from_cube(int offset, hex h);
hex qoffset_to_cube(int offset, offset_coord h);
offset_coord roffset_from_cube(int offset, hex h);
hex roffset_to_cube(int offset, offset_coord h);

// Doubled coordinate conversions
doubled_coord qdoubled_from_cube(hex h);
hex qdoubled_to_cube(doubled_coord h);
doubled_coord rdoubled_from_cube(hex h);
hex rdoubled_to_cube(doubled_coord h);

// Dynamic array operations
hex_array hex_array_create(void);
void hex_array_push(hex_array* array, hex hex);
void hex_array_free(hex_array* array);

// Edge drawing functions
void draw_hex_edge(layout layout, hex hex1, hex hex2, float thickness, Color color);
void draw_hex_shared_edge(layout layout, hex hex1, hex hex2, float thickness, Color color);
void draw_hex_edges_highlight(layout layout, hex center_hex, int direction_mask, float thickness, Color color);
void draw_hex_shared_edges_highlight(layout layout, hex center_hex, int direction_mask, float thickness, Color color);
bool are_hexes_adjacent(hex hex1, hex hex2);
int get_edge_direction(hex from_hex, hex to_hex);
void get_shared_edge_points(layout layout, hex hex1, hex hex2, point* edge_start, point* edge_end);

// Advanced edge pattern functions
void draw_hex_ring_edges(layout layout, hex center, int radius, float thickness, Color color);
void draw_hex_path_edges(layout layout, hex_array path, float thickness, Color color);
void draw_hex_border_edges(layout layout, hex_array hexes, float thickness, Color color);

// Grid generation functions
hex_array generate_hex_grid_radius(hex center, int radius);
hex_array generate_hex_grid_rectangle(int width, int height);
hex_array generate_hex_grid_parallelogram(int q1, int q2, int r1, int r2);

// Utility functions
void print_hex(hex h);
void print_point(point p);

// Test functions
void test_all(void);

#endif // HEXGRID_H
