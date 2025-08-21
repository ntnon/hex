#include <stdbool.h>
#include <stdio.h>

#define CLAY_IMPLEMENTATION
#include "../include/third_party/clay.h" // UI system
#include "render/clay_renderer_raylib.h"

#include "game/board.h"
#include "game/camera.h"
#include "game/game_controller.h"
#include "render/renderer.h"
#include "ui.h"

int main(void) {
  const int initial_width = 1000;
  const int initial_height = 700;

  game_t game;
  game_init(&game);
  board_randomize(game.board);
  inventory_fill(game.inventory, 5);

  game_controller_t controller = {0};

  input_state_t input;

  ui_load_fonts();
  UI_Context ui = ui_init(initial_width, initial_height);

  ui_build_layout(&controller);
  controller_init(&controller, &game); // must happen after ui_init
  controller.game_bounds = Clay_GetElementData(CLAY_ID("right")).boundingBox;

  camera_init(&controller.camera, controller.game_bounds.width,
              controller.game_bounds.height);

  while (!WindowShouldClose()) {
    get_input_state(&input);
    controller_update(&controller, &input);
    controller.game_bounds = Clay_GetElementData(CLAY_ID("right")).boundingBox;

    Clay_RenderCommandArray renderCommands = ui_build_layout(&controller);

    BeginDrawing();
    ClearBackground(WHITE);
    Clay_Raylib_Render(renderCommands, UI_FONTS);
    controller_process_events(&controller);

    BeginMode2D(controller.camera);

    BeginScissorMode(controller.game_bounds.x, controller.game_bounds.y,
                     controller.game_bounds.width,
                     controller.game_bounds.height);
    render_board(game.board);
    EndScissorMode();

    EndDrawing();
  }
  Clay_Raylib_Close();
}
