#ifndef GRID_TYPES_H
#define GRID_TYPES_H
#include "stddef.h"
#include <stdbool.h>
#include <limits.h>

// Forward declarations


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

/**
 * @brief Chunk identifier for coordinate-based chunking
 */
typedef struct {
    int chunk_x;    /* Chunk coordinate in primary axis */
    int chunk_y;    /* Chunk coordinate in secondary axis */
} chunk_id_t;

// Sentinel value for invalid chunk IDs (e.g., out-of-bounds coordinates)
#define INVALID_CHUNK_ID ((chunk_id_t){INT_MAX, INT_MAX})

/**
 * @brief Instance data for a single hex tile
 */
typedef struct hex_instance_t {
    float position[2];         /* World position (x, y) */
    float color[4];           /* RGBA color values */
} hex_instance_t;

/**
 * @brief Render data for instanced chunk rendering
 */
typedef struct chunk_render_data_t {
    hex_instance_t *instances; /* Instance data for tiles in chunk */
    size_t instance_count;     /* Number of tile instances */
    size_t instance_capacity;  /* Allocated instance capacity */
    unsigned int vbo_id;       /* OpenGL VBO for instance data */
    bool needs_rebuild;        /* Whether instance data needs rebuilding */
    bool gpu_buffer_valid;     /* Whether GPU buffer is up to date */
} chunk_render_data_t;

/**
 * @brief A chunk represents a spatial subdivision of the grid for efficient rendering
 */
typedef struct grid_chunk_t {
    chunk_id_t id;              /* Identifier for this chunk */
    bool dirty;                 /* Whether the chunk needs rebuilding */
    chunk_render_data_t *render_data; /* Cached rendering geometry */
    struct grid_chunk_t *next;  /* For hash table chaining */
} grid_chunk_t;

/**
 * @brief Coordinate array pool for memory reuse
 */
typedef struct coord_pool_entry {
    grid_cell_t *coords;
    size_t capacity;
    bool in_use;
    struct coord_pool_entry *next;
} coord_pool_entry_t;

/**
 * @brief Chunk management system for coordinate-based spatial subdivision
 */
typedef struct {
    int chunk_size;             /* Size of each chunk dimension (optimized for hex: 8-16 recommended) */
    grid_chunk_t **hash_table;  /* Hash table of chunk buckets */
    size_t hash_table_size;     /* Size of hash table (power of 2) */
    size_t num_chunks;          /* Number of active chunks */
    bool system_dirty;          /* Whether the chunk system needs rebuilding */
    coord_pool_entry_t *coord_pool; /* Pool of reusable coordinate arrays */

    /* Static hex mesh for instanced rendering */
    float *hex_vertices;        /* Canonical hex vertex data */
    unsigned int *hex_indices;  /* Canonical hex triangle indices */
    size_t hex_vertex_count;    /* Number of vertices in canonical hex */
    size_t hex_index_count;     /* Number of indices in canonical hex */
    unsigned int hex_vao_id;    /* VAO for hex mesh */
    unsigned int hex_vbo_id;    /* VBO for hex vertices */
    unsigned int hex_ebo_id;    /* EBO for hex indices */
    bool hex_mesh_initialized;  /* Whether static hex mesh is ready */
} chunk_system_t;




#endif // GRID_TYPES_H
