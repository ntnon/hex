#ifndef UI_TYPES_H
#define UI_TYPES_H

#include "third_party/clay.h"

#define UI_ID_NONE CLAY_ID("none")
#define UI_ID_ROOT CLAY_ID("root")
#define UI_ID_GAME CLAY_ID("game")
#define UI_ID_GAME_AREA CLAY_ID("game_area")

#define UI_ID_INVENTORY CLAY_ID("inventory")
#define UI_ID_INVENTORY_ITEM_BASE_STRING "inventory_item_base_"

#define UI_ID_REWARDS CLAY_ID("rewards")
#define UI_ID_REWARD_BASE_STRING "reward_base_"

#define UI_BUTTON_ADD_INVENTORY_ITEM CLAY_ID("add_item")

// Menu UI element IDs
#define UI_ID_MAIN_MENU CLAY_ID("main_menu")
#define UI_ID_MENU_ITEM_NEW_GAME CLAY_ID("menu_new_game")
#define UI_ID_MENU_ITEM_SETTINGS CLAY_ID("menu_settings")
#define UI_ID_MENU_ITEM_QUIT CLAY_ID("menu_quit")

#define UI_ID_SETTINGS_MENU CLAY_ID("settings_menu")
#define UI_ID_SETTINGS_BACK_BUTTON CLAY_ID("settings_back")

#define UI_ID_PAUSE_MENU CLAY_ID("pause_menu")
#define UI_ID_PAUSE_RESUME_BUTTON CLAY_ID("pause_resume")
#define UI_ID_PAUSE_SETTINGS_BUTTON CLAY_ID("pause_settings")
#define UI_ID_PAUSE_QUIT_BUTTON CLAY_ID("pause_quit")

// Tile info card
#define UI_ID_TILE_INFO_CARD CLAY_ID("tile_info_card")



#endif // UI_TYPES_H
