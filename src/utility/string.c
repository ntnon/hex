#include "utility/string.h"

// Combine a string and an integer into a buffer.
// Example: combine_string_int("inventory_item", 5, buffer, sizeof(buffer));
// buffer will contain "inventory_item5"
const char *combine_string_int(const char *str, int num) {
  static char buffer[64]; // static so it survives after return
  snprintf(buffer, sizeof(buffer), "%s%d", str, num);
  return buffer;
}
