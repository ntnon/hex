<<<<<<< HEAD
#include <stdbool.h>
#include <stdio.h>

<<<<<<< HEAD
#include "controller/input_state.h"
#include "raylib.h"
#include "ui/ui_context.h"
=======
#define RAYGUI_IMPLEMENTATION
#include "../include/third_party/raygui.h"
#include "controller/input_controller.h"
#include "raylib.h"
#include "render/renderer.h"
#include "screen/game_screen.h"
#include "screen/menu_screen.h"
#include "screen/pause_screen.h"
#include "screen/screen_manager.h"
#include <stdbool.h>
#include <stdio.h>
>>>>>>> parent of 53165fd (continued decoupling of raylib and game logic.)
=======
#define CLAY_IMPLEMENTATION
#include "../include/third_party/clay.h" // UI system
#include "render/clay_renderer_raylib.h"

#include "game/camera.h"
#include "game/game_controller.h"
#include "render/renderer.h"
#include "ui.h"
>>>>>>> pre_slop

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

<<<<<<< HEAD
  // Initialize Clay
  Clay_Initialize ((Clay_Arena){ 1024 * 1024 }, (Clay_Dimensions){ 1024 },
                   (Clay_ErrorHandler){});

<<<<<<< HEAD
  // Initialize UI context
  ui_context_t ui_ctx;
  ui_context_init (&ui_ctx);

  bool running = true;
=======
  input_controller_t input_ctrl;
  input_controller_init (&input_ctrl);
=======
Effects and alterations:
1. Increase / Decrease production
- Flat
- Percentage
2. Add product
3. Remove product
4. Change color of entity
5. Remove entity
6. Move entity
>>>>>>> pre_slop

Modifiers:
1. Recurring
- For each n-th loop
2. One time
- For each (entity/cycle)
3. Gated
- Condition gate (e.g green production > 20% of total production)
- Luck based gate (roll )

 */

<<<<<<< HEAD
  pause_screen_t pause_screen;
  pause_screen_init (&pause_screen, initial_width, initial_height);

  // Register screens with input controller
  input_controller_register_screen (&input_ctrl, SCREEN_MENU,
                                    menu_input_handler, menu_action_handler,
                                    menu_render_handler, &menu_screen);

  input_controller_register_screen (&input_ctrl, SCREEN_GAME,
                                    game_input_handler, game_action_handler,
                                    game_render_handler, &game_screen);

  input_controller_register_screen (&input_ctrl, SCREEN_PAUSE,
                                    pause_input_handler, pause_action_handler,
                                    pause_render_handler, &pause_screen);

  // Set initial screen
  screen_manager_switch (&screen_mgr, SCREEN_MENU);
>>>>>>> parent of 53165fd (continued decoupling of raylib and game logic.)

  // Main game loop
  while (running && !WindowShouldClose ())
    {
<<<<<<< HEAD
      // Handle input
      input_state_t input;
      get_input_state (&input);

      // Update based on current screen
      switch (ui_ctx.current_screen)
        {
        case SCREEN_MENU:
          handle_menu_input (&ui_ctx, &input);
          break;
        case SCREEN_GAME:
          handle_game_input (&ui_ctx, &input);
          break;
        case SCREEN_SETTINGS:
          handle_settings_input (&ui_ctx, &input);
          break;
        }

      // Render
      BeginDrawing ();
      ClearBackground (WHITE);

      // Begin Clay frame
      Clay_BeginLayout ();

      // Render current screen
      switch (ui_ctx.current_screen)
        {
        case SCREEN_MENU:
          render_menu (&ui_ctx);
          break;
        case SCREEN_GAME:
          render_game (&ui_ctx);
          break;
        case SCREEN_SETTINGS:
          render_settings (&ui_ctx);
          break;
        }

      // End Clay frame and render
      Clay_RenderCommandArray clay_commands = Clay_EndLayout ();

      // TODO: Render Clay commands with Raylib renderer
      // For now, basic rendering will work
=======

      // Handle input and actions for current screen
      input_controller_update (&input_ctrl, &screen_mgr, &running);

      // Render current screen
      BeginDrawing ();
      ClearBackground (WHITE);

      input_controller_render (&input_ctrl, &screen_mgr);
>>>>>>> parent of 53165fd (continued decoupling of raylib and game logic.)
=======
int main(void) {
  const int initial_width = 1300;
  const int initial_height = 700;

  game_t game;
  game_init(&game);

  game_controller_t controller;

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
    ClearBackground(WHITE);
    Clay_Raylib_Render(renderCommands, UI_FONTS);
    controller_process_events(&controller);

    BeginMode2D(controller.game_camera);

    BeginScissorMode(controller.game_bounds.x, controller.game_bounds.y,
                     controller.game_bounds.width,
                     controller.game_bounds.height);
>>>>>>> pre_slop

    render_hex_grid(game.board->grid);
    // render_board(game.board);
    EndScissorMode();
    for (int i = 0; i < inventory_get_size(controller.game->inventory); i++) {
        Clay_ElementId id = { .id = UI_ID_INVENTORY_ITEM_BASE_STRING + i };
        Clay_BoundingBox box = Clay_GetElementData(id).boundingBox;
    
        // Render your custom thing inside this box
        BeginScissorMode(box.x, box.y, box.width, box.height);
    
        // Example: draw item preview
        Item *item = inventory_get_item(controller->game->inventory, i);
        if (item) {
            DrawText(i, box.x + 5, box.y + 5, 10, BLACK);
            // or DrawTexture inside box
        }
    
        EndScissorMode();
    }
    EndDrawing();
  }

<<<<<<< HEAD
<<<<<<< HEAD
  ui_context_cleanup (&ui_ctx);
=======
  // Cleanup
  game_screen_unload (&game_screen);
  menu_screen_unload (&menu_screen);
  pause_screen_unload (&pause_screen);
  input_controller_destroy (&input_ctrl);

>>>>>>> parent of 53165fd (continued decoupling of raylib and game logic.)
  CloseWindow ();
  return 0;
=======
  Clay_Raylib_Close();
>>>>>>> pre_slop
}
