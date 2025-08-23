#include <stdbool.h>
#include <stdio.h>

#define CLAY_IMPLEMENTATION
#include "../include/third_party/clay.h" // UI system
#include "render/clay_renderer_raylib.h"

#include "game/camera.h"
#include "game/game_controller.h"
#include "render/renderer.h"
#include "ui.h"

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

 */

int main(void) {
  const int initial_width = 1300;
  const int initial_height = 700;

  game_t game;
  game_init(&game);

  game_controller_t controller;
  controller_init(&controller, &game);

  UI_Context ui = ui_init(initial_width, initial_height);

  // Initial layout build to get game bounds
  ui_build_layout(&controller);
  Clay_BoundingBox game_bounds = Clay_GetElementData(UI_ID_GAME).boundingBox;
  controller.game_bounds = (Clay_BoundingBox){
    game_bounds.x, game_bounds.y, game_bounds.width, game_bounds.height};

  camera_set_offset(&controller.game_camera, game_bounds.width,
                    game_bounds.height);

  while (!WindowShouldClose()) {
    get_input_state(&controller.input);
    controller_update(&controller, &controller.input);

    Clay_RenderCommandArray renderCommands = ui_build_layout(&controller);

    BeginDrawing();
    ClearBackground(WHITE);

    // Render Clay UI
    Clay_Raylib_Render(renderCommands, UI_FONTS);

    // Process controller events
    controller_process_events(&controller);

    EndDrawing();
  }

  Clay_Raylib_Close();
  CloseWindow();

  return 0;
}
