#ifndef CLAY_RENDERER_RAYLIB_H
#define CLAY_RENDERER_RAYLIB_H

#include "../third_party/clay.h"
#include "raylib.h"

// Initialize Clay-Raylib renderer
void Clay_Raylib_Initialize(int width, int height, const char *title, unsigned int flags);

// Clean up Clay-Raylib renderer resources
void Clay_Raylib_Close(void);

// Render Clay command array using Raylib
void Clay_Raylib_Render(Clay_RenderCommandArray renderCommands, Font *fonts);

// Text measurement function for Clay
Clay_Dimensions Clay_Raylib_MeasureText(Clay_StringSlice text, Clay_TextElementConfig *config, void *userData);

#endif // CLAY_RENDERER_RAYLIB_H
