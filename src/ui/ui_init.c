#include "third_party/clay_renderer_raylib.h"
#include "ui.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static void HandleClayErrors(Clay_ErrorData errorData) {
  printf("Clay Error: %s\n", errorData.errorText.chars);
}

// Safe wrapper around Clay_Raylib_MeasureText with validation
static Clay_Dimensions SafeMeasureText(Clay_StringSlice text,
                                       Clay_TextElementConfig *config,
                                       void *userData) {
  // Validate input parameters - use fallback for any corruption
  if (!config || !userData || config->fontId >= MAX_FONTS ||
      config->fontSize == 0 || config->fontSize > 200 || text.length < 0 ||
      text.length > 10000 || !text.chars) {
    // Silent fallback to prevent log spam
    return (Clay_Dimensions){.width = (text.length > 0 ? text.length : 10) * 8,
                             .height = 16};
  }

  Font *fonts = (Font *)userData;

  // Additional safety check - ensure fontId is exactly 0 for now
  uint32_t safeFontId = 0;
  Font fontToUse = fonts[safeFontId];

  // Validate font data
  if (!fontToUse.glyphs || fontToUse.baseSize <= 0) {
    return (Clay_Dimensions){.width = text.length * 8, .height = 16};
  }

  // Create a safe config copy to prevent corruption
  Clay_TextElementConfig safeConfig = {
    .userData = NULL,
    .textColor = config->textColor,
    .fontId = safeFontId,
    .fontSize =
      (config->fontSize > 0 && config->fontSize <= 72) ? config->fontSize : 16,
    .letterSpacing = (config->letterSpacing >= 0 && config->letterSpacing <= 10)
                       ? config->letterSpacing
                       : 0,
    .lineHeight = 0,
    .wrapMode = CLAY_TEXT_WRAP_WORDS};

  // Check text is safe
  for (int i = 0; i < text.length; i++) {
    if (text.chars[i] < 32 || text.chars[i] > 126) {
      return (Clay_Dimensions){.width = text.length * 8,
                               .height = safeConfig.fontSize};
    }
  }

  // Call the original function with validated parameters
  return Clay_Raylib_MeasureText(text, &safeConfig, userData);
}

static bool clay_initialized = false;

UI_Context ui_init(int screen_width, int screen_height) {
  UI_Context ctx = {0};

  Clay_Raylib_Initialize(screen_width, screen_height, "HexHex",
                         FLAG_WINDOW_HIGHDPI | FLAG_MSAA_4X_HINT |
                           FLAG_VSYNC_HINT);
  clay_initialized = true;

  // Load fonts after window/graphics initialization
  ui_load_fonts();
  printf("Fonts loaded after graphics initialization\n");

  // Initialize text configs after fonts are loaded
  ui_init_text_configs();

  // Validate that fonts are properly loaded before Clay initialization
  extern Font UI_FONTS[];
  if (!UI_FONTS[0].glyphs || UI_FONTS[0].baseSize <= 0) {
    printf("ERROR: Font 0 not properly loaded, cannot continue\n");
    return ctx;
  }

  // Additional validation - check that glyph data is properly aligned
  for (int i = 0; i < MAX_FONTS; i++) {
    if (UI_FONTS[i].glyphs != NULL) {
      // Check memory alignment of glyph data
      uintptr_t glyphAddr = (uintptr_t)UI_FONTS[i].glyphs;
      if (glyphAddr % 8 != 0) {
        printf("Warning: Font %d glyph data is not 8-byte aligned (addr: %p)\n",
               i, UI_FONTS[i].glyphs);
      }
    }
  }
  printf("Font validation passed\n");

  // Allocate memory for Clay with proper alignment
  uint64_t clayRequiredMemory = Clay_MinMemorySize();
  // Use malloc with manual alignment check instead of aligned_alloc for
  // compatibility
  ctx.arena.memory = malloc(clayRequiredMemory + 8);
  if (!ctx.arena.memory) {
    printf("ERROR: Failed to allocate memory for Clay\n");
    return ctx;
  }

  // Ensure 8-byte alignment
  uintptr_t addr = (uintptr_t)ctx.arena.memory;
  if (addr % 8 != 0) {
    // Adjust pointer to be 8-byte aligned
    addr = (addr + 7) & ~7;
    ctx.arena.memory = (void *)addr;
  }
  ctx.arena.capacity = clayRequiredMemory;
  printf("Clay memory allocated: %llu bytes at %p\n", clayRequiredMemory,
         ctx.arena.memory);

  Clay_Initialize(
    ctx.arena,
    (Clay_Dimensions){.width = screen_width, .height = screen_height},
    (Clay_ErrorHandler){HandleClayErrors});

  Clay_SetMeasureTextFunction(SafeMeasureText, UI_FONTS);

  return ctx;
}

void ui_shutdown(UI_Context *ctx) {
  if (ctx && ctx->arena.memory) {
    // Let CloseWindow handle Clay cleanup to avoid double-free
    clay_initialized = false;
    free(ctx->arena.memory);
    ctx->arena.memory = NULL;
    ctx->arena.capacity = 0;

    // Unload custom fonts (font 0 is default, don't unload it)
    extern Font UI_FONTS[];
    for (int i = 1; i < MAX_FONTS; i++) {
      if (UI_FONTS[i].glyphs != NULL &&
          UI_FONTS[i].glyphs != UI_FONTS[0].glyphs) {
        UnloadFont(UI_FONTS[i]);
        UI_FONTS[i] = UI_FONTS[0]; // Reset to default
      }
    }
  }
}
