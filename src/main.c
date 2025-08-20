#include <stdbool.h>
#include <stdio.h>

#include "game/board.h"
#include "game/camera.h"

#include "raylib.h"
#include "render/renderer.h"

#define CLAY_IMPLEMENTATION
#include "../include/third_party/clay.h" // UI system
#include "render/clay_renderer_raylib.h"
#include "ui.h"

int main(void) {
  const int initial_width = 1000;
  const int initial_height = 700;

  game_t game;
  game_init(&game);
  board_randomize(game.board);
  inventory_fill(game.inventory, 5);

  Camera2D camera = {0};
  camera.target = (Vector2){0.0f, 0.0f};
  camera.offset = (Vector2){GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};
  camera.zoom = 1.0f;

  ui_load_fonts();
  UI_Context ui = ui_init(initial_width, initial_height);

  while (!WindowShouldClose()) {
    input_state_t input;
    get_input_state(&input);

    Clay_SetLayoutDimensions(
     (Clay_Dimensions){.width = GetScreenWidth(), .height = GetScreenHeight()});
    Clay_RenderCommandArray renderCommands = ui_build_layout(&game);

    update_camera(&camera, &input);
    BeginDrawing();
    ClearBackground(WHITE);
    Clay_Raylib_Render(renderCommands, UI_FONTS);
    BeginMode2D(camera);
    Clay_BoundingBox topRect =
     Clay_GetElementData(CLAY_ID("right")).boundingBox;
    BeginScissorMode(topRect.x, topRect.y, topRect.width, topRect.height);
    render_board(game.board);
    EndScissorMode();
    EndDrawing();
  }

  Clay_Raylib_Close();
}
