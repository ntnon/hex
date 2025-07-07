#include "../grid/grid_system.h"
#include "../tile/tile_manager.h"
#include "../tile/pool_manager.h"

typedef struct  {
    grid_t *grid;
    tile_manager_t *tile_manager;
    pool_manager_t *pool_manager;
    tile_to_pool_entry_t *tile_to_pool;
}board_t;

board_t* board_create(void);

void clear_board(board_t *board);

void free_board(board_t *board);

void randomize_board(board_t *board);

void add_tile(board_t *board, tile_t* tile);

void get_neighbor_pools (board_t *board, tile_t *tile, pool_t **out_pools,
                    size_t max_neighbors);

void remove_tile(board_t *board, tile_t* tile);

void cycle_tile_type(board_t *board, tile_t *tile);

//static pool_t* find_new_tile_pool_candidates(board_t *board, tile_t *tile);
