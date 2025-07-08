#include "../../include/board/renderer.h"
#include "../../include/grid/grid_cell_utils.h"
#include "raylib.h"
#include <float.h>
#include <math.h>
#include <stdio.h>

#define TILE_CACHE_SIZE 2048 // Maximum size for each tile

// Structure to hold a cached tile
typedef struct
{
  RenderTexture2D texture;
  // Its offset in the overall grid cache (world-space position of this tile's
  // top-left)
  Vector2 offset;
  // The actual size of this tile (could be smaller than TILE_CACHE_SIZE at
  // edges)
  int width;
  int height;
} TileCache;

// Static variables for the tiled cache
static bool tiledCacheInitialized = false;
static TileCache *tileCaches = NULL;
static int tileCountX = 0, tileCountY = 0;
static Vector2 gridCacheMin = { 0 }; // overall grid bounding box minimum

// Helper: Draw each tile from the tile map; user_data is a pointer to the
// grid.
static void
draw_tile_wrapper (tile_map_entry_t *entry, void *user_data)
{
  const grid_t *grid = (const grid_t *)user_data;
  renderer_draw_tile (entry->tile, grid);
}

/*
 * update_tiled_grid_cache: Builds the tiled cache for the grid.
 *
 * This computes the overall bounding box of the hex grid by iterating over all
 * cells, then splits that into smaller tiles (each up to TILE_CACHE_SIZE in
 * size).
 */
static void
update_tiled_grid_cache (const board_t *board)
{

  const grid_t *grid = board->grid;
  float min_x = FLT_MAX, min_y = FLT_MAX;
  float max_x = -FLT_MAX, max_y = -FLT_MAX;

  // Compute overall grid bounding box from all hex cell corners.
  for (size_t i = 0; i < grid->num_cells; i++)
    {
      grid_cell_t cell = grid->cells[i];
      point_t corners[6];
      grid->vtable->get_corners (grid, cell, corners);
      for (int j = 0; j < 6; j++)
        {
          if (corners[j].x < min_x)
            min_x = corners[j].x;
          if (corners[j].y < min_y)
            min_y = corners[j].y;
          if (corners[j].x > max_x)
            max_x = corners[j].x;
          if (corners[j].y > max_y)
            max_y = corners[j].y;
        }
    }
  gridCacheMin.x = min_x;
  gridCacheMin.y = min_y;
  int totalWidth = (int)ceilf (max_x - min_x);
  int totalHeight = (int)ceilf (max_y - min_y);

  // Calculate how many tiles are needed in each direction.
  tileCountX = (totalWidth + TILE_CACHE_SIZE - 1) / TILE_CACHE_SIZE;
  tileCountY = (totalHeight + TILE_CACHE_SIZE - 1) / TILE_CACHE_SIZE;

  // Allocate array for tile caches.
  tileCaches = malloc (sizeof (TileCache) * tileCountX * tileCountY);
  if (!tileCaches)
    {
      printf ("Failed to allocate tileCaches\n");
      return;
    }

  // Create each tile cache.
  for (int ty = 0; ty < tileCountY; ty++)
    {
      for (int tx = 0; tx < tileCountX; tx++)
        {
          int index = ty * tileCountX + tx;
          TileCache *tile = &tileCaches[index];

          // Determine the size for this tile (handle edges)
          tile->width = ((tx + 1) * TILE_CACHE_SIZE > totalWidth)
                            ? (totalWidth - tx * TILE_CACHE_SIZE)
                            : TILE_CACHE_SIZE;
          tile->height = ((ty + 1) * TILE_CACHE_SIZE > totalHeight)
                             ? (totalHeight - ty * TILE_CACHE_SIZE)
                             : TILE_CACHE_SIZE;

          // The world offset for this tile.
          tile->offset.x = min_x + tx * TILE_CACHE_SIZE;
          tile->offset.y = min_y + ty * TILE_CACHE_SIZE;

          tile->texture = LoadRenderTexture (tile->width, tile->height);
          // Optional: Setup bilinear filtering

          BeginTextureMode (tile->texture);
          ClearBackground (BLANK);
          // Setup a camera so that the tile's region of the grid is drawn.
          Camera2D cam = { 0 };
          cam.offset = (Vector2){ 0, 0 };
          // Translate so that the tile's top left becomes (0,0) in the
          // texture.
          cam.target = (Vector2){ tile->offset.x, tile->offset.y };
          cam.rotation = 0.0f;
          cam.zoom = 1.0f;
          BeginMode2D (cam);
          // Render every tile from the board.
          tile_map_foreach (board->tile_manager->tiles, draw_tile_wrapper,
                            (void *)grid);
          EndMode2D ();
          EndTextureMode ();
        }
    }
  tiledCacheInitialized = true;
}

