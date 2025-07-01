#ifndef HEX_CORE_H
#define HEX_CORE_H

#include "raylib.h" // For Color, etc.

// point structure (already defined)
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

// orientation for layout
typedef struct {
    double f0, f1, f2, f3;  // forward matrix
    double b0, b1, b2, b3;  // backward matrix
    double start_angle;     // in multiples of 60°
} orientation;

// edge structure (defined by two points)
typedef struct {
    point p1;
    point p2;
} edge;

// dynamic array for edges
typedef struct {
    edge* edges;    // Array of edge structs
    int num_edges;  // Current number of edges
    int capacity;   // Allocated capacity
} edges;

// Basic hex operations
hex hex_create(int q, int r, int s);
hex hex_add(hex a, hex b);
hex hex_subtract(hex a, hex b);
hex hex_scale(hex a, int k);
hex hex_rotate_left(hex a);
hex hex_rotate_right(hex a);
bool hex_equal(hex a, hex b);

#endif
