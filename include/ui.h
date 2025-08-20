#include "third_party/clay.h"
#include "raylib.h"

#ifndef UI_STYLES_H
#define UI_STYLES_H

extern const Clay_Color M_LIGHTGRAY  ;
extern const Clay_Color M_GRAY       ;
extern const Clay_Color M_DARKGRAY   ;
extern const Clay_Color M_YELLOW     ;
extern const Clay_Color M_GOLD       ;
extern const Clay_Color M_ORANGE     ;
extern const Clay_Color M_PINK       ;
extern const Clay_Color M_RED        ;
extern const Clay_Color M_MAROON     ;
extern const Clay_Color M_GREEN      ;
extern const Clay_Color M_LIME       ;
extern const Clay_Color M_DARKGREEN  ;
extern const Clay_Color M_SKYBLUE    ;
extern const Clay_Color M_BLUE       ;
extern const Clay_Color M_DARKBLUE   ;
extern const Clay_Color M_PURPLE     ;
extern const Clay_Color M_VIOLET     ;
extern const Clay_Color M_DARKPURPLE ;
extern const Clay_Color M_BEIGE      ;
extern const Clay_Color M_BROWN      ;
extern const Clay_Color M_DARKBROWN  ;
extern const Clay_Color M_WHITE      ;
extern const Clay_Color M_BLACK      ;
extern const Clay_Color M_BLANK      ;
extern const Clay_Color M_MAGENTA    ;
extern const Clay_Color M_RAYWHITE   ;

// Fonts
#define FONT_DEFAULT_SPACING 3
#define MAX_FONTS 8
extern const uint32_t FONT_ID_LATO;
extern Font UI_FONTS[MAX_FONTS];

void ui_load_fonts(void);


// Text configs
extern const Clay_TextElementConfig TEXT_CONFIG_LARGE;
extern const Clay_TextElementConfig TEXT_CONFIG_MEDIUM;




#endif // UI_STYLES_H


#ifndef UI_LAYOUT_H
#define UI_LAYOUT_H

#include "game/game.h"

// Build and return the layout for the main UI
Clay_RenderCommandArray ui_build_layout(game_t *game);

#endif // UI_LAYOUT_H


#ifndef UI_INIT_H
#define UI_INIT_H

#include "third_party/clay.h"
#include <stdint.h>

typedef struct {
    Clay_Arena arena;
} UI_Context;

// Initialize Clay and memory arena
UI_Context ui_init(int screen_width, int screen_height);

// Shutdown Clay and free memory
void ui_shutdown(UI_Context *ctx);

#endif // UI_INIT_H
