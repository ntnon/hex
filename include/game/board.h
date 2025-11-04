#ifndef BOARD_H
#define BOARD_H

#include "grid/grid_geometry.h"
#include "tile/tile_map.h"
#include "tile/pool_manager.h"
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
    pool_manager_t *pools;
    uint32_t next_pool_id;
    Camera2D camera; // Camera for this board
} board_t;

board_t *board_create(grid_type_e grid_type, int radius, board_type_e board_type);
void clear_board(board_t *board);
void free_board(board_t *board);

void board_randomize(board_t *board, int radius, board_type_e board_type);

void board_add_tile(board_t *board, tile_t* tile);
void remove_tile(board_t *board, tile_t* tile);
void board_fill(board_t *board, int radius, board_type_e board_type);
void get_neighbor_pools(board_t *board, tile_t *tile, pool_t **out_pools, size_t max_neighbors);
bool board_rotate(board_t *board, grid_cell_t center, int rotation_steps);
void cycle_tile_type(board_t *board, tile_t *tile);

void board_add_tiles_batch(board_t *board, tile_t **tiles, size_t count);
void assign_pools_batch(board_t *board);
void flood_fill_assign_pool(board_t *board, tile_t *start_tile, pool_t *pool);
void board_fill_batch(board_t *board, int radius, board_type_e board_type);
void board_fill_fast(board_t *board, int radius, board_type_e board_type);

bool is_merge_valid(board_t *target_board, board_t *source_board, grid_cell_t target_center, grid_cell_t source_center);

bool merge_boards(board_t *target_board, board_t *source_board, grid_cell_t target_center, grid_cell_t source_center);


tile_t *get_tile_at_cell(const board_t *board, grid_cell_t cell);

/**
 * @brief Test function to verify pool merging and singleton logic works correctly.
 * @param board The board to test on (will be cleared and modified).
 */
void test_pool_logic(board_t *board);

/**
 * @brief Validates that all tiles in a tile map are within the board's grid bounds.
 * @param board The board that defines the valid bounds.
 * @param tile_map The tile map to validate.
 * @return True if all tiles are within bounds, false otherwise.
 */
bool board_validate_tile_map_bounds(const board_t *board, const tile_map_t *tile_map);

/**
 * @brief Calculates the smallest bounding box that contains all cells in the board.
 * @param board The board to calculate bounds for.
 * @param out_min_x Output for minimum x coordinate.
 * @param out_min_y Output for minimum y coordinate.
 * @param out_max_x Output for maximum x coordinate.
 * @param out_max_y Output for maximum y coordinate.
 * @return True if bounds were calculated successfully, false otherwise.
 */
bool board_calculate_bounds(const board_t *board, float *out_min_x, float *out_min_y, float *out_max_x, float *out_max_y);

#endif /* BOARD_H */
