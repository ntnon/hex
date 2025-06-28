#include "mouse_controller.h"
#include "raylib.h"
#include "raymath.h"
#include <stdio.h>
#include <stdlib.h>

void
mouse_controller_init (mouse_controller *mouse, Camera2D *camera,
                       game_board_controller *game_board_controller)
{
  mouse->screen_pos = (Vector2){ 0, 0 };
  mouse->world_pos = (Vector2){ 0, 0 };
  mouse->left_pressed = false;
  mouse->left_released = false;
  mouse->left_down = false;
  mouse->right_pressed = false;
  mouse->right_released = false;
  mouse->right_down = false;
  mouse->wheel_move = 0.0f;
  mouse->hovering_hex = false;

  mouse->hovered_hex = NULL; // Initialize hovered_hex to NULL
  mouse->camera = camera;    // Set the camera reference
  mouse->game_board_controller
      = game_board_controller; // Set the game board reference
}

void
mouse_controller_update (mouse_controller *mouse, Camera2D *camera,
                         game_board_controller *game_board_controller)
{
  mouse->screen_pos = GetMousePosition ();
  mouse->world_pos = GetScreenToWorld2D (mouse->screen_pos, *camera);

  mouse->left_pressed = IsMouseButtonPressed (MOUSE_LEFT_BUTTON);
  mouse->left_released = IsMouseButtonReleased (MOUSE_LEFT_BUTTON);
  mouse->left_down = IsMouseButtonDown (MOUSE_LEFT_BUTTON);

  mouse->right_pressed = IsMouseButtonPressed (MOUSE_RIGHT_BUTTON);
  mouse->right_released = IsMouseButtonReleased (MOUSE_RIGHT_BUTTON);
  mouse->right_down = IsMouseButtonDown (MOUSE_RIGHT_BUTTON);

  mouse->wheel_move = GetMouseWheelMove ();

  mouse_controller_update_camera (mouse);
  game_board *preview_board = game_board_controller->preview_board;

  // Detect hovered hex
  hex hovered = pixel_to_hex_rounded (
      preview_board->layout,
      (point){ mouse->world_pos.x, mouse->world_pos.y });

  hex *new_hovered_hex = NULL;
  for (int i = 0; i < preview_board->hex_array.count; i++)
    {
      if (hex_equal (hovered, preview_board->hex_array.data[i]))
        {
          new_hovered_hex = &preview_board->hex_array.data[i];
          break;
        }
    }

  // Only update if the hovered hex has changed
  if (mouse->hovered_hex != new_hovered_hex)
    {
      mouse->hovered_hex = new_hovered_hex;
      mouse->hovering_hex = (new_hovered_hex != NULL);

      // Optional: Trigger events or log changes
      if (mouse->hovered_hex != NULL)
        {
          printf ("Hovered hex changed to: q=%d, r=%d, s=%d\n",
                  mouse->hovered_hex->q, mouse->hovered_hex->r,
                  mouse->hovered_hex->s);
        }
      else
        {
          printf ("No hex is hovered.\n");
        }
    }
}

void
mouse_controller_update_camera (mouse_controller *mouse)
{
  // Handle panning with right or left mouse button
  if (mouse->right_down || mouse->left_down)
    {
      Vector2 delta = GetMouseDelta ();
      delta = Vector2Scale (delta, -1.0f / mouse->camera->zoom);

      // Update the camera target immediately
      mouse->camera->target = Vector2Add (mouse->camera->target, delta);

      // Update the world position of the mouse to match the new camera
      // position
      mouse->world_pos
          = GetScreenToWorld2D (mouse->screen_pos, *mouse->camera);
    }

  // Handle zooming with the mouse wheel
  if (mouse->wheel_move != 0)
    {
      float scale = 0.1f * mouse->wheel_move;
      mouse->camera->zoom
          = Clamp (expf (logf (mouse->camera->zoom) + scale), 0.1f, 10.0f);

      // Update the world position of the mouse after zooming
      mouse->world_pos
          = GetScreenToWorld2D (mouse->screen_pos, *mouse->camera);
    }
}

void
mouse_controller_free (mouse_controller *controller)
{
  if (!controller)
    {
      return; // Nothing to free
    }

  // Free the hovered hex if it was dynamically allocated
  if (controller->hovered_hex)
    {
      free (controller->hovered_hex);
      controller->hovered_hex = NULL;
    }

  // Free the camera if it was dynamically allocated
  if (controller->camera)
    {
      free (controller->camera);
      controller->camera = NULL;
    }

  // Free the game board controller if it was dynamically allocated
  if (controller->game_board_controller)
    {
      game_board_controller_free (
          controller->game_board_controller); // Assuming this function exists
      controller->game_board_controller = NULL;
    }

  // Finally, free the mouse controller itself
  free (controller);
}
