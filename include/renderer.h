#ifndef RENDERER_H
#define RENDERER_H

#include "mouse_controller.h"
#include "raylib.h"

typedef struct
{
  Camera2D camera;                    // Camera for rendering
  game_controller *game_controller;   // Reference to the game board
  mouse_controller *mouse_controller; // Reference to the mouse controller
} renderer;

// Initializes the renderer
void renderer_init (renderer *r, Camera2D *camera, game_controller *game_controller,
                    mouse_controller *mouse_controller);

// Renders the game board and other components
void render (renderer *renderer);

void render_stats (renderer *r);

// Frees any resources used by the renderer (if needed)
void renderer_free (renderer *renderer);

#endif // RENDERER_H
