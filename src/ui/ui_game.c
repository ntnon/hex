#include "controller/input_state.h"
#include "game/inventory.h"
#include "raylib.h"
#include "stdio.h"
#include "tile/tile.h"
#include "ui.h"
#include "ui_types.h"

#define INVENTORY_PADDING 10
#define INVENTORY_GAP INVENTORY_PADDING

bool is_id_valid(const Clay_ElementId id) { return id.id != ID_NONE.id; }

static void ui_add_inventory_button(game_controller_t *controller);
static void ui_reward_area(game_controller_t *controller);
static void ui_reward(game_controller_t *controller);
static void ui_tool_bar(game_controller_t *controller);

void ButtonComponent(Clay_String buttonText, Clay_ElementId id) {
    // Red box button with 8px of padding
    CLAY_AUTO_ID(
      {.layout = {.padding = CLAY_PADDING_ALL(8)}, .backgroundColor = M_RED}) {
        CLAY_TEXT(buttonText, &TEXT_CONFIG_MEDIUM);
        Clay_OnHover(ui_hover_handler, 0);
    }
}

static void ui_game_area(game_controller_t *game_controller) {
    CLAY(ID_GAME_AREA,
         {.cornerRadius = 0,
          .backgroundColor = {0, 0, 0, 1}, // Very faint black to make it "real"
          // .clip = {.horizontal = true, .vertical = true},  // Disabled for
          // testing
          .layout = {.sizing = (Clay_Sizing){.height = CLAY_SIZING_GROW(),
                                             .width = CLAY_SIZING_GROW()}}}) {
        Clay_OnHover(ui_hover_handler, 0);
        ui_tool_bar(game_controller);
        // Add a tiny invisible child to ensure the element has content
        CLAY_AUTO_ID({.layout = {.sizing = {.width = CLAY_SIZING_FIXED(1),
                                            .height = CLAY_SIZING_FIXED(1)}}}) {
        }
    }
}

static void ui_add_inventory_button(game_controller_t *controller) {
    CLAY(UI_BUTTON_ADD_INVENTORY_ITEM,
         {.backgroundColor = M_ORANGE,
          .layout = {.sizing = (Clay_Sizing){.width = CLAY_SIZING_FIT(),
                                             .height = CLAY_SIZING_FIT()}}}) {
        CLAY_TEXT(CLAY_STRING("Add to inventory"), &TEXT_CONFIG_MEDIUM);
        Clay_OnHover(ui_hover_handler, 0);
    }
}

static void ui_inventory_item(game_controller_t *controller,
                              inventory_item_t item, int index,
                              int inventory_size, float item_height) {
    bool is_selected = (controller->game->inventory->selected_index == index);
    Clay_Color bg_color = is_selected ? M_ORANGE : M_BEIGE;

    CLAY(item.id,
         {.backgroundColor = bg_color,
          .aspectRatio = 1.0,
          .clip = true,
          .layout = {.layoutDirection = CLAY_TOP_TO_BOTTOM,

                     .sizing = (Clay_Sizing){.height = CLAY_SIZING_GROW(),
                                             .width = CLAY_SIZING_GROW()}}}) {
        Clay_OnHover(ui_hover_handler, 0);
    }
}

void ui_inventory_area(game_controller_t *controller){
  CLAY(ID_INVENTORY_AREA,
       {.cornerRadius = 0,
        .backgroundColor = M_GRAY,
        .layout = {.childGap = INVENTORY_GAP,
                   .childAlignment = CLAY_ALIGN_X_CENTER,
                   .padding = CLAY_PADDING_ALL(INVENTORY_PADDING),
                   .layoutDirection = CLAY_TOP_TO_BOTTOM,
                   .sizing = (Clay_Sizing){.width = CLAY_SIZING_GROW(4, 200),
                                           .height = CLAY_SIZING_GROW()}}}){
    Clay_OnHover(ui_hover_handler, 0);
ui_add_inventory_button(controller);

int inventory_size = inventory_get_size(controller->game->inventory);
int total_height = GetScreenHeight();
float available_height =
  total_height - 2 * INVENTORY_PADDING - INVENTORY_GAP * (inventory_size - 1);
float item_height = available_height / inventory_size;

for (int i = 0; i < inventory_size; i++) {
    inventory_item_t item = inventory_get_item(controller->game->inventory, i);
    ui_inventory_item(controller, item, i, inventory_size, item_height);
}
}
}
;

static void ui_reward_area(game_controller_t *controller) {
    CLAY(ID_REWARDS,
         {.backgroundColor = M_GRAY,

          .layout = {.layoutDirection = CLAY_LEFT_TO_RIGHT,
                     .childAlignment.y = CLAY_ALIGN_Y_CENTER,
                     .sizing = (Clay_Sizing){.width = CLAY_SIZING_GROW(),
                                             .height = CLAY_SIZING_GROW()},
                     .childGap = 40,
                     .padding = CLAY_PADDING_ALL(40)}}) {
        ui_reward(controller);
    }
}

