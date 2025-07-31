#include "../board/board.h"
#include "../game/inventory.h"
//#include "../controller/game_controller.h"

#include <stdbool.h>

typedef struct {
    board_t *board;
    inventory_t *inventory;
}game_t;

game_t *game_create(void);

void free_game(game_t* game);
