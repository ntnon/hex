#include "piece.h"
#include "hex_grid.h"
#include <stdlib.h>

hex emptyArray[0] = {};
hex single[1] = { { 0, 0, 0 } };
hex pair[2] = { { 0, 0, 0 }, { 0, 1, 0 } };
hex triple[3] = { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 } };
hex quad[4] = { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 }, { 0, 1, 1 } };
hex line_3[3] = { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 2, 0 } };
hex line_4[4] = { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 2, 0 }, { 0, 3, 0 } };
hex h[5] = { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 }, { 0, 1, 1 }, { 0, 2, 0 } };
hex l_right[4] = { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 2, 0 }, { 0, 2, 1 } };
hex l_left[4] = { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 2, 0 }, { 0, 1, 1 } };
hex diamond[4] = { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 }, { 0, 1, 1 } };
hex sept[7] = { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 }, { 0, 1, 1 },
                { 0, 2, 0 }, { 0, 2, 1 }, { 0, 1, 2 } };

hex *
piece_type_to_hex_array (piece_type type)
{
  hex anchor = hex_create (0, 0, 0);
  hex hex_array = { 0 };
  switch (type)
    {
    case piece_SHAPE_SINGLE:
      return single;
    case piece_SHAPE_PAIR:
      return pair;
    case piece_SHAPE_TRIPLE:
      return triple;
    case piece_SHAPE_QUAD:
      return quad;
    case piece_SHAPE_H:
      return h;
    case piece_SHAPE_LINE_3:
      return line_3;
    case piece_SHAPE_LINE_4:
      return line_4;
    case piece_SHAPE_L_RIGHT:
      return l_right;
    case piece_SHAPE_L_LEFT:
      return l_left;
    case piece_SHAPE_DIAMOND:
      return diamond;
    case piece_SHAPE_SEPT:
      return sept;
    default:
      return single;
    };

  return emptyArray;
}

piece_array
piece_array_create (void)
{
  piece_array array;
  array.size = 0;
  array.pieces = NULL;
  return array;
}

void
piece_array_free (piece_array *array)
{
  if (!array)
    return;
  if (array->pieces)
    {
      free (array->pieces);
      array->pieces = NULL;
    }
  array->size = 0;
}
