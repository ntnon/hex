#include "utility/string.h"
#include <string.h>

// Combine a string and an integer into a buffer.
// Example: combine_string_int("inventory_item", 5, buffer, sizeof(buffer));
// buffer will contain "inventory_item5"
char *combine_string_int(const char *str, int num) {
  static char buffer[64]; // static so it survives after return
  snprintf(buffer, sizeof(buffer), "%s%d", str, num);
  return buffer;
}

// Clay string formatting utilities
// These use thread-local static buffers for convenience

Clay_String clay_string_format_int(const char *format, int value) {
  static __thread char buffer[128];
  snprintf(buffer, sizeof(buffer), format, value);
  return (Clay_String){.chars = buffer, .length = strlen(buffer)};
}

Clay_String clay_string_format_uint(const char *format, unsigned int value) {
  static __thread char buffer[128];
  snprintf(buffer, sizeof(buffer), format, value);
  return (Clay_String){.chars = buffer, .length = strlen(buffer)};
}

Clay_String clay_string_format_float(const char *format, float value) {
  static __thread char buffer[128];
  snprintf(buffer, sizeof(buffer), format, value);
  return (Clay_String){.chars = buffer, .length = strlen(buffer)};
}

Clay_String clay_string_literal(const char *text) {
  return (Clay_String){.chars = (char *)text, .length = strlen(text)};
}
