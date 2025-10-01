#include "controller/app_controller.h"
#include <stdbool.h>
#include <stdio.h>

#define CLAY_IMPLEMENTATION
#include "../include/third_party/clay.h" // UI system
#include "raylib.h"

#include "third_party/clay_renderer_raylib.h"

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


Ability to consume a type of upgrade, so that it no longer appears in the
upgrade menu. Pay for this. This is like "reducing" your deck.

Pay with inventory slots
*/

int main(int argc, char *argv[]) {
  const int initial_width = 1300;
  const int initial_height = 700;

  // Initialize window
  InitWindow(initial_width, initial_height, "HexHex Game");
  SetTargetFPS(60);

  // Initialize app controller
  app_controller_t app_controller;
  app_controller_init(&app_controller);
  printf("App controller initialized successfully\n");

  // Validate app controller state
  if (!app_controller.is_initialized) {
    printf("ERROR: App controller failed to initialize properly\n");
    return 1;
  }
  printf("App controller validation passed\n");

  // Initialize UI system
  UI_Context ui = ui_init(initial_width, initial_height);

  printf("Starting main game loop\n");
  // Main game loop
  while (!WindowShouldClose() && !app_controller_should_quit(&app_controller)) {
    input_state_t input;
    get_input_state(&input);

    // Update app controller
    app_controller_update(&app_controller, &input);
    app_controller_process_events(&app_controller);

    // Build UI layout based on current app state with error handling
    Clay_RenderCommandArray renderCommands;
    printf("Building UI layout for app state %d\n",
           app_controller_get_state(&app_controller));

    renderCommands = ui_build_layout(&app_controller);

    // Validate render commands before passing to Clay
    if (renderCommands.length == 0 || !renderCommands.internalArray) {
      printf("Error: Invalid render commands (length=%d, array=%p), skipping "
             "frame\n",
             renderCommands.length, renderCommands.internalArray);
      BeginDrawing();
      ClearBackground(BROWN);
      EndDrawing();
      continue;
    }

    printf("UI layout built successfully - %d render commands\n",
           renderCommands.length);

    BeginDrawing();
    ClearBackground(BROWN);

    // Render game world if in playing state
    if (app_controller_get_state(&app_controller) == APP_STATE_PLAYING ||
        app_controller_get_state(&app_controller) == APP_STATE_PAUSED) {
      if (app_controller.game) {
        Clay_BoundingBox game_bounds =
          Clay_GetElementData(UI_ID_GAME).boundingBox;

        camera_set_offset(&app_controller.game->board->camera,
                          game_bounds.width, game_bounds.height);
        BeginMode2D(app_controller.game->board->camera);
        render_board(app_controller.game->board);
        render_board_previews(app_controller.game->board);
        EndMode2D();
      }
    }

    // Render UI
    Clay_Raylib_Render(renderCommands, UI_FONTS);

    // Render inventory overlay if in game
    if (app_controller_get_state(&app_controller) == APP_STATE_PLAYING ||
        app_controller_get_state(&app_controller) == APP_STATE_PAUSED) {
      if (app_controller.game) {
        render_inventory(app_controller.game->inventory);
      }
    }

    EndDrawing();
  }

  // Cleanup - simplified order
  app_controller_cleanup(&app_controller);
  ui_shutdown(&ui);
  CloseWindow();

  return 0;
}
