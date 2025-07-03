#ifndef GRID_SYSTEM_H
#define GRID_SYSTEM_H

#include "grid_types.h"

// Forward-declare the main grid struct to be used in the v-table definition
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
    void (*get_neighbor)(const grid_cell_t cell, int direction, grid_cell_t* out_neighbor);

    /**
     * @brief Calculates the grid-specific distance between two cells.
     * @param a The first cell.
     * @param b The second cell.
     * @return The distance in number of cells.
     */
    int (*distance)(grid_cell_t a, grid_cell_t b);

    /**
     * @brief Gets the pixel coordinates of a cell's corners.
     * @param grid The grid system instance.
     * @param cell The cell whose corners are to be calculated.
     * @param corners An array to be filled with the corner points. The size
     *                of the array must be appropriate for the grid type (e.g., 6 for hex).
     */
     void (*get_corners)(const grid_t *grid, grid_cell_t cell, point_t corners[]);

    /**
     * @brief Generates all the cells for a grid of a given size.
     * @param grid The grid system instance to populate.
     * @param size A generic size parameter (e.g., radius for hex, width/height for square).
     */
    void (*generate_cells)(grid_t* grid, int size);

    grid_t *(*grid_create)(grid_type_e type, layout_t layout, int size);
    void (*draw_grid)(const grid_t* grid);

} grid_vtable_t;

// The main grid object. This is the primary struct that game logic will interact with.
// It abstracts away the underlying grid implementation details.
struct grid_t {
    grid_type_e type;
    layout_t layout;

    // A pointer to the v-table containing the function implementations for this grid's type.
    const grid_vtable_t* vtable;

    // A dynamic array holding all the cells that make up this grid.
    grid_cell_t* cells;
    int cell_count;
    int cell_capacity;
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
grid_t* grid_create(grid_type_e type, layout_t layout, int size);

/**
 * @brief Frees all memory associated with a grid object.
 * @param grid A pointer to the grid_t object to be freed.
 */
void grid_free(grid_t* grid);

void draw_grid(const grid_t *grid);



/**
 * @brief The public instance of the v-table for hexagonal grids.
 *
 * This struct is initialized in hex_grid.c with pointers to the static
 * implementation functions. The grid_create() factory function uses this
* instance to construct a grid of type GRID_TYPE_HEXAGON.
 */
extern const grid_vtable_t hex_grid_vtable;

#endif // GRID_SYSTEM_H
