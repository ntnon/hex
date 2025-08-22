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
 * User choose to improve:
 * - single tiles
 * - pools
 * - tile colors
 * - pieces
 */

int main(void) {
  const int initial_width = 1000;
  const int initial_height = 700;

  game_t game;
  game_init(&game);

  game_controller_t controller = {0};
  input_state_t input;

  UI_Context ui = ui_init(initial_width, initial_height);

  // build an initial layout, to know where to place elements
  ui_build_layout(&controller);
  controller_init(&controller, &game); // must happen after ui_init

  while (!WindowShouldClose()) {
    get_input_state(&input);
    controller_update(&controller, &input);
    controller.game_bounds = Clay_GetElementData(UI_ID_GAME).boundingBox;

    Clay_RenderCommandArray renderCommands = ui_build_layout(&controller);

    BeginDrawing();
    ClearBackground(WHITE);
    Clay_Raylib_Render(renderCommands, UI_FONTS);
    controller_process_events(&controller);

    BeginMode2D(controller.game_camera);

    BeginScissorMode(controller.game_bounds.x, controller.game_bounds.y,
                     controller.game_bounds.width,
                     controller.game_bounds.height);

    render_hex_grid(game.board->grid);
    render_board(game.board);
    EndScissorMode();

    EndDrawing();
  }
  Clay_Raylib_Close();
}
