#include "game/inventory.h"
#include <stdbool.h>
#include <stdio.h>

#define CLAY_IMPLEMENTATION
#include "../include/third_party/clay.h" // UI system
#include "raylib.h"

#include "third_party/clay_renderer_raylib.h"

#include "controller/game_controller.h"
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
 */

int main(void) {
  const int initial_width = 1300;
  const int initial_height = 700;

  game_t game;
  game_init(&game);

  print_board_debug_info(game.board);
  game_controller_t controller;

  ui_load_fonts();
  UI_Context ui = ui_init(initial_width, initial_height);
  controller_init(&controller, &game);

  ui_build_layout(&controller);
  Clay_BoundingBox game_bounds = Clay_GetElementData(UI_ID_GAME).boundingBox;

  camera_set_offset(&controller.game_camera, game_bounds.width,
                    game_bounds.height);
  while (!WindowShouldClose()) {
    get_input_state(&controller.input);
    controller_update(&controller, &controller.input);

    Clay_RenderCommandArray renderCommands = ui_build_layout(&controller);

    BeginDrawing();
    ClearBackground(BROWN);
    controller_process_events(&controller);

    BeginMode2D(controller.game_camera);

    render_hex_grid(game.board->grid);
    render_board(game.board);
    EndMode2D();

    Clay_Raylib_Render(renderCommands, UI_FONTS);

    for (int i = 0; i < inventory_get_size(controller.game->inventory); i++) {
      inventory_item_t item = inventory_get_item(controller.game->inventory, i);
      if (!is_id_valid(item.id))
        continue;
      Clay_BoundingBox boundingBox = Clay_GetElementData(item.id).boundingBox;
      if (boundingBox.width > 0 && boundingBox.height > 0) {

        BeginScissorMode(boundingBox.x, boundingBox.y, boundingBox.width,
                         boundingBox.height);
        DrawText(
          combine_string_int("hi:________1__________2__________3__________"
                             "4__________5__________6__________7_______",
                             i),
          boundingBox.x + 10, boundingBox.y + 10, 10, BLACK);
        EndScissorMode();
      }
    }

    EndDrawing();
  }
  Clay_Raylib_Close();
}
