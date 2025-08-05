#ifndef SETTINGS_UI_H
#define SETTINGS_UI_H

#include "ui_context.h"
#include "../controller/input_state.h"
#include "../screen/settings_screen.h"

// Initialize settings UI state
void settings_ui_init(ui_context_t* ctx);

// Draw settings UI using Clay
void settings_ui_draw(ui_context_t* ctx);

// Handle settings UI input and return the action to take
settings_action_t settings_ui_update(ui_context_t* ctx, input_state_t* input);

// Reset settings UI state
void settings_ui_cleanup(ui_context_t* ctx);

#endif // SETTINGS_UI_H