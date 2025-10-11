#ifndef GRID_GEOMETRY_H
#define GRID_GEOMETRY_H

#include "grid_types.h"
#include <stdlib.h>
#include <stdbool.h>

// Forward declarations
typedef struct grid_vtable_t grid_vtable_t;

/**
 * @brief Virtual function table for grid geometry implementations.
 *
 * This table defines the interface that all grid types must implement.
 * All functions work with pure mathematical concepts - no instance state,
 * no radius limits, no growth tracking.
 */
struct grid_vtable_t {
    /**
     * @brief Converts a grid cell to pixel coordinates.
     * @param layout The layout configuration for coordinate conversion.
     * @param cell The grid cell to convert.
     * @return The pixel coordinates of the cell center.
     */
    point_t (*cell_to_pixel)(const layout_t* layout, grid_cell_t cell);

    /**
     * @brief Converts pixel coordinates to the nearest grid cell.
     * @param layout The layout configuration for coordinate conversion.
     * @param p The pixel point to convert.
     * @return The grid cell at or nearest to the pixel point.
     */
    grid_cell_t (*pixel_to_cell)(const layout_t* layout, point_t p);

    /**
     * @brief Gets a neighbor cell in the specified direction.
     * @param cell The source cell.
     * @param direction The direction index (interpretation depends on grid type).
     * @param out_neighbor Output parameter for the neighbor cell.
     */
    void (*get_neighbor)(grid_cell_t cell, int direction, grid_cell_t* out_neighbor);

    /**
     * @brief Gets all neighbor cells.
     * @param cell The source cell.
     * @param out_neighbors Array to store neighbors (must be sized for grid type).
     */
    void (*get_all_neighbors)(grid_cell_t cell, grid_cell_t* out_neighbors);

    /**
     * @brief Gets all cells within a given range from a center cell.
     * @param center The center cell.
     * @param range The maximum distance from center.
     * @param out_cells Output array for cells (caller allocates).
     * @param out_count Output parameter for number of cells found.
     */
    void (*get_cells_in_range)(grid_cell_t center, int range,
                              grid_cell_t** out_cells, size_t* out_count);

    /**
     * @brief Rotates a cell around the origin.
     * @param cell The cell to rotate.
     * @param rotations Number of rotational steps (interpretation depends on grid type).
     * @param out_cell Output parameter for the rotated cell.
     * @return True if rotation is valid for this cell.
     */
    bool (*rotate_cell)(grid_cell_t cell, int rotations, grid_cell_t* out_cell);

    /**
     * @brief Calculates the distance between two cells.
     * @param a First cell.
     * @param b Second cell.
     * @return The distance in grid steps.
     */
    int (*distance)(grid_cell_t a, grid_cell_t b);

    /**
     * @brief Gets the corner points of a cell.
     * @param layout The layout configuration for coordinate conversion.
     * @param cell The cell.
     * @param corners Output array for corner points (must be sized for grid type).
     */
    void (*get_corners)(const layout_t* layout, grid_cell_t cell, point_t* corners);

    /**
     * @brief Calculates offset between two cells.
     * @param from Source cell.
     * @param to Destination cell.
     * @return The offset as a grid cell (represents direction and magnitude).
     */
    grid_cell_t (*calculate_offset)(grid_cell_t from, grid_cell_t to);

    /**
     * @brief Applies an offset to a cell.
     * @param cell The base cell.
     * @param offset The offset to apply.
     * @return The resulting cell after applying the offset.
     */
    grid_cell_t (*apply_offset)(grid_cell_t cell, grid_cell_t offset);

    /**
     * @brief Gets the origin cell for this grid type.
     * @return The cell at coordinates (0,0) or equivalent.
     */
    grid_cell_t (*get_origin)(void);

