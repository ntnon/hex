#ifndef UI_TYPES_H
#define UI_TYPES_H

#include "third_party/clay.h"

#define ID_NONE CLAY_ID("none")
#define ID_ROOT CLAY_ID("root")
#define ID_GAME CLAY_ID("game")
#define ID_GAME_AREA CLAY_ID("game_area")
#define ID_TOOL_BAR CLAY_ID("tool_bar")
#define ID_GAME_SCREEN CLAY_ID("game_screen")

#define ID_INVENTORY_AREA CLAY_ID("inventory_area")
#define ID_INVENTORY_ITEM_BASE_STRING "inventory_item_base_"

#define ID_REWARDS CLAY_ID("rewards")
#define ID_REWARD_BASE_STRING "reward_base_"

#define UI_BUTTON_ADD_INVENTORY_ITEM CLAY_ID("add_item")

// Menu UI element IDs
#define ID_MAIN_MENU CLAY_ID("main_menu")
#define ID_MENU_ITEM_NEW_GAME CLAY_ID("menu_new_game")
#define ID_MENU_ITEM_SETTINGS CLAY_ID("menu_settings")
#define ID_MENU_ITEM_QUIT CLAY_ID("menu_quit")

#define ID_SETTINGS_MENU CLAY_ID("settings_menu")
#define ID_SETTINGS_BACK_BUTTON CLAY_ID("settings_back")

#define ID_PAUSE_MENU CLAY_ID("pause_menu")
#define ID_PAUSE_RESUME_BUTTON CLAY_ID("pause_resume")
#define ID_PAUSE_SETTINGS_BUTTON CLAY_ID("pause_settings")
#define ID_PAUSE_QUIT_BUTTON CLAY_ID("pause_quit")

// Tile info card
#define ID_TILE_INFO_CARD CLAY_ID("tile_info_card")



#endif // UI_TYPES_H
