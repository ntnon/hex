#ifndef PIECE_TYPE_H
#define PIECE_TYPE_H

#include "hex_grid.h"

typedef enum {
    PIECE_SHAPE_SINGLE,
    PIECE_SHAPE_PAIR,
    PIECE_SHAPE_TRIPLE,
    PIECE_SHAPE_QUAD,
    PIECE_SHAPE_H,
    PIECE_SHAPE_LINE_3,
    PIECE_SHAPE_LINE_4,
    PIECE_SHAPE_L_RIGHT,
    PIECE_SHAPE_L_LEFT,
    PIECE_SHAPE_DIAMOND,
    PIECE_SHAPE_SEPT,
    PIECE_SHAPE_T,
    PIECE_TYPE_COUNT
} piece_type;

typedef struct {
    piece_type name;
    const hex *hex_array; // Pointer to an array of hex objects
} piece_shape;

typedef struct {
    const hex *array;
    int size;
} piece_type_info;

typedef struct {
    int id;
    int count;
    int capacity;
    piece_shape *shapes; // Pointer to an array of piece_shape objects
} piece;

// Function declarations
piece_shape get_piece_shape_from_type(piece_type type);
hex get_piece_anchor(const piece* piece);
const hex* piece_type_to_hex_array(piece_type type);
piece* piece_move(piece* piece, hex b);
piece* piece_rotate(piece* piece, int direction);
bool validate_piece(const piece* piece);
void piece_free(piece* piece);

#endif
