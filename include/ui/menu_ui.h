#ifndef MENU_UI_H
#define MENU_UI_H

#include "ui_context.h"
#include "../controller/input_state.h"
#include "../screen/menu_screen.h"

// Initialize menu UI state
void menu_ui_init(ui_context_t* ctx);

// Draw menu UI using Clay
void menu_ui_draw(ui_context_t* ctx);

// Handle menu UI input and return the action to take
menu_action_t menu_ui_update(ui_context_t* ctx, input_state_t* input);

// Reset menu UI state
void menu_ui_cleanup(ui_context_t* ctx);

#endif // MENU_UI_H