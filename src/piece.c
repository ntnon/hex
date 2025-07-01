#include "piece.h"
#include "hex_grid.h"
#include <stdlib.h>

// Define the hex arrays
const hex emptyArray[0] = {};
const hex single[1] = { { 0, 0, 0 } };
const hex pair[2] = { { 0, 0, 0 }, { 0, 1, 0 } };
const hex triple[3] = { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 } };
const hex quad[4] = { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 }, { 0, 1, 1 } };
const hex line_3[3] = { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 2, 0 } };
const hex line_4[4] = { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 2, 0 }, { 0, 3, 0 } };
const hex h[5]
    = { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 }, { 0, 1, 1 }, { 0, 2, 0 } };
const hex l_right[4] = { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 2, 0 }, { 0, 2, 1 } };
const hex l_left[4] = { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 2, 0 }, { 0, 1, 1 } };
const hex diamond[4] = { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 }, { 0, 1, 1 } };
const hex sept[7] = { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 }, { 0, 1, 1 },
                      { 0, 2, 0 }, { 0, 2, 1 }, { 0, 1, 2 } };

// Function to map piece_type to hex array
const hex *
piece_type_to_hex_array (piece_type type)
{
  switch (type)
    {
    case PIECE_SHAPE_SINGLE:
      return single;
    case PIECE_SHAPE_PAIR:
      return pair;
    case PIECE_SHAPE_TRIPLE:
      return triple;
    case PIECE_SHAPE_QUAD:
      return quad;
    case PIECE_SHAPE_H:
      return h;
    case PIECE_SHAPE_LINE_3:
      return line_3;
    case PIECE_SHAPE_LINE_4:
      return line_4;
    case PIECE_SHAPE_L_RIGHT:
      return l_right;
    case PIECE_SHAPE_L_LEFT:
      return l_left;
    case PIECE_SHAPE_DIAMOND:
      return diamond;
    case PIECE_SHAPE_SEPT:
      return sept;
    default:
      return single; // Default to `single` if the type is unknown
    }
}

// Function to map piece_type to piece_type_info
piece_type_info
piece_type_to_piece_type_info (piece_type type)
{
  switch (type)
    {
    case PIECE_SHAPE_SINGLE:
      return (piece_type_info){ single, 1 };
    case PIECE_SHAPE_PAIR:
      return (piece_type_info){ pair, 2 };
    case PIECE_SHAPE_TRIPLE:
      return (piece_type_info){ triple, 3 };
    case PIECE_SHAPE_QUAD:
      return (piece_type_info){ quad, 4 };
    case PIECE_SHAPE_H:
      return (piece_type_info){ h, 5 };
    case PIECE_SHAPE_LINE_3:
      return (piece_type_info){ line_3, 3 };
    case PIECE_SHAPE_LINE_4:
      return (piece_type_info){ line_4, 4 };
    case PIECE_SHAPE_L_RIGHT:
      return (piece_type_info){ l_right, 4 };
    case PIECE_SHAPE_L_LEFT:
      return (piece_type_info){ l_left, 4 };
    case PIECE_SHAPE_DIAMOND:
      return (piece_type_info){ diamond, 4 };
    case PIECE_SHAPE_SEPT:
      return (piece_type_info){ sept, 7 };
    default:
      return (piece_type_info){ single, 1 };
    }
}
