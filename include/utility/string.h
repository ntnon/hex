#include <stdio.h>
#include "third_party/clay.h"

char *combine_string_int(const char *str, int num);

// Clay string formatting utilities
Clay_String clay_string_format_int(const char *format, int value);
Clay_String clay_string_format_uint(const char *format, unsigned int value);
Clay_String clay_string_format_float(const char *format, float value);
Clay_String clay_string_literal(const char *text);

// Convenience macros for common patterns
#define CLAY_TEXT_INT(format, value, config) \
    CLAY_TEXT(clay_string_format_int(format, value), config)
