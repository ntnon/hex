#include <stdbool.h>
#include <stdio.h>

#define CLAY_IMPLEMENTATION
#include "../include/third_party/clay.h" // UI system
#include "raylib.h"

#include "third_party/clay_renderer_raylib.h"

#include "controller/game_controller.h"
#include "game/board.h"
#include "game/camera.h"

#include "renderer.h"
#include "ui.h"
#include "utility/string.h"

/*

Apply effect to entity:
1. Local
- Tile
- Pool
2. Global
- All pools of a color
- All tiles of a color
- All pools
- All tiles
3. Future (pieces)
- Add tile to piece
- Remove tile from piece
- Boost tile in piece
- Boost all tiles in piece

Conditions
1. Tile adjacency
2. Pool adjacency
3. Tile color
4. Pool color
5. Pool size
6. Pool center
7. Pool max distance (tiles farthest removed in pool)
8. Pool compactness (how many shared edges in pool)


Effects and alterations:
1. Increase / Decrease production
- Flat
- Percentage
2. Add product
3. Remove product
4. Change color of entity
5. Remove entity
6. Move entity

Modifiers:
1. Recurring
- For each n-th loop
2. One time
- For each (entity/cycle)
3. Gated
- Condition gate (e.g green production > 20% of total production)
- Luck based gate (roll )


Stone tiles - dead tiles, no production
Paintbrush - reskin a tile or group of tiles


Ability to consume a type of upgrade, so that it no longer appears in the upgrade menu. Pay for this. This is like "reducing" your deck.
 
Pay with inventory slots
*/

int main(int argc, char *argv[]) {
  const int initial_width = 1300;
  const int initial_height = 700;

  game_t game;
  game_init(&game);

  print_board_debug_info(game.board);

  game_controller_t controller;

  UI_Context ui = ui_init(initial_width, initial_height);
  controller_init(&controller, &game);

  ui_build_layout(&controller);

  // Cache will be initialized dynamically on first rebuild

  while (!WindowShouldClose()) {
    get_input_state(&controller.input);
    controller_update(&controller, &controller.input);

    Clay_RenderCommandArray renderCommands = ui_build_layout(&controller);

    BeginDrawing();
    ClearBackground(BROWN);
    controller_process_events(&controller);

    if (controller.game->state == GAME_STATE_PLAYING) {
      // Render the main game board
      Clay_BoundingBox game_bounds =
        Clay_GetElementData(UI_ID_GAME).boundingBox;

      camera_set_offset(&controller.game->board->camera, game_bounds.width,
                        game_bounds.height);
      BeginMode2D(controller.game->board->camera);
      // render_hex_grid(controller.game->board->grid);
      render_board(controller.game->board);
      render_board_previews(controller.game->board);
      EndMode2D();

      // Render CLAY
      Clay_Raylib_Render(renderCommands, UI_FONTS);

      // Render inventory on top of Clay
      render_inventory(controller.game->inventory);
    } else {
      Clay_Raylib_Render(renderCommands, UI_FONTS);
    }

    EndDrawing();
  }
  Clay_Raylib_Close();
}
