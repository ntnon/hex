#include "ui.h"
#include <stdio.h>

// COLORS
const Clay_Color M_LIGHTGRAY = {200, 200, 200, 255}; // Light Gray
const Clay_Color M_GRAY = {130, 130, 130, 255};      // Gray
const Clay_Color M_DARKGRAY = {80, 80, 80, 255};     // Dark Gray
const Clay_Color M_YELLOW = {253, 249, 0, 255};      // Yellow
const Clay_Color M_GOLD = {255, 203, 0, 255};        // Gold
const Clay_Color M_ORANGE = {255, 161, 0, 255};      // Orange
const Clay_Color M_PINK = {255, 109, 194, 255};      // Pink
const Clay_Color M_RED = {230, 41, 55, 255};         // Red
const Clay_Color M_MAROON = {190, 33, 55, 255};      // Maroon
const Clay_Color M_GREEN = {0, 228, 48, 255};        // Green
const Clay_Color M_LIME = {0, 158, 47, 255};         // Lime
const Clay_Color M_DARKGREEN = {0, 117, 44, 255};    // Dark Green
const Clay_Color M_SKYBLUE = {102, 191, 255, 255};   // Sky Blue
const Clay_Color M_BLUE = {0, 121, 241, 255};        // Blue
const Clay_Color M_DARKBLUE = {0, 82, 172, 255};     // Dark Blue
const Clay_Color M_PURPLE = {200, 122, 255, 255};    // Purple
const Clay_Color M_VIOLET = {135, 60, 190, 255};     // Violet
const Clay_Color M_DARKPURPLE = {112, 31, 126, 255}; // Dark Purple
const Clay_Color M_BEIGE = {211, 176, 131, 255};     // Beige
const Clay_Color M_BROWN = {127, 106, 79, 255};      // Brown
const Clay_Color M_DARKBROWN = {76, 63, 47, 255};    // Dark Brown
const Clay_Color M_WHITE = {255, 255, 255, 255};     // White
const Clay_Color M_BLACK = {0, 0, 0, 255};           // Black
const Clay_Color M_BLANK = {0, 0, 0, 0};             // Blank (Transparent)
const Clay_Color M_MAGENTA = {255, 0, 255, 255};     // Magenta
const Clay_Color M_RAYWHITE = {245, 245, 245,
                               255}; // My own White (raylib logo)

// FONTS
const uint32_t FONT_ID_LATO = 0;

// Text configs - initialized after font loading to prevent corruption
Clay_TextElementConfig TEXT_CONFIG_LARGE = {0};
Clay_TextElementConfig TEXT_CONFIG_MEDIUM = {0};

// Function to safely initialize text configs after fonts are loaded
void ui_init_text_configs(void) {
  TEXT_CONFIG_LARGE =
    (Clay_TextElementConfig){.userData = NULL,
                             .textColor = M_BLACK,
                             .fontId = FONT_ID_LATO,
                             .fontSize = 24,
                             .letterSpacing = FONT_DEFAULT_SPACING,
                             .lineHeight = 0, // 0 means use measured height
                             .wrapMode = CLAY_TEXT_WRAP_WORDS};

  TEXT_CONFIG_MEDIUM =
    (Clay_TextElementConfig){.userData = NULL,
                             .textColor = M_WHITE,
                             .fontId = FONT_ID_LATO,
                             .fontSize = 20,
                             .letterSpacing = FONT_DEFAULT_SPACING,
                             .lineHeight = 0, // 0 means use measured height
                             .wrapMode = CLAY_TEXT_WRAP_WORDS};

  printf("Text configs initialized safely\n");
}

Font UI_FONTS[MAX_FONTS] = {0};

void ui_load_fonts(void) {
  // Initialize all fonts to default font first
  Font defaultFont = GetFontDefault();
  for (int i = 0; i < MAX_FONTS; i++) {
    UI_FONTS[i] = defaultFont;
  }

  // Load custom fonts with error checking
  const char *font_paths[MAX_FONTS - 1] = {
    "resources/sprite_fonts/pixelplay.png",
    "resources/sprite_fonts/mecha.png",
    "resources/sprite_fonts/setback.png",
    "resources/sprite_fonts/romulus.png",
    "resources/sprite_fonts/pixantiqua.png",
    "resources/sprite_fonts/alpha_beta.png",
    "resources/sprite_fonts/jupiter_crash.png",
    "resources/sprite_fonts/alagard.png"};

  for (int i = 1; i < MAX_FONTS; i++) {
    Font loadedFont = LoadFont(font_paths[i - 1]);
    // Validate font loaded properly
    if (loadedFont.glyphs != NULL && loadedFont.baseSize > 0) {
      UI_FONTS[i] = loadedFont;
      printf("Successfully loaded font %d: %s\n", i, font_paths[i - 1]);
    } else {
      printf("Warning: Failed to load font %s, using default font\n",
             font_paths[i - 1]);
      // Font already initialized to default above
    }
  }

  printf("Font loading completed. Default font baseSize: %d, glyphs: %p\n",
         UI_FONTS[0].baseSize, UI_FONTS[0].glyphs);
}
