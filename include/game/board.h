#ifndef BOARD_H
#define BOARD_H

#include "grid/grid_system.h"
#include "tile/tile_map.h"
#include "tile/pool_map.h"
#include "utility/array_shuffle.h"
#include "raylib.h"

typedef struct {
    tile_t *tile;
    grid_cell_t preview_position;
    bool is_valid_position;
} preview_entry_t;

typedef struct {
    grid_t *grid;
    tile_map_t *tiles;
    pool_map_t *pools;
    uint32_t next_pool_id;
    Camera2D camera;
    
    // Preview system
    preview_entry_t *preview_tiles;
    size_t num_preview_tiles;
    size_t preview_capacity;
} board_t;

board_t *board_create(grid_type_e grid_type, int radius);

void clear_board(board_t *board);

void free_board(board_t *board);

void board_randomize(board_t *board);

void add_tile(board_t *board, tile_t* tile);

void get_neighbor_pools(board_t *board, tile_t *tile, pool_t **out_pools,
                        size_t max_neighbors);

void remove_tile(board_t *board, tile_t* tile);

void cycle_tile_type(board_t *board, tile_t *tile);

bool valid_tile(board_t *board, tile_t *tile);

//static pool_t* find_new_tile_pool_candidates(board_t *board, tile_t *tile);

bool is_merge_valid(board_t *target_board, board_t *source_board,
                    grid_cell_t target_center, grid_cell_t source_center);

bool merge_boards(board_t *target_board, board_t *source_board,
                  grid_cell_t target_center, grid_cell_t source_center);

tile_t *get_tile_at_cell(board_t *board, grid_cell_t cell);

void print_board_debug_info(board_t *board);

// Preview system functions
void board_init_preview_system(board_t *board, size_t initial_capacity);
void board_free_preview_system(board_t *board);
void board_add_preview_tile(board_t *board, tile_t *tile, grid_cell_t position);
void board_clear_preview_tiles(board_t *board);
void board_validate_preview_tiles(board_t *board);
bool board_is_preview_position_valid(board_t *board, tile_t *tile, grid_cell_t position);

#endif /* BOARD_H */
