#ifndef GRID_GEOMETRY_H
#define GRID_GEOMETRY_H

#include "grid_types.h"
#include <stdlib.h>

// Forward declarations
typedef struct grid_t grid_t;

// The "v-table" (Virtual Table) of grid operations.
// This struct holds function pointers to the concrete implementation for a
// specific grid tessellation (hex, square, etc.).
typedef struct {
    /**
     * @brief Converts a grid cell's coordinates to pixel coordinates.
     * @param grid The grid system instance.
     * @param cell The grid cell to convert.
     * @return The calculated pixel coordinate as a point_t.
     */
    point_t (*to_pixel)(const grid_t* grid, grid_cell_t cell);

    /**
     * @brief Converts pixel coordinates to the nearest grid cell.
     * @param grid The grid system instance.
     * @param p The pixel point to convert.
     * @return The closest grid cell.
     */
    grid_cell_t (*from_pixel)(const grid_t* grid, point_t p);

    /**
     * @brief Gets a neighboring cell in a specified direction.
     * @param cell The starting cell.
     * @param direction An integer representing the direction (e.g., 0-5 for hex).
     * @param out_neighbor Pointer to a grid_cell_t to store the result.
     */
    void (*get_neighbor_cell)(const grid_cell_t cell, int direction, grid_cell_t* out_neighbor);

    /**
     * @brief Gets the neighboring cells of a given cell.
     * @param cell The starting cell.
     * @param neighbors An array to be filled with the neighboring cells.
     */
    void (*get_neighbor_cells)(const grid_cell_t cell, grid_cell_t neighbors[]);

    /**
     * @brief Gets all coordinates within a specified range of a center coordinate.
     * @param grid The grid system instance.
     * @param center The center coordinate.
     * @param range The maximum distance from center (0 = just center, 1 = center + neighbors, etc).
     * @param out_cells Pointer to store the allocated array of cells.
     * @param out_count Pointer to store the number of cells.
     * @note Caller is responsible for freeing the allocated array.
     */
    void (*get_cells_in_range)(const grid_t* grid, grid_cell_t center, int range,
                              grid_cell_t** out_cells, size_t* out_count);


    /**
     * @brief Calculates the grid-specific distance between two cells.
     * @param a The first cell.
     * @param b The second cell.
     * @return The distance in number of cells.
     */
    int (*distance)(grid_cell_t a, grid_cell_t b);

    /**
     * @brief Gets the pixel coordinates of a cell's corners. It is very important that the order of the corners are clockwise. Otherwise, the rendering method will not work correctly.
     * @param grid The grid system instance.
     * @param cell The cell whose corners are to be calculated.
     * @param corners An array to be filled with the corner points. The size
     *                of the array must be appropriate for the grid type (e.g., 6 for hex).
     */
     void (*get_corners)(const grid_t *grid, grid_cell_t cell, point_t corners[]);

    /**
     * @brief Gets all valid cells in the grid.
     * @param grid The grid system instance.
     * @param out_cells Pointer to store the allocated array of cells.
     * @param out_count Pointer to store the number of cells.
     * @note Caller is responsible for freeing the allocated array.
     */
    void (*get_all_cells)(const grid_t* grid, grid_cell_t** out_cells, size_t* out_count);

    /**
     * @brief Gets the canonical hex mesh geometry for instanced rendering.
     * @param grid The grid system instance.
     * @param vertices Output buffer for vertex data (x, y for each vertex).
     * @param indices Output buffer for triangle indices.
     * @param vertex_count Output for number of vertices.
     * @param index_count Output for number of indices.
     * @return True if mesh data was successfully generated.
     */
    bool (*get_hex_mesh)(const grid_t* grid, float** vertices, unsigned int** indices,
                        size_t* vertex_count, size_t* index_count);

    grid_t *(*grid_create)(grid_type_e type, layout_t layout, int size);

    int num_neighbors;

    bool (*is_valid_cell)(const grid_t* grid, grid_cell_t check_cell);

    void(*grid_free)(grid_t* grid);

    /**
     * @brief Calculates offset between two cells.
     * @param target The target cell position.
     * @param source The source cell position.
     * @return The offset needed to transform source to target.
     */
    grid_cell_t (*calculate_offset)(grid_cell_t target, grid_cell_t source);

    /**
     * @brief Applies an offset to a cell.
     * @param cell The original cell.
     * @param offset The offset to apply.
     * @return The resulting cell after applying offset.
     */
    grid_cell_t (*apply_offset)(grid_cell_t cell, grid_cell_t offset);

    /**
     * @brief Gets the center cell of the grid.
     * @param grid The grid system instance.
     * @return The center cell of the grid.
     */
    grid_cell_t (*get_center_cell)(const grid_t* grid);

    /**
     * @brief Gets a pointer to the cell at the specified pixel coordinates.
     * @param grid The grid system instance.
     * @param p The pixel coordinates to check.
     * @return A pointer to the grid cell at the pixel coordinates, or NULL if outside the grid.
     */
    grid_cell_t* (*get_cell_at_pixel)(const grid_t* grid, point_t p);
    void (*print_cell)(const grid_t *grid, grid_cell_t cell);


} grid_vtable_t;

