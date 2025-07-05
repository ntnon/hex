/**************************************************************************//**
 * @file tile_manager.h
 * @brief Manages the overall game state related to tiles, including tile pools.
 *
 * This acts as the central hub for tile and pool operations, coordinating
 * updates and interactions based on game logic.
 *****************************************************************************/

#ifndef TILE_MANAGER_H
#define TILE_MANAGER_H

//#include "../grid/grid_system.h"
#include "tile.h"
#include "tile_map.h"
#include "raylib.h"

// --- Tile Manager Structure ---
/**
 * @brief The main manager for all tiles and their associated pools.
 *
 * This struct holds the global collection of tiles (likely a hash map)
 * and the manager for all tile pools. It's responsible for game logic
 * related to tile interactions and pool updates.
 */
typedef struct tile_manager {
    tile_map_entry_t* tiles;
} tile_manager_t;

tile_manager_t* tile_manager_create(void);
void tile_manager_free(tile_manager_t* tm);
void tile_manager_clear(tile_manager_t* tm);

void draw_tiles(tile_manager_t* tm);

void tile_manager_add_tile(tile_manager_t* tm, tile_t* tile);
bool tile_manager_remove_tile(tile_manager_t* tm, grid_cell_t cell);
tile_t* tile_manager_get_tile(tile_manager_t* tm, grid_cell_t cell);

tile_t* cell_to_tile_ptr(tile_manager_t *tm, grid_cell_t cell);

void cells_to_tile_ptrs(tile_manager_t *tm, const grid_cell_t *cells, size_t num_cells, tile_t **out_tile_ptrs);

#endif // TILE_MANAGER_H
