#include "third_party/clay_renderer_raylib.h"
#include "ui.h"
#include <stdio.h>
#include <stdlib.h>

static void HandleClayErrors(Clay_ErrorData errorData) {
  printf("Clay Error: %s\n", errorData.errorText.chars);
}

UI_Context ui_init(int screen_width, int screen_height) {
  ui_load_fonts();
  UI_Context ctx = {0};

  Clay_Raylib_Initialize(screen_width, screen_height, "HexHex",
                         FLAG_WINDOW_HIGHDPI | FLAG_MSAA_4X_HINT |
                           FLAG_VSYNC_HINT);

  // Allocate memory for Clay
  uint64_t clayRequiredMemory = Clay_MinMemorySize();
  ctx.arena.memory = malloc(clayRequiredMemory);
  ctx.arena.capacity = clayRequiredMemory;

  Clay_Initialize(
    ctx.arena,
    (Clay_Dimensions){.width = screen_width, .height = screen_height},
    (Clay_ErrorHandler){HandleClayErrors});

  Clay_SetMeasureTextFunction(Clay_Raylib_MeasureText, UI_FONTS);

  return ctx;
}

void ui_shutdown(UI_Context *ctx) {
  if (ctx && ctx->arena.memory) {
    Clay_Raylib_Close();
    free(ctx->arena.memory);
    ctx->arena.memory = NULL;
    ctx->arena.capacity = 0;
  }
}
