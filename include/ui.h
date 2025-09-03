#ifndef UI_H
#define UI_H

#include "third_party/clay.h"
#include "raylib.h"
#include "controller/game_controller.h"
#include "ui_types.h"
#include <stdint.h>

/* ============================================================================
 * UI Layout System
 * ============================================================================ */



bool is_id_valid(const Clay_ElementId id);
Clay_RenderCommandArray ui_build_layout(game_controller_t *controller);



/* ============================================================================
 * UI Styles and Colors
 * ============================================================================ */

extern const Clay_Color M_LIGHTGRAY;
extern const Clay_Color M_GRAY;
extern const Clay_Color M_DARKGRAY;
extern const Clay_Color M_YELLOW;
extern const Clay_Color M_GOLD;
extern const Clay_Color M_ORANGE;
extern const Clay_Color M_PINK;
extern const Clay_Color M_RED;
extern const Clay_Color M_MAROON;
extern const Clay_Color M_GREEN;
extern const Clay_Color M_LIME;
extern const Clay_Color M_DARKGREEN;
extern const Clay_Color M_SKYBLUE;
extern const Clay_Color M_BLUE;
extern const Clay_Color M_DARKBLUE;
extern const Clay_Color M_PURPLE;
extern const Clay_Color M_VIOLET;
extern const Clay_Color M_DARKPURPLE;
extern const Clay_Color M_BEIGE;
extern const Clay_Color M_BROWN;
extern const Clay_Color M_DARKBROWN;
extern const Clay_Color M_WHITE;
extern const Clay_Color M_BLACK;
extern const Clay_Color M_BLANK;
extern const Clay_Color M_MAGENTA;
extern const Clay_Color M_RAYWHITE;

/* Fonts */
#define FONT_DEFAULT_SPACING 3
#define MAX_FONTS 8
extern const uint32_t FONT_ID_LATO;
extern Font UI_FONTS[MAX_FONTS];

void ui_load_fonts(void);

/* Text configs */
extern Clay_TextElementConfig TEXT_CONFIG_LARGE;
extern Clay_TextElementConfig TEXT_CONFIG_MEDIUM;

/* ============================================================================
 * UI Initialization
 * ============================================================================ */


typedef struct {
    Clay_Arena arena;
} UI_Context;

UI_Context ui_init(int screen_width, int screen_height);
void ui_shutdown(UI_Context *ctx);

/* ============================================================================
 * UI Event System
 * ============================================================================ */

#define MAX_UI_EVENTS 64

void handle_hover(Clay_ElementId elementId, Clay_PointerData pointer, intptr_t userData);
void handle_inventory_click(Clay_ElementId elementId, Clay_PointerData pointerData, intptr_t userData);
void handle_inventory_item_click(Clay_ElementId elementId, Clay_PointerData pointerData, intptr_t userData);
ui_event_t ui_poll_event(void);
void ui_push_event(ui_event_t evt);
void ui_clear_events(void);
void game_screen(game_controller_t *controller);
#endif // UI_H
