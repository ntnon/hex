// // #include "../../include/input_controller.h"
// #include "../../include/controller/input_hover_render.h"

// void
// input_hover_render (const board_t *board, const board_input_controller_t
// *ctrl)
// {
//   if (!ctrl->has_hovered_cell)
//     return;
//   tile_t *tile = cell_to_tile_ptr (board->tile_manager, ctrl->hovered_cell);
//   if (!tile)
//     return;
//   Vector2 mouse = GetMousePosition ();
//   char info[128];

//   pool_t *pool = tile_to_pool_map_get_pool_by_tile (board->tile_to_pool,
//   tile); int pool_id = pool ? pool->id : -1; int pool_max_friendly_neighbors
//   = pool ? pool->highest_n : -1;

//   snprintf (info, sizeof (info), "Type: %d\nValue: %d\nPool: %d\nMFN: %d",
//             tile->data.type, tile->data.value, pool_id,
//             pool_max_friendly_neighbors);

//   int height = 60, width = 120;
//   DrawRectangle (mouse.x + 16, mouse.y + 16, width, height, Fade (GRAY,
//   0.9f)); DrawText (info, mouse.x + 24, mouse.y + 24, 16, BLACK);
// }