static void ui_reward(game_controller_t *controller) {
    for (int i = 0; i < controller->game->reward_count; i++) {
        CLAY(CLAY_IDI(ID_REWARD_BASE_STRING, i),
             {.backgroundColor = M_BEIGE,
              .aspectRatio = 1.0,
              .layout = {.layoutDirection = CLAY_LEFT_TO_RIGHT,
                         .sizing = (Clay_Sizing){.width = CLAY_SIZING_GROW(),
                                                 .height = CLAY_SIZING_GROW()},
                         .childGap = 5,
                         .padding = CLAY_PADDING_ALL(8)}}) {
            // Reward content would go here
        }
    }
};

void ui_tile_info_card(game_controller_t *controller, Vector2 mouse_pos) {
    if (!controller->should_show_tile_info || !controller->hovered_tile) {
        return;
    }

    tile_t *tile = controller->hovered_tile;
    game_t *game = controller->game;
    pool_t *pool = pool_manager_get_pool_by_tile(game->board->pools, tile);
    int score = pool ? pool_tile_score(pool) : 0;
    if (pool) {
        pool_print(pool);
    }

    // Position the info card near the mouse, but keep it on screen
    float card_width = 200;
    float card_height = 120;
    float screen_width = GetScreenWidth();
    float screen_height = GetScreenHeight();

    // Default position: offset from mouse
    float card_x = mouse_pos.x + 20;
    float card_y = mouse_pos.y - card_height / 2;

    // Adjust if card would go off screen
    if (card_x + card_width > screen_width) {
        card_x = mouse_pos.x - card_width - 20;
    }
    if (card_y < 0) {
        card_y = 10;
    } else if (card_y + card_height > screen_height) {
        card_y = screen_height - card_height - 10;
    }

    CLAY(ID_TILE_INFO_CARD,
         {
           .floating = {.attachTo = CLAY_ATTACH_TO_ROOT,
                        .offset = {.x = (int)card_x, .y = (int)card_y},
                        .zIndex = 1000,
                        .pointerCaptureMode =
                          CLAY_POINTER_CAPTURE_MODE_PASSTHROUGH},
           .layout = {.sizing = {.width = CLAY_SIZING_FIT(),
                                 .height = CLAY_SIZING_FIT()},
                      .padding = CLAY_PADDING_ALL(12),
                      .childGap = 8,
                      .layoutDirection = CLAY_TOP_TO_BOTTOM},
         }) {

        // Tile card
        CLAY_AUTO_ID(
          {.layout = {.sizing = {.width = CLAY_SIZING_GROW(),
                                 .height = CLAY_SIZING_GROW()},
                      .padding = CLAY_PADDING_ALL(12),
                      .childGap = 8,
                      .layoutDirection = CLAY_TOP_TO_BOTTOM},

           .backgroundColor = (Clay_Color){40, 40, 40, 240},
           .cornerRadius = CLAY_CORNER_RADIUS(6),
           .border = {.color = (Clay_Color){80, 80, 80, 255}, .width = 1}}) {
            switch (tile->data.type) {
            case TILE_MAGENTA:
                CLAY_TEXT(CLAY_STRING("Magenta tile"), &TEXT_CONFIG_MEDIUM);
                break;
            case TILE_CYAN:
                CLAY_TEXT(CLAY_STRING("Cyan tile"), &TEXT_CONFIG_MEDIUM);
                break;
            case TILE_YELLOW:
                CLAY_TEXT(CLAY_STRING("Yellow tile"), &TEXT_CONFIG_MEDIUM);
                break;
            case TILE_GREEN:
                CLAY_TEXT(CLAY_STRING("Green tile"), &TEXT_CONFIG_MEDIUM);
                break;
            default:
                CLAY_TEXT(CLAY_STRING("Unknown tile"), &TEXT_CONFIG_MEDIUM);
                break;
            }

            static char value_text[32];
            snprintf(value_text, sizeof(value_text), "Value: %d",
                     tile->data.value);
            Clay_String value_string = {.chars = value_text,
                                        .length = strlen(value_text)};
            CLAY_TEXT(value_string, &TEXT_CONFIG_MEDIUM);

            static char tile_modifier_text[32];
            snprintf(tile_modifier_text, sizeof(tile_modifier_text),
                     "Modifier: %.2f\n", tile_get_modifier(tile));
            Clay_String tile_modifier = {.chars = tile_modifier_text,
                                         .length = strlen(tile_modifier_text)};
            CLAY_TEXT(tile_modifier, &TEXT_CONFIG_MEDIUM);
        }

        // Pool card
        if (pool) {
            CLAY_AUTO_ID({.layout = {.sizing = {.width = CLAY_SIZING_GROW(),
                                                .height = CLAY_SIZING_GROW()},
                                     .padding = CLAY_PADDING_ALL(12),
                                     .childGap = 8,
                                     .layoutDirection = CLAY_TOP_TO_BOTTOM},

                          .backgroundColor = (Clay_Color){40, 40, 40, 240},
                          .cornerRadius = CLAY_CORNER_RADIUS(6),
                          .border = {.color = (Clay_Color){80, 80, 80, 255},
                                     .width = 1}}) {
                CLAY_TEXT(CLAY_STRING("Pool"), &TEXT_CONFIG_MEDIUM);

                static char pool_text[32];
                snprintf(pool_text, sizeof(pool_text), "Tiles: %u", score);
                Clay_String pool_score = {.chars = pool_text,
                                          .length = strlen(pool_text)};
                CLAY_TEXT(pool_score, &TEXT_CONFIG_MEDIUM);

                static char pool_modifier_text[32];
                snprintf(pool_modifier_text, sizeof(pool_modifier_text),
                         "Modifier: %.2f", pool_get_modifier(pool));
                Clay_String pool_modifier = {.chars = pool_modifier_text,
                                             .length =
                                               strlen(pool_modifier_text)};
                CLAY_TEXT(pool_modifier, &TEXT_CONFIG_MEDIUM);

                static char neighbor_tile_count[32];
                snprintf(neighbor_tile_count, sizeof(neighbor_tile_count),
                         "Neighbors: %zu", pool->neighbor_tiles.n);
                Clay_String neighbor_tiles = {.chars = neighbor_tile_count,
                                              .length =
                                                strlen(neighbor_tile_count)};
                CLAY_TEXT(neighbor_tiles, &TEXT_CONFIG_MEDIUM);

                // Geometric properties with user-friendly names
                // static char span_text[32];
                // snprintf(span_text, sizeof(span_text), "Span: %d",
                //          pool->diameter);
                // Clay_String span = {.chars = span_text,
                //                     .length = strlen(span_text)};
                // CLAY_TEXT(span, &TEXT_CONFIG_MEDIUM);

                // static char edge_text[32];
                // snprintf(edge_text, sizeof(edge_text), "Edges: %d",
                //          pool->edge_count);
                // Clay_String edge = {.chars = edge_text,
                //                     .length = strlen(edge_text)};
                // CLAY_TEXT(edge, &TEXT_CONFIG_MEDIUM);

                // static char compactness_text[32];
                // snprintf(compactness_text, sizeof(compactness_text),
                //          "Compactness: %.2f", pool->compactness_score);
                // Clay_String compactness = {.chars = compactness_text,
                //                            .length =
                //                            strlen(compactness_text)};
                // CLAY_TEXT(compactness, &TEXT_CONFIG_MEDIUM);
            }
        }
    }
}

