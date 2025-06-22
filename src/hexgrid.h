#ifndef HEXGRID_H
#define HEXGRID_H

#include "raylib.h"
#include <stdbool.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define MAX_HEX_ARRAY 1000

// Point structure
typedef struct {
    double x;
    double y;
} Point;

// Hex coordinates (cube coordinates)
typedef struct {
    int q;
    int r; 
    int s;
} Hex;

// Fractional hex coordinates
typedef struct {
    double q;
    double r;
    double s;
} FractionalHex;

// Offset coordinates
typedef struct {
    int col;
    int row;
} OffsetCoord;

// Doubled coordinates
typedef struct {
    int col;
    int row;
} DoubledCoord;

// Orientation for layout
typedef struct {
    double f0, f1, f2, f3;  // forward matrix
    double b0, b1, b2, b3;  // backward matrix
    double start_angle;     // in multiples of 60°
} Orientation;

// Layout for hex-to-pixel conversion
typedef struct {
    Orientation orientation;
    Point size;
    Point origin;
} Layout;

// Dynamic array for hex coordinates
typedef struct {
    Hex* data;
    int count;
    int capacity;
} HexArray;

// Constants for offset coordinates
extern const int EVEN;
extern const int ODD;

// Predefined orientations
extern const Orientation layout_pointy;
extern const Orientation layout_flat;

// Basic hex operations
Hex hex_create(int q, int r, int s);
Hex hex_add(Hex a, Hex b);
Hex hex_subtract(Hex a, Hex b);
Hex hex_scale(Hex a, int k);
Hex hex_rotate_left(Hex a);
Hex hex_rotate_right(Hex a);
bool hex_equal(Hex a, Hex b);

// Directions and neighbors
Hex hex_direction(int direction);
Hex hex_neighbor(Hex hex, int direction);
Hex hex_diagonal_neighbor(Hex hex, int direction);

// Distance and length
int hex_length(Hex hex);
int hex_distance(Hex a, Hex b);

// Fractional hex operations
FractionalHex fractional_hex_create(double q, double r, double s);
Hex hex_round(FractionalHex h);
FractionalHex hex_lerp(FractionalHex a, FractionalHex b, double t);

// Line drawing
HexArray hex_linedraw(Hex a, Hex b);

// Layout operations
Point point_create(double x, double y);
Layout layout_create(Orientation orientation, Point size, Point origin);
Point hex_to_pixel(Layout layout, Hex h);
FractionalHex pixel_to_hex_fractional(Layout layout, Point p);
Hex pixel_to_hex_rounded(Layout layout, Point p);
Point hex_corner_offset(Layout layout, int corner);
void get_hex_corners(Layout layout, Hex h, Point corners[6]);
HexArray polygon_corners(Layout layout, Hex h);

// Offset coordinate conversions
OffsetCoord qoffset_from_cube(int offset, Hex h);
Hex qoffset_to_cube(int offset, OffsetCoord h);
OffsetCoord roffset_from_cube(int offset, Hex h);
Hex roffset_to_cube(int offset, OffsetCoord h);

// Doubled coordinate conversions
DoubledCoord qdoubled_from_cube(Hex h);
Hex qdoubled_to_cube(DoubledCoord h);
DoubledCoord rdoubled_from_cube(Hex h);
Hex rdoubled_to_cube(DoubledCoord h);

// Dynamic array operations
HexArray hex_array_create(void);
void hex_array_push(HexArray* array, Hex hex);
void hex_array_free(HexArray* array);

// Edge drawing functions
void draw_hex_edge(Layout layout, Hex hex1, Hex hex2, float thickness, Color color);
void draw_hex_shared_edge(Layout layout, Hex hex1, Hex hex2, float thickness, Color color);
void draw_hex_edges_highlight(Layout layout, Hex center_hex, int direction_mask, float thickness, Color color);
void draw_hex_shared_edges_highlight(Layout layout, Hex center_hex, int direction_mask, float thickness, Color color);
bool are_hexes_adjacent(Hex hex1, Hex hex2);
int get_edge_direction(Hex from_hex, Hex to_hex);
void get_shared_edge_points(Layout layout, Hex hex1, Hex hex2, Point* edge_start, Point* edge_end);

// Advanced edge pattern functions
void draw_hex_ring_edges(Layout layout, Hex center, int radius, float thickness, Color color);
void draw_hex_path_edges(Layout layout, HexArray path, float thickness, Color color);
void draw_hex_border_edges(Layout layout, HexArray hexes, float thickness, Color color);

// Grid generation functions
HexArray generate_hex_grid_radius(Hex center, int radius);
HexArray generate_hex_grid_rectangle(int width, int height);
HexArray generate_hex_grid_parallelogram(int q1, int q2, int r1, int r2);

// Utility functions
void print_hex(Hex h);
void print_point(Point p);

// Test functions
void test_all(void);

#endif // HEXGRID_H