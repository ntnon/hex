#ifndef UI_H
#define UI_H

#include "third_party/clay.h"
#include "raylib.h"
#include "controller/app_controller.h"
#include "controller/game_controller.h"
#include "game/game.h"
#include "ui_types.h"
#include "utility/geometry.h"
#include <stdint.h>

/* ============================================================================
 * UI Layout System
 * ============================================================================ */



bool is_id_valid(const Clay_ElementId id);
void ui_game_screen(game_controller_t *game_controller, const input_state_t *input);
void ui_game(game_controller_t *game_controller, const input_state_t *input);




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
#define MAX_FONTS 9
extern const uint32_t FONT_ID_LATO;
extern Font UI_FONTS[MAX_FONTS];

void ui_load_fonts(void);
void ui_init_text_configs(void);

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

void ui_hover_handler(Clay_ElementId elementId, Clay_PointerData pointerData, intptr_t userData);
void handle_menu_button_hover(Clay_ElementId elementId, Clay_PointerData pointerData, intptr_t userData);
Clay_ElementId ui_get_hovered_element(void);
bool ui_is_hovered(Clay_ElementId elementId);
bool ui_was_clicked(Clay_ElementId elementId);
bool ui_was_clicked_any(void);
void ui_clear_click(void);
void game_screen(game_controller_t *controller);

/* App-level UI functions */
void ui_main_menu(app_controller_t *app_controller);
void ui_settings_menu(app_controller_t *app_controller);
void ui_pause_menu(app_controller_t *app_controller);
void ui_tile_info_card(game_controller_t *controller, Vector2 mouse_pos);
void ui_game(game_controller_t *game_controller, const input_state_t *input);
bounds_t ui_get_element_bounds(Clay_ElementId elementId);

Clay_RenderCommandArray ui_root(app_controller_t *app_controller, const input_state_t *input);
#endif // UI_H