void ui_tool_bar(game_controller_t *game_controller) {

    CLAY(ID_TOOL_BAR, {.backgroundColor = M_BLACK,
                       .layout = {
                         .layoutDirection = CLAY_LEFT_TO_RIGHT,
                         .sizing = (Clay_Sizing){.width = CLAY_SIZING_GROW(),
                                                 .height = CLAY_SIZING_FIT()},
                       }}) {
        ButtonComponent(CLAY_STRING("Calculate"),
                        ID_GENERATION_INCREASE_BUTTON);
        CLAY_AUTO_ID({
          .backgroundColor = M_BLUE,
        }) {}
    }
}

void ui_game_screen(game_controller_t *game_controller,
                    const input_state_t *input) {
    CLAY({
      .id = ID_GAME_SCREEN,
      .layout =
        {
          .childGap = 10,
          .layoutDirection = CLAY_TOP_TO_BOTTOM,
          .sizing = (Clay_Sizing){.width = CLAY_SIZING_GROW(),
                                  .height = CLAY_SIZING_GROW()},
        },

    });
    {
        ui_tool_bar(game_controller);
        ui_game(game_controller, input);
    }
}

void ui_game(game_controller_t *game_controller, const input_state_t *input) {

    CLAY(ID_GAME, {.backgroundColor = M_BLANK,
                   .layout = {
                     .layoutDirection = CLAY_LEFT_TO_RIGHT,
                     .sizing = (Clay_Sizing){.width = CLAY_SIZING_GROW(),
                                             .height = CLAY_SIZING_GROW()},
                   }}) {

        if (true) {
            ui_game_area(game_controller);
            ui_inventory_area(game_controller);
            ui_tile_info_card(game_controller,
                              (Vector2){input->mouse.x, input->mouse.y});
        } else { // PREVENTS CONDITIONAL RENDERING

            switch (game_controller->state) {
            case GAME_STATE_VIEW:
                ui_game_area(game_controller);
                ui_inventory_area(game_controller);
                ui_tile_info_card(game_controller,
                                  (Vector2){input->mouse.x, input->mouse.y});
                break;
            case GAME_STATE_PLACE:
                ui_game_area(game_controller);
                ui_inventory_area(game_controller);
                // Add tile info card overlay

                break;
            case GAME_STATE_REWARD:
                ui_reward_area(game_controller);
                break;
            case GAME_STATE_INSPECT:
                ui_game_area(game_controller);
                ui_inventory_area(game_controller);
                // Add tile info card overlay
                ui_tile_info_card(game_controller,
                                  (Vector2){input->mouse.x, input->mouse.y});
                break;
            default:
                break;
            }
        }
    }
}
