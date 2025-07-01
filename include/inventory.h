#include "piece.h"

typedef struct {
    int hover_index;
    int selected_index;
} inventory;

inventory *inventory_create(void);
void add_piece(piece piece);
void remove_piece(piece piece);
void select_piece(int index);