    /**
     * @brief Gets all cells in a ring at a specific distance from center.
     * @param center The center cell.
     * @param radius The exact distance from center.
     * @param out_cells Output array for cells (caller allocates).
     * @param out_count Output parameter for number of cells found.
     */
    void (*get_ring)(grid_cell_t center, int radius,
                    grid_cell_t** out_cells, size_t* out_count);

    /**
     * @brief Gets cells along a line between two cells.
     * @param start Starting cell.
     * @param end Ending cell.
     * @param out_cells Output array for cells (caller allocates).
     * @param out_count Output parameter for number of cells found.
     */
    void (*get_line)(grid_cell_t start, grid_cell_t end,
                    grid_cell_t** out_cells, size_t* out_count);

    /**
     * @brief Checks if two cells are equal.
     * @param a First cell.
     * @param b Second cell.
     * @return True if cells have the same coordinates.
     */
    bool (*cells_equal)(grid_cell_t a, grid_cell_t b);

    /**
     * @brief Generates mesh data for rendering a single cell.
     * @param layout The layout configuration for coordinate conversion.
     * @param vertices Output array for vertex positions.
     * @param indices Output array for triangle indices.
     * @param vertex_count Output parameter for number of vertices.
     * @param index_count Output parameter for number of indices.
     * @return True if mesh generation succeeded.
     */
    bool (*get_cell_mesh)(const layout_t* layout, float** vertices,
                         unsigned int** indices, size_t* vertex_count,
                         size_t* index_count);

    /**
     * @brief Number of neighbors for this grid type.
     */
    int neighbor_count;

    /**
     * @brief Number of corners/vertices for cells in this grid type.
     */
    int corner_count;

    /**
     * @brief Grid type identifier.
     */
    grid_type_e type;
};

// --- Public geometry functions ---
// These dispatch to the appropriate implementation based on grid_type

/**
 * @brief Gets the vtable for a specific grid type.
 * @param type The grid type.
 * @return Pointer to the vtable, or NULL if type is unknown.
 */
const grid_vtable_t* grid_geometry_get_vtable(grid_type_e type);

/**
 * @brief Converts a grid cell to pixel coordinates.
 */
point_t grid_geometry_cell_to_pixel(grid_type_e type, const layout_t* layout,
                                    grid_cell_t cell);

/**
 * @brief Converts pixel coordinates to a grid cell.
 */
grid_cell_t grid_geometry_pixel_to_cell(grid_type_e type, const layout_t* layout,
                                        point_t p);

/**
 * @brief Gets a neighbor cell in the specified direction.
 */
void grid_geometry_get_neighbor(grid_type_e type, grid_cell_t cell,
                                int direction, grid_cell_t* out_neighbor);

/**
 * @brief Gets all neighbor cells.
 */
void grid_geometry_get_all_neighbors(grid_type_e type, grid_cell_t cell,
                                     grid_cell_t* out_neighbors);

/**
 * @brief Gets all cells within range from a center cell.
 */
void grid_geometry_get_cells_in_range(grid_type_e type, grid_cell_t center,
                                      int range, grid_cell_t** out_cells,
                                      size_t* out_count);

/**
 * @brief Calculates distance between two cells.
 */
int grid_geometry_distance(grid_type_e type, grid_cell_t a, grid_cell_t b);

/**
 * @brief Gets corner points of a cell.
 */
void grid_geometry_get_corners(grid_type_e type, const layout_t* layout,
                               grid_cell_t cell, point_t* corners);

/**
 * @brief Rotates a cell around the origin.
 */
bool grid_geometry_rotate_cell(grid_type_e type, grid_cell_t cell,
                               int rotations, grid_cell_t* out_cell);

/**
 * @brief Gets the origin cell for a grid type.
 */
grid_cell_t grid_geometry_get_origin(grid_type_e type);

/**
 * @brief Calculates offset between two cells.
 */
grid_cell_t grid_geometry_calculate_offset(grid_type_e type, grid_cell_t from,
                                           grid_cell_t to);

/**
 * @brief Applies an offset to a cell.
 */
