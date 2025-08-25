#include "ui.h"

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
const uint32_t FONT_ID_LATO = 5;

// Text configs
const Clay_TextElementConfig TEXT_CONFIG_LARGE = {.fontId = FONT_ID_LATO,
                                                  .fontSize = 24,
                                                  .letterSpacing =
                                                    FONT_DEFAULT_SPACING,
                                                  .textColor = M_BLACK};

const Clay_TextElementConfig TEXT_CONFIG_MEDIUM = {.fontId = FONT_ID_LATO,
                                                   .fontSize = 24,
                                                   .letterSpacing =
                                                     FONT_DEFAULT_SPACING,
                                                   .textColor = M_GRAY};

Font UI_FONTS[MAX_FONTS] = {0};

void ui_load_fonts(void) {
  UI_FONTS[0] = LoadFont("resources/sprite_fonts/alagard.png");
  UI_FONTS[1] = LoadFont("resources/sprite_fonts/pixelplay.png");
  UI_FONTS[2] = LoadFont("resources/sprite_fonts/mecha.png");
  UI_FONTS[3] = LoadFont("resources/sprite_fonts/setback.png");
  UI_FONTS[4] = LoadFont("resources/sprite_fonts/romulus.png");
  UI_FONTS[5] = LoadFont("resources/sprite_fonts/pixantiqua.png");
  UI_FONTS[6] = LoadFont("resources/sprite_fonts/alpha_beta.png");
  UI_FONTS[7] = LoadFont("resources/sprite_fonts/jupiter_crash.png");
}
