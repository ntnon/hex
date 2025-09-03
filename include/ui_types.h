#ifndef UI_TYPES_H
#define UI_TYPES_H

#include "third_party/clay.h"

#define UI_ID_NONE CLAY_ID("none")
#define UI_ID_MAIN CLAY_ID("main")
#define UI_ID_GAME CLAY_ID("game")

#define UI_ID_INVENTORY CLAY_ID("inventory")
#define UI_ID_INVENTORY_ITEM_BASE_STRING "inventory_item_base_"

#define UI_ID_REWARDS CLAY_ID("rewards")
#define UI_ID_REWARD_BASE_STRING "reward_base_"


#define UI_BUTTON_ADD_INVENTORY_ITEM CLAY_ID("add_item")


typedef enum {
    UI_EVENT_NONE = 0,
    UI_EVENT_HOVER_START,
    UI_EVENT_HOVER_END,
    UI_EVENT_CLICK,
    UI_EVENT_INVENTORY_CLICK,
    UI_EVENT_INVENTORY_ITEM_CLICK,
    UI_EVENT_DRAG_START,
    UI_EVENT_DRAG_END
} ui_event_type_t;

typedef struct {
    ui_event_type_t type;
    Clay_ElementId element_id;
    Clay_ElementData element_data;
} ui_event_t;

#endif // UI_TYPES_H