grid_cell_t grid_geometry_apply_offset(grid_type_e type, grid_cell_t cell,
                                       grid_cell_t offset);

/**
 * @brief Gets cells in a ring at exact distance from center.
 */
void grid_geometry_get_ring(grid_type_e type, grid_cell_t center, int radius,
                            grid_cell_t** out_cells, size_t* out_count);

/**
 * @brief Gets cells along a line between two cells.
 */
void grid_geometry_get_line(grid_type_e type, grid_cell_t start, grid_cell_t end,
                            grid_cell_t** out_cells, size_t* out_count);

/**
 * @brief Checks if two cells are equal.
 */
bool grid_geometry_cells_equal(grid_type_e type, grid_cell_t a, grid_cell_t b);

/**
 * @brief Gets mesh data for rendering a cell.
 */
bool grid_geometry_get_cell_mesh(grid_type_e type, const layout_t* layout,
                                 float** vertices, unsigned int** indices,
                                 size_t* vertex_count, size_t* index_count);

/**
 * @brief Gets the number of neighbors for a grid type.
 */
int grid_geometry_get_neighbor_count(grid_type_e type);

/**
 * @brief Gets the number of corners for cells in a grid type.
 */
int grid_geometry_get_corner_count(grid_type_e type);

// --- Utility functions for collections of cells ---

/**
 * @brief Calculates the diameter of a cell collection.
 * @param type The grid type.
 * @param cells Array of cells.
 * @param cell_count Number of cells.
 * @return Maximum distance between any two cells.
 */
int grid_geometry_calculate_diameter(grid_type_e type, grid_cell_t* cells,
                                     size_t cell_count);

/**
 * @brief Calculates the geometric center of a cell collection.
 * @param type The grid type.
 * @param cells Array of cells.
 * @param cell_count Number of cells.
 * @return The center cell (may not be in the collection).
 */
grid_cell_t grid_geometry_calculate_center(grid_type_e type, grid_cell_t* cells,
                                           size_t cell_count);

/**
 * @brief Counts external edges of a cell collection.
 * @param type The grid type.
 * @param cells Array of cells.
 * @param cell_count Number of cells.
 * @return Number of edges not shared with other cells in the collection.
 */
int grid_geometry_count_external_edges(grid_type_e type, grid_cell_t* cells,
                                       size_t cell_count);

/**
 * @brief Counts internal edges of a cell collection.
 * @param type The grid type.
 * @param cells Array of cells.
 * @param cell_count Number of cells.
 * @return Number of edges shared between cells in the collection.
 */
int grid_geometry_count_internal_edges(grid_type_e type, grid_cell_t* cells,
                                       size_t cell_count);

/**
 * @brief Calculates the smallest bounding box that contains all cells.
 * @param type The grid type.
 * @param layout The layout configuration for coordinate conversion.
 * @param cells Array of cells to calculate bounds for.
 * @param cell_count Number of cells.
 * @param out_min_x Output for minimum x coordinate.
 * @param out_min_y Output for minimum y coordinate.
 * @param out_max_x Output for maximum x coordinate.
 * @param out_max_y Output for maximum y coordinate.
 * @return True if bounds were calculated successfully, false otherwise.
 */
bool grid_geometry_calculate_bounds(grid_type_e type, const layout_t* layout,
                                    grid_cell_t* cells, size_t cell_count,
                                    float* out_min_x, float* out_min_y,
                                    float* out_max_x, float* out_max_y);

// --- Registration of grid implementations ---

/**
 * @brief Registers the hexagonal grid implementation.
 */
void grid_geometry_register_hex(void);

/**
 * @brief Registers the square grid implementation.
 */
void grid_geometry_register_square(void);

/**
 * @brief Registers the triangular grid implementation.
 */
void grid_geometry_register_triangle(void);

/**
 * @brief Initializes all available grid geometry implementations.
 */
void grid_geometry_init(void);

#endif // GRID_GEOMETRY_H
