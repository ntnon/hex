#include "third_party/clay.h"
#include "raylib.h"
#include "game/game_controller.h"
#include <stdint.h>
#include "ui_types.h"

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

// Build and return the layout for the main UI
Clay_RenderCommandArray ui_build_layout(game_controller_t *controller);

#endif // UI_LAYOUT_H


#ifndef UI_INIT_H
#define UI_INIT_H


typedef struct {
    Clay_Arena arena;
} UI_Context;

// Initialize Clay and memory arena
UI_Context ui_init(int screen_width, int screen_height);

// Shutdown Clay and free memory
void ui_shutdown(UI_Context *ctx);

#endif // UI_INIT_H

#ifndef UI_EVENT_H
#define UI_EVENT_H

#define MAX_UI_EVENTS 64

void handle_hover(Clay_ElementId elementId, Clay_PointerData pointer, intptr_t userData);
ui_event_t ui_poll_event(void);
void ui_push_event(ui_event_t evt);
void ui_clear_events(void);
#endif // UI_EVENT_H
