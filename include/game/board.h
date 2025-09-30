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
    grid_cell_t *hovered_grid_cell;

    // Preview system
    struct board_preview_t *preview_boards;
    size_t num_preview_boards;
    size_t preview_capacity;
} board_t;

typedef struct board_preview_t {
    grid_cell_t *preview_positions;   /* Where tiles would be placed */
    tile_data_t *preview_tiles;       /* What tiles would be placed */
    size_t num_preview_tiles;

    grid_cell_t *conflict_positions;  /* Positions that would conflict */
    size_t num_conflicts;

    bool is_valid_merge;              /* Can this merge happen? */
} board_preview_t;

board_t *board_create(grid_type_e grid_type, int radius, board_type_e board_type);
board_t *board_clone(board_t *original);

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
bool board_grow(board_t *board, int growth_amount);

//static pool_t* find_new_tile_pool_candidates(board_t *board, tile_t *tile);

bool is_merge_valid(board_t *target_board, board_t *source_board,
                    grid_cell_t target_center, grid_cell_t source_center);

bool merge_boards(board_t *target_board, board_t *source_board,
                  grid_cell_t target_center, grid_cell_t source_center);

bool board_rotate(board_t *board, grid_cell_t center, int rotation_steps);

tile_t *get_tile_at_cell(board_t *board, grid_cell_t cell);

void print_board_debug_info(board_t *board);

// Simplified preview system functions
void board_init_preview_system(board_t *board, size_t initial_capacity);
void board_free_preview_system(board_t *board);
board_preview_t* board_create_merge_preview(board_t *target_board, board_t *source_board,
                                           grid_cell_t target_position, grid_cell_t source_center);
void board_free_preview(board_preview_t *preview);
void board_clear_preview_boards(board_t *board);
void board_update_preview(board_t *board, board_t *source_board, grid_cell_t mouse_position);

/**
 * @brief Validates that all tiles in a tile map are within the board's grid bounds.
 * @param board The board that defines the valid bounds.
 * @param tile_map The tile map to validate.
 * @return True if all tiles are within bounds, false otherwise.
 */
bool board_validate_tile_map_bounds(const board_t *board, const tile_map_t *tile_map);

#endif /* BOARD_H */
