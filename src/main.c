#include "raylib.h"
#define CLAY_IMPLEMENTATION
#include "../include/third_party/clay.h"
#include "third_party/clay_renderer_raylib.inc"

#include "controller/app_controller.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "game/board.h"
#include "game/camera.h"

#include "renderer/renderer.h"
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
void HandleClayErrors(Clay_ErrorData errorData) {
    // See the Clay_ErrorData struct for more information
    printf("%s", errorData.errorText.chars);
    switch (errorData.errorType) {
        // etc
    }
}

int main() {

    const int initial_width = 1300;
    const int initial_height = 700;

    // Initialize app controller
    app_controller_t app_controller;
    app_controller_init(&app_controller);
    printf("App controller initialized successfully\n");

    // Validate app controller state
    if (!app_controller.is_initialized) {
        printf("ERROR: App controller failed to initialize properly\n");
        return 1;
    }

    uint64_t totalMemorySize = Clay_MinMemorySize();
    Clay_Arena arena = Clay_CreateArenaWithCapacityAndMemory(
      totalMemorySize, malloc(totalMemorySize));
    ui_load_fonts();
    ui_init_text_configs();
    // After Clay_Initialize...
    Clay_Raylib_Initialize(initial_width, initial_height, "HexHex Game",
                           FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT |
                             FLAG_WINDOW_HIGHDPI | FLAG_MSAA_4X_HINT);
    // Note: screenWidth and screenHeight will need to come from your
    // environment, Clay doesn't handle window related tasks

    Clay_Initialize(arena, (Clay_Dimensions){initial_width, initial_height},
                    (Clay_ErrorHandler){HandleClayErrors});
    Clay_SetMeasureTextFunction(Raylib_MeasureText, UI_FONTS);

    // Set FPS after window is created
    SetTargetFPS(60);
    // SetExitKey(0);

    printf("Starting main game loop\n");
    // Main game loop
    while (!WindowShouldClose() &&
           !app_controller_should_quit(&app_controller)) {
        input_state_t input;
        input_state_init(&input); // Initialize before getting state
        get_input_state(&input);

        // Build UI layout based on current app state with error handling
        Clay_RenderCommandArray renderCommands;

        renderCommands = ui_root(&app_controller, &input);

        // Get the hovered element from the tracking system
        input.hovered_element_id = ui_get_hovered_element();

        // Set drag bounds if we're in game and hovering over game area
        if (app_controller_get_state(&app_controller) == APP_STATE_GAME) {
            if (input.hovered_element_id.id == ID_GAME_AREA.id) {
                Clay_BoundingBox game_bounds =
                  Clay_GetElementData(ID_GAME_AREA).boundingBox;
                input.drag_bounds = game_bounds;
            }
        }

        // Update app controller (after UI build so click state is available)
        app_controller_update(&app_controller, &input);

        // Validate render commands before passing to Clay
        if (renderCommands.length == 0 || !renderCommands.internalArray) {
            printf(
              "Error: Invalid render commands (length=%d, array=%p), skipping "
              "frame\n",
              renderCommands.length, renderCommands.internalArray);
            BeginDrawing();
            ClearBackground(BROWN);
            EndDrawing();
            continue;
        }

        BeginDrawing();
        ClearBackground(BROWN);

        // Render game world if in playing state
        if (app_controller_get_state(&app_controller) == APP_STATE_GAME) {
            if (app_controller.game) {
                Clay_BoundingBox game_bounds =
                  Clay_GetElementData(ID_GAME_AREA).boundingBox;

                camera_set_offset(&app_controller.game->board->camera,
                                  game_bounds.width, game_bounds.height);
                BeginMode2D(app_controller.game->board->camera);
                render_game(app_controller.game);

                EndMode2D();
            }
        }

        // Render UI
        Clay_Raylib_Render(renderCommands, UI_FONTS);

        // Render inventory overlay if in game
        if (app_controller_get_state(&app_controller) == APP_STATE_GAME) {
            if (app_controller.game) {
                render_inventory(app_controller.game->inventory);
            }
        }

        EndDrawing();

        // Clear click state at the very end of frame, after all processing
        ui_clear_click();
    }

    // Cleanup - simplified order
    app_controller_cleanup(&app_controller);
    // ui_shutdown(&ui);
    CloseWindow();

    return 0;
}
