#include "../grid/grid_system.h"
#include "../tile/tile_manager.h"
typedef struct  {
    grid_t *grid;
    tile_manager_t *tile_manager;
}board_t;

void board_draw(board_t *board);
