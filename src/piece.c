#include "piece.h"
#include "hex_grid.h"
#include <stdlib.h>

Hex emptyArray[0] = {};
Hex single[1] = { { 0, 0, 0 } };
Hex pair[2] = { { 0, 0, 0 }, { 0, 1, 0 } };
Hex triple[3] = { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 } };
Hex quad[4] = { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 }, { 0, 1, 1 } };
Hex line_3[3] = { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 2, 0 } };
Hex line_4[4] = { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 2, 0 }, { 0, 3, 0 } };
Hex h[5] = { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 }, { 0, 1, 1 }, { 0, 2, 0 } };
Hex l_right[4] = { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 2, 0 }, { 0, 2, 1 } };
Hex l_left[4] = { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 2, 0 }, { 0, 1, 1 } };
Hex diamond[4] = { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 }, { 0, 1, 1 } };
Hex sept[7] = { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 }, { 0, 1, 1 },
                { 0, 2, 0 }, { 0, 2, 1 }, { 0, 1, 2 } };

Hex *
piece_type_to_hex_array (piece_type type)
{
  Hex anchor = hex_create (0, 0, 0);
  Hex hex_array = { 0 };
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