// The main grid object. This is the primary struct that game logic will interact with.
// It abstracts away the underlying grid implementation details.
struct grid_t {
    grid_type_e type;
    layout_t layout;
    const grid_vtable_t* vtable;
    int radius;  /* Mathematical bounds - for hex: max distance from origin */
    int initial_radius;  /* Original radius when grid was created */
    int total_growth;    /* Total amount grid has grown since creation */
};


//
// --- Public API ---
//

/**
 * @brief Factory function to create and initialize a grid of a specific type.
 * @param type The type of grid to create (e.g., GRID_TYPE_HEXAGON).
 * @param layout The layout defining cell size and orientation.
 * @param size A generic size parameter (e.g., radius).
 * @return A pointer to the newly created grid_t object, or NULL on failure.
 */
grid_t *grid_create(grid_type_e type, layout_t layout, int size);

/**
 * @brief Frees all memory associated with a grid object.
 * @param grid A pointer to the grid_t object to be freed.
 */
void grid_free(grid_t* grid);


bool is_valid_cell(const grid_t* grid, grid_cell_t check_cell);

/**
 * @brief Gets the center cell of the grid.
 * @param grid The grid system instance.
 * @return The center cell of the grid.
 */
grid_cell_t grid_get_center_cell(const grid_t* grid);

/**
 * @brief Gets a pointer to the cell at the specified pixel coordinates.
 * @param grid The grid system instance.
 * @param p The pixel coordinates to check.
 * @return A pointer to the grid cell at the pixel coordinates, or NULL if outside the grid.
 */
grid_cell_t* grid_get_cell_at_pixel(const grid_t* grid, point_t p);

void print_cell(const grid_t *grid, grid_cell_t cell);

/**
 * @brief Gets all valid cells in the grid.
 * @param grid The grid system instance.
 * @param out_cells Pointer to store the allocated array of cells.
 * @param out_count Pointer to store the number of cells.
 * @note Caller is responsible for freeing the allocated array.
 */
void grid_get_all_cells(const grid_t* grid, grid_cell_t** out_cells, size_t* out_count);

/**
 * @brief Gets all coordinates within a specified range of a center coordinate.
 * @param grid The grid system instance.
 * @param center The center coordinate.
 * @param range The maximum distance from center (0 = just center, 1 = center + neighbors, etc).
 * @param out_cells Pointer to store the allocated array of cells.
 * @param out_count Pointer to store the number of cells.
 * @note Caller is responsible for freeing the allocated array.
 */
void grid_get_cells_in_range(const grid_t* grid, grid_cell_t center, int range, 
                            grid_cell_t** out_cells, size_t* out_count);

/**
 * @brief Grows the grid by the specified amount.
 * @param grid The grid system instance.
 * @param growth_amount Amount to increase the grid radius by.
 * @return true if growth was successful, false otherwise.
 */
bool grid_grow(grid_t* grid, int growth_amount);

/**
 * @brief Gets the total amount the grid has grown since creation.
 * @param grid The grid system instance.
 * @return Total growth amount, or -1 if grid is NULL.
 */
int grid_get_total_growth(const grid_t* grid);

/**
 * @brief Gets the initial radius when the grid was created.
 * @param grid The grid system instance.
 * @return Initial radius, or -1 if grid is NULL.
 */
int grid_get_initial_radius(const grid_t* grid);

/**
 * @brief The public instance of the v-table for hexagonal grids.
 *
 * This struct is initialized in hex_grid.c with pointers to the static
 * implementation functions. The grid_create() factory function uses this
* instance to construct a grid of type GRID_TYPE_HEXAGON.
 */
extern const grid_vtable_t hex_grid_vtable;

#endif // grid_geometry_H
