#include "raylib.h"
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
  controller_init(&controller, &game);

  input_state_t input;

  ui_load_fonts();
  UI_Context ui = ui_init(initial_width, initial_height);

  while (!WindowShouldClose()) {

    get_input_state(&input);
    controller_update(&controller, &input);

    Clay_RenderCommandArray renderCommands =
      ui_build_layout(&game, &controller);

    BeginDrawing();
    ClearBackground(WHITE);
    Clay_Raylib_Render(renderCommands, UI_FONTS);
    controller_handle_events(&controller);

    BeginMode2D(controller.camera);
    Clay_BoundingBox topRect =
      Clay_GetElementData(CLAY_ID("right")).boundingBox;
    BeginScissorMode(topRect.x, topRect.y, topRect.width, topRect.height);
    render_board(game.board);
    EndScissorMode();
    EndDrawing();
  }

  Clay_Raylib_Close();
}
