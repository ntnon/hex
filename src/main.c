#include <stdbool.h>
#include <stdio.h>

#include "game/board.h"
#include "game/camera.h"
#include "game/game.h"

#include "raylib.h"
#include "render/renderer.h"

#define CLAY_IMPLEMENTATION
#include "../include/third_party/clay.h" // UI system
#include "render/clay_renderer_raylib.h"

#define FONT_DEFAULT_SPACING 3
#define MAX_FONTS 8
const uint32_t FONT_ID_LATO = 5;

// ..

void HandleClayErrors(Clay_ErrorData errorData) {
  printf("%s", errorData.errorText.chars);
}

int main(void) {
  const int initial_width = 1000;
  const int initial_height = 700;

  const Clay_Color COLOR_BACKGROUND = (Clay_Color){245, 245, 245, 255};
  const Clay_Color COLOR_BUTTON_PRIMARY = (Clay_Color){100, 150, 200, 255};
  const Clay_Color COLOR_BUTTON_SECONDARY = (Clay_Color){120, 120, 120, 255};
  const Clay_Color COLOR_BUTTON_DANGER = (Clay_Color){200, 100, 100, 255};
  const Clay_Color COLOR_OVERLAY = (Clay_Color){0, 0, 0, 150};
  const Clay_Color COLOR_PANEL = (Clay_Color){80, 80, 80, 255};
  const Clay_Color COLOR_TEXT_DARK = (Clay_Color){0, 0, 0, 255};
  const Clay_Color COLOR_TEXT_LIGHT = (Clay_Color){255, 255, 255, 255};
  static const Clay_TextElementConfig textConfigLarge = {
   .fontId = FONT_ID_LATO,
   .fontSize = 24,
   .letterSpacing = FONT_DEFAULT_SPACING,
   .textColor = COLOR_TEXT_DARK};

  static const Clay_TextElementConfig textConfigMedium = {
   .fontId = FONT_ID_LATO,
   .fontSize = 24,
   .letterSpacing = FONT_DEFAULT_SPACING, // default spacing
   .textColor = COLOR_TEXT_LIGHT};

  game_t game;
  game_init(&game);
  board_randomize(game.board);
  inventory_fill(game.inventory, 5);

  Camera2D camera = {0};
  camera.target = (Vector2){0.0f, 0.0f};
  camera.offset = (Vector2){GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};
  camera.zoom = 1.0f;

  Clay_Raylib_Initialize(
   initial_width, initial_height, "HexHex",
   FLAG_WINDOW_HIGHDPI | FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT |
    FLAG_VSYNC_HINT); // Extra parameters to this function are new
  // since the video was published
  // Window initialization

  Font fonts[MAX_FONTS] = {0};

  fonts[0] = LoadFont("resources/sprite_fonts/alagard.png");
  fonts[1] = LoadFont("resources/sprite_fonts/pixelplay.png");
  fonts[2] = LoadFont("resources/sprite_fonts/mecha.png");
  fonts[3] = LoadFont("resources/sprite_fonts/setback.png");
  fonts[4] = LoadFont("resources/sprite_fonts/romulus.png");
  fonts[5] = LoadFont("resources/sprite_fonts/pixantiqua.png");
  fonts[6] = LoadFont("resources/sprite_fonts/alpha_beta.png");
  fonts[7] = LoadFont("resources/sprite_fonts/jupiter_crash.png");

  uint64_t clayRequiredMemory = Clay_MinMemorySize();

  Clay_Arena clayMemory = (Clay_Arena){
   .memory = malloc(clayRequiredMemory),
   .capacity = clayRequiredMemory,
  };

  Clay_Initialize(
   clayMemory,
   (Clay_Dimensions){.width = GetScreenWidth(), .height = GetScreenHeight()},
   (Clay_ErrorHandler){HandleClayErrors});

  Clay_SetMeasureTextFunction(Clay_Raylib_MeasureText, fonts);
  while (!WindowShouldClose()) {
    input_state_t input;
    get_input_state(&input);

    Clay_SetLayoutDimensions(
     (Clay_Dimensions){.width = GetScreenWidth(), .height = GetScreenHeight()});

    Clay_BeginLayout();

    CLAY({.id = CLAY_ID("main"),
          .layout = {.layoutDirection = CLAY_LEFT_TO_RIGHT,
                     .sizing = (Clay_Sizing){.width = CLAY_SIZING_GROW(),
                                             .height = CLAY_SIZING_GROW()},
                     .childGap = 5,
                     .padding = CLAY_PADDING_ALL(8)},

          .backgroundColor = COLOR_BUTTON_SECONDARY}) {

      CLAY({
       .id = CLAY_ID("right"),
       .layout =
        {
         .sizing = (Clay_Sizing){.height = CLAY_SIZING_GROW(),
                                 .width = CLAY_SIZING_PERCENT(0.8)},
        },
       .cornerRadius = 5,
       .backgroundColor = COLOR_BUTTON_PRIMARY

       // Later you’d replace with render_board_in_rect(game.board, r);

      }){

      };

      CLAY({
       .id = CLAY_ID("left"),
       .cornerRadius = 5,
       .layout = {.sizing = (Clay_Sizing){.height = CLAY_SIZING_GROW(),
                                          .width = CLAY_SIZING_PERCENT(0.2)}},
       .backgroundColor = COLOR_PANEL,
       // Clay_OnHover(HandleButtonInteraction, &buttonData);
      });
    };

    Clay_RenderCommandArray renderCommands = Clay_EndLayout();
    update_camera(&camera, &input);
    BeginDrawing();
    ClearBackground(WHITE);
    Clay_Raylib_Render(renderCommands, fonts);
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