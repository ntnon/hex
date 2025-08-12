#!/bin/bash

# Build script for Clay UI example
# This script compiles the Clay UI example with Raylib

set -e

# Configuration
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
EXAMPLE_DIR="$PROJECT_ROOT/examples"
BUILD_DIR="$PROJECT_ROOT/build"
INCLUDE_DIR="$PROJECT_ROOT/include"
SRC_DIR="$PROJECT_ROOT/src"

# Create build directory if it doesn't exist
mkdir -p "$BUILD_DIR"

# Compiler settings
CC=gcc
CFLAGS="-std=c99 -Wall -Wextra -O2"
INCLUDES="-I$INCLUDE_DIR -I$INCLUDE_DIR/third_party"
LIBS="-lraylib -lm"

# Platform-specific library paths and flags
case "$(uname)" in
    "Darwin")
        # macOS
        LIBS="$LIBS -framework OpenGL -framework Cocoa -framework IOKit -framework CoreFoundation -framework CoreVideo"
        ;;
    "Linux")
        # Linux
        LIBS="$LIBS -lGL -lX11 -lXrandr -lXinerama -lXi -lXcursor -lXxf86vm -lpthread -ldl"
        ;;
    *)
        echo "Warning: Unknown platform, using default library settings"
        ;;
esac

# Source files to compile
CLAY_UI_SOURCES="$SRC_DIR/ui/clay_ui.c"
EXAMPLE_SOURCE="$EXAMPLE_DIR/clay_ui_example.c"

# Check if Clay header exists
if [ ! -f "$INCLUDE_DIR/third_party/clay.h" ]; then
    echo "Error: Clay header not found at $INCLUDE_DIR/third_party/clay.h"
    echo "Please ensure Clay is properly installed in the third_party directory"
    exit 1
fi

# Check if input_state.h exists (create minimal version if needed)
if [ ! -f "$INCLUDE_DIR/controller/input_state.h" ]; then
    echo "Creating minimal input_state.h..."
    mkdir -p "$INCLUDE_DIR/controller"
    cat > "$INCLUDE_DIR/controller/input_state.h" << 'EOF'
#ifndef INPUT_STATE_H
#define INPUT_STATE_H

#include <stdbool.h>

typedef struct {
    bool key_escape;
    bool key_enter;
    bool mouse_left_pressed;
    bool mouse_right_pressed;
    float mouse_x;
    float mouse_y;
} input_state_t;

#endif // INPUT_STATE_H
EOF
fi

# Build the example
echo "Building Clay UI example..."
echo "Compiler: $CC"
echo "Flags: $CFLAGS"
echo "Includes: $INCLUDES"
echo "Libraries: $LIBS"
echo

$CC $CFLAGS $INCLUDES \
    $CLAY_UI_SOURCES \
    $EXAMPLE_SOURCE \
    $LIBS \
    -o "$BUILD_DIR/clay_ui_example"

if [ $? -eq 0 ]; then
    echo "Build successful!"
    echo "Executable: $BUILD_DIR/clay_ui_example"
    echo
    echo "To run the example:"
    echo "  cd $PROJECT_ROOT"
    echo "  ./build/clay_ui_example"
else
    echo "Build failed!"
    exit 1
fi
