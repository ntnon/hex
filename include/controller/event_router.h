#ifndef EVENT_ROUTER_H
#define EVENT_ROUTER_H

#include "ui_types.h"
#include "game/game.h"


/* Forward declarations */
struct game_controller;

typedef struct {
    game_t *game;
    Clay_ElementId last_clicked_element_id;
    Clay_ElementId hovered_element_id;
} event_router_t;

/* Function declarations */
void event_router_init(event_router_t *router, game_t *game);
void event_router_process_events(event_router_t *router);
void event_router_handle_click(event_router_t *router, ui_event_t evt);
void event_router_handle_hover_start(event_router_t *router, ui_event_t evt);
void event_router_handle_hover_end(event_router_t *router, ui_event_t evt);
void event_router_handle_inventory_click(event_router_t *router, ui_event_t evt);

#endif // EVENT_ROUTER_H
