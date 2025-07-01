#include "renderer.h"

void
renderer_init (renderer *r, Camera2D *camera, game_controller *game_controller,
               mouse_controller *mouse_controller)
{
  // Initialize the camera
  r->camera = (Camera2D){ 0 };
  r->camera.target = (Vector2){ 0, 0 }; // Center of the game board
  r->camera.offset
      = (Vector2){ GetScreenWidth () / 2.0f, GetScreenHeight () / 2.0f };
  r->camera.zoom = 1.0f;

  // Store references to the game board and mouse controller
  r->game_controller = game_controller;
  r->mouse_controller = mouse_controller;
}

void
render (renderer *r)
{
  BeginDrawing ();
  ClearBackground (RAYWHITE);

  BeginMode2D (r->camera);
  game_controller_draw (r->game_controller);
  EndMode2D ();

  mouse_controller_update (r->mouse_controller, &r->camera,
                           r->game_controller);

  render_stats (r);
  EndDrawing ();
}

void
renderer_free (renderer *r)
{
  // Currently, no dynamic memory is allocated in the renderer,
  // but this function can be used for cleanup if needed in the future.
}

void
draw_text_with_outline (const char *text, int posX, int posY, int fontSize,
                        Color textColor, Color outlineColor)
{
  // Draw the outline (dark edges)
  DrawText (text, posX - 1, posY, fontSize, outlineColor); // Left
  DrawText (text, posX + 1, posY, fontSize, outlineColor); // Right
  DrawText (text, posX, posY - 1, fontSize, outlineColor); // Up
  DrawText (text, posX, posY + 1, fontSize, outlineColor); // Down

  // Draw the main text
  DrawText (text, posX, posY, fontSize, textColor);
}

void
render_stats (renderer *r)
{
  game_board *board = r->game_controller->preview_board;
  if (!board)
    {
      // If the game board is NULL, don't draw anything
      return;
    }

  // Define positions for the text
  int x = 10;                         // Starting x position
  int y = 10;                         // Starting y position
  int line_spacing = 20;              // Spacing between lines
  int text_width = 100;               // Approximate width of the text area
  int text_height = 3 * line_spacing; // Total height for 3 lines of text

  // Draw a grayish background rectangle
  Color background_color = (Color){ 200, 200, 200, 255 }; // Light gray
  DrawRectangle (x - 5, y - 5, text_width, text_height + 10, background_color);

  // Draw the information from the game board with dark edges
  draw_text_with_outline (
      TextFormat ("M: %d (%d)", board->magentas, board->magenta_production), x,
      y + 0 * line_spacing, 20, MAGENTA, BLACK);

  draw_text_with_outline (
      TextFormat ("C: %d (%d)", board->cyans, board->cyan_production), x,
      y + 1 * line_spacing, 20, SKYBLUE, BLACK);

  draw_text_with_outline (
      TextFormat ("Y: %d (%d)", board->yellows, board->yellow_production), x,
      y + 2 * line_spacing, 20, YELLOW, BLACK);
}
