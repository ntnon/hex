#ifndef BOARD_H
#define BOARD_H

#include "grid/grid_geometry.h"
#include "tile/tile_map.h"
#include "tile/pool_map.h"
#include "utility/array_shuffle.h"
#include "raylib.h"

typedef enum {
    BOARD_TYPE_MAIN,        /* Main game board with center tile */
    BOARD_TYPE_INVENTORY    /* Inventory piece without center tile */
} board_type_e;

typedef struct {
    // Geometry configuration
    grid_type_e geometry_type;        /* Which grid geometry to use (hex, square, etc.) */
    const grid_vtable_t *geometry;    /* Function pointers for this geometry type */
    layout_t layout;                  /* Layout for pixel conversions */
    int radius;                       /* Board spatial bounds */
    board_type_e board_type;          /* Main board or inventory piece */

    // Game data
    tile_map_t *tiles;
    pool_map_t *pools;
    uint32_t next_pool_id;
    Camera2D camera;

} board_t;

board_t *board_create(grid_type_e grid_type, int radius, board_type_e board_type);

void clear_board(board_t *board);

void free_board(board_t *board);

void board_randomize(board_t *board, int radius, board_type_e board_type);
void board_fill(board_t *board, int radius, board_type_e board_type);

void add_tile(board_t *board, tile_t* tile);

void get_neighbor_pools(board_t *board, tile_t *tile, pool_t **out_pools,
                        size_t max_neighbors);

void remove_tile(board_t *board, tile_t* tile);

void cycle_tile_type(board_t *board, tile_t *tile);

bool valid_tile(board_t *board, tile_t *tile);

/**
 * @brief Grows the board's grid by the specified amount.
 * @param board The board to grow.
 * @param growth_amount Amount to increase the grid radius by.
 * @return true if growth was successful, false otherwise.
 */

//static pool_t* find_new_tile_pool_candidates(board_t *board, tile_t *tile);

bool is_merge_valid(board_t *target_board, board_t *source_board,
                    grid_cell_t target_center, grid_cell_t source_center);

bool merge_boards(board_t *target_board, board_t *source_board,
                  grid_cell_t target_center, grid_cell_t source_center);

bool board_rotate(board_t *board, grid_cell_t center, int rotation_steps);

tile_t *get_tile_at_cell(const board_t *board, grid_cell_t cell);




/**
 * @brief Validates that all tiles in a tile map are within the board's grid bounds.
 * @param board The board that defines the valid bounds.
 * @param tile_map The tile map to validate.
 * @return True if all tiles are within bounds, false otherwise.
 */
bool board_validate_tile_map_bounds(const board_t *board, const tile_map_t *tile_map);

#endif /* BOARD_H */
