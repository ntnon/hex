#ifndef HEX_GRID_H
#define HEX_GRID_H

#include "grid_types.h"
#include "grid_system.h"

//
// --- Hexagonal Grid Implementation ---
//
// These are the concrete implementations of the grid operations for a
// hexagonal tessellation. These functions will be assigned to the function
// pointers in the grid_vtable_t for a hexagonal grid instance.
//

/**
 * @brief Converts hexagonal coordinates to pixel coordinates.
 * @param grid The grid system instance (provides layout).
 * @param cell The hexagonal cell to convert.
 * @return The calculated pixel coordinate as a point_t.
 */
point_t hex_to_pixel_impl(const grid_t* grid, grid_cell_t cell);

/**
 * @brief Converts pixel coordinates to the nearest hexagonal cell.
 * @param grid The grid system instance (provides layout).
 * @param p The pixel point to convert.
 * @return The closest hexagonal grid cell.
 */
grid_cell_t hex_from_pixel_impl(const grid_t* grid, point_t p);

/**
 * @brief Gets a neighboring hexagonal cell in a specified direction.
 * @param cell The starting hexagonal cell.
 * @param direction An integer from 0-5 representing the direction.
 * @param out_neighbor Pointer to a grid_cell_t to store the resulting neighbor.
 */
void hex_get_neighbor_impl(grid_cell_t cell, int direction, grid_cell_t* out_neighbor);

/**
 * @brief Calculates the distance between two hexagonal cells.
 * @param a The first hexagonal cell.
 * @param b The second hexagonal cell.
 * @return The distance in number of cells (Manhattan distance on cube coordinates).
 */
int hex_distance_impl(grid_cell_t a, grid_cell_t b);

/**
 * @brief Gets the pixel coordinates of a hexagon's 6 corners.
 * @param grid The grid system instance (provides layout).
 * @param cell The hexagonal cell whose corners are to be calculated.
 * @param corners An array of 6 point_t to be filled with the corner coordinates.
 */
void hex_get_corners_impl(const grid_t* grid, grid_cell_t cell, point_t corners[]);

/**
 * @brief Generates all hexagonal cells within a given radius from the center.
 * @param grid The grid system instance to populate with cells.
 * @param radius The radius of the hexagonal grid to generate.
 */
void hex_generate_cells_impl(grid_t* grid, int radius);


// The public instance of the v-table for hexagonal grids.
// This is defined in hex_grid.c and used by the grid_create factory function.
extern const grid_vtable_t hex_grid_vtable;

#endif // HEX_GRID_H