void
renderer_draw_board (const board_t *board)
{
  if (!board)
    {
      printf ("ERROR: board is null\n");
      return;
    }

  if (!board->grid)
    {
      printf ("ERROR: board->grid is null\n");
      return;
    }

  if (!board->tile_manager)
    {
      printf ("ERROR: board->tile_manager is null\n");
      return;
    }

  // Build tiled cache if not already built.
  if (!tiledCacheInitialized)
    update_tiled_grid_cache (board);

  // Draw each cached tile at its proper world offset.
  for (int ty = 0; ty < tileCountY; ty++)
    {
      for (int tx = 0; tx < tileCountX; tx++)
        {
          int index = ty * tileCountX + tx;
          TileCache *tile = &tileCaches[index];
          // RenderTextures in Raylib are y-flipped.
          Rectangle srcRec = { 0, 0, (float)tile->texture.texture.width,
                               -(float)tile->texture.texture.height };
          DrawTextureRec (tile->texture.texture, srcRec, tile->offset, WHITE);
        }
    }
}
void
renderer_draw_tile (const tile_t *tile, const grid_t *grid)
{

  if (!tile)
    {
      printf ("ERROR: tile is null\n");
      return;
    }
  if (!grid)
    {
      printf ("ERROR: grid is null\n");
      return;
    }
  if (!grid->vtable)
    {
      printf ("ERROR: grid->vtable is null\n");
      return;
    }
  if (!grid->vtable->draw_cell_with_colors)
    {
      printf ("ERROR: grid->vtable->draw_cell_with_colors is null\n");
      return;
    }

  // Draw the colored hex cell.
  grid->vtable->draw_cell_with_colors (grid, tile->cell, tile_get_color (tile),
                                       BLANK);

  // Compute the center of the hex cell using the to_pixel function.
  point_t center = grid->vtable->to_pixel (grid, tile->cell);

  // Create a string representing the hex coordinates.
  char coord_text[32];
  grid_cell_to_string (&tile->cell, coord_text, sizeof (coord_text));

  // Draw the coordinate text centered on the cell.
  // Adjust the x and y values (here subtracting 10) as needed.
  DrawText (coord_text, (int)center.x - 10, (int)center.y - 10, 10, BLACK);
}

void
renderer_draw_hover_info (const board_t *board,
                          const board_input_controller_t *ctrl)
{
  if (!ctrl->has_hovered_cell)
    return;
  tile_t *tile = cell_to_tile_ptr (board->tile_manager, ctrl->hovered_cell);
  if (!tile)
    return;
  Vector2 mouse = GetMousePosition ();
  char info[128];

  pool_t *pool = tile_to_pool_map_get_pool_by_tile (board->tile_to_pool, tile);
  int pool_id = pool ? pool->id : -1;
  int pool_max_friendly_neighbors = pool ? pool->highest_n : -1;

  snprintf (info, sizeof (info), "Type: %d\nValue: %d\nPool: %d\nMFN: %d",
            tile->type, tile->value, pool_id, pool_max_friendly_neighbors);

  int height = 60, width = 120;
  DrawRectangle (mouse.x + 16, mouse.y + 16, width, height, Fade (GRAY, 0.9f));
  DrawText (info, mouse.x + 24, mouse.y + 24, 16, BLACK);
}

void
renderer_draw (const board_t *board,
               const board_input_controller_t *input_ctrl)
{

  BeginMode2D ((Camera2D){
      .offset = (Vector2){ GetScreenWidth () / 2.0f + input_ctrl->pan.x,
                           GetScreenHeight () / 2.0f + input_ctrl->pan.y },
      .zoom = input_ctrl->zoom,
      .rotation = 0.0f,
      .target = (Vector2){ 0, 0 } });
  renderer_draw_board (board);
  EndMode2D ();
  renderer_draw_hover_info (board, input_ctrl);
}
