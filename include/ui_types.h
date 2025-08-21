#ifndef UI_TYPES_H
#define UI_TYPES_H

#include "third_party/clay.h"

typedef enum {
    UI_EVENT_NONE = 0,
    UI_EVENT_HOVER_START,
    UI_EVENT_HOVER_END,
    UI_EVENT_CLICK,
    UI_EVENT_DRAG_START,
    UI_EVENT_DRAG_END
} ui_event_type_t;

typedef struct {
    ui_event_type_t type;
    Clay_ElementId element_id;
    Clay_ElementData element_data;
} ui_event_t;

#endif // UI_TYPES_H
