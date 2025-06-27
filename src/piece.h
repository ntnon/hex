#include "hex_grid.h"
#include "tile.h"

typedef enum {
    piece_SHAPE_SINGLE,
    piece_SHAPE_PAIR,
    piece_SHAPE_TRIPLE,
    piece_SHAPE_QUAD,
    piece_SHAPE_H,
    piece_SHAPE_LINE_3,
    piece_SHAPE_LINE_4,
    piece_SHAPE_L_RIGHT,
    piece_SHAPE_L_LEFT,
    piece_SHAPE_DIAMOND,
    piece_SHAPE_SEPT,
    piece_SHAPE_T,
    piece_TYPE_COUNT
} piece_type;

typedef struct pieceShape  {
    piece_type name;
    Hex* hex_array;
} pieceShape;

typedef struct piece {
    int id;
    int tileCount;
    tile_array tiles;
    pieceShape shape;
} piece;

typedef struct {
    int size;
    piece* pieces;
} piece_array;

pieceShape get_piece_shape_from_type(piece_type type);

piece_array piece_array_create();
void piece_array_free(piece_array* array);

Hex get_piece_anchor(piece* piece);

//size of shape must be equal to tileCount, or else this function will fail
piece piece_create(piece* piece, pieceShape shape, tile_array tiles);

Hex* piece_type_to_hex_array(piece_type type);

piece piece_move(piece* piece, Hex b);

piece piece_rotate(piece* piece, int direction);

bool validate_piece(piece* piece);

void piece_free(piece* piece);
