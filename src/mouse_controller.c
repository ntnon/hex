#include "mouse_controller.h"
#include "raylib.h"
#include "raymath.h"
#include "tile.h"
#include <stdio.h>
#include <stdlib.h>

void
cycle_tile (mouse_controller *mouse)
{
  printf ("Cycle tile type\n");
  if (mouse->hovering_hex)
    {
      tile_array *arr = mouse->game_controller->preview_board->tile_array;
      if (!arr)
        {
          printf ("Error: tile_array is NULL\n");
          return;
        }
      tile *target_tile
          = get_tile_by_hex (mouse->game_controller->preview_board->tile_array,
                             mouse->hovered_hex);
      printf ("Target tile: %p\n", target_tile);
      tile_cycle (target_tile);
    }
}

void
mouse_controller_init (mouse_controller *mouse, Camera2D *camera,
                       game_controller *game_controller)
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
  mouse->hovered_hex = (hex){ 0, 0, 0 };

  mouse->camera = camera;                   // Set the camera reference
  mouse->game_controller = game_controller; // Set the game board reference
}

void
mouse_controller_update (mouse_controller *mouse, Camera2D *camera,
                         game_controller *game_controller)
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

  if (mouse->left_released)
    {
      cycle_tile (mouse);
    }

  mouse_controller_update_camera (mouse);
  update_hovered_hex (mouse);
}

static void
update_hovered_hex (mouse_controller *mouse)
{
  game_board *preview_board = mouse->game_controller->preview_board;
  game_board *main_board = mouse->game_controller->main_board;

  // Convert the mouse world position to a hex
  hex hovered = pixel_to_hex_rounded (
      preview_board->layout,
      (point){ mouse->world_pos.x, mouse->world_pos.y });

  hex new_hovered_hex = (hex){ 0, 0, 0 }; // Default value
  bool hovering = false;

  // Check if the hovered hex exists in the hex array
  for (int i = 0; i < preview_board->hex_array.count; i++)
    {
      if (hex_equal (hovered, preview_board->hex_array.data[i]))
        {
          new_hovered_hex = preview_board->hex_array.data[i];
          hovering = true;
          break;
        }
    }

  // Update the hovered hex and hovering state if they have changed
  if (!hex_equal (mouse->hovered_hex, new_hovered_hex)
      || mouse->hovering_hex != hovering)
    {
      mouse->hovered_hex = new_hovered_hex;
      mouse->hovering_hex = hovering;

      // Optional: Trigger events or log changes
      if (mouse->hovering_hex)
        {
          highlight_manager_set_tile (preview_board->highlight_manager,
                                      tile_create_empty (mouse->hovered_hex));
        }
      else
        {
          clear_highlights (preview_board->highlight_manager);
        }
    }
  reveal_tile_info (mouse);
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

  // Free the camera if it was dynamically allocated
  if (controller->camera)
    {
      free (controller->camera);
      controller->camera = NULL;
    }

  // Free the game board controller if it was dynamically allocated
  if (controller->game_controller)
    {
      game_controller_free (
          controller->game_controller); // Assuming this function exists
      controller->game_controller = NULL;
    }

  // Finally, free the mouse controller itself
  free (controller);
}

void
reveal_tile_info (mouse_controller *controller)
{
  if (!controller || !controller->hovering_hex)
    {
      return; // Do nothing if the controller is NULL or not hovering over a
              // hex
    }

  // Retrieve the hovered hex
  hex hovered_hex = controller->hovered_hex;

  // Find the tile associated with the hovered hex
  tile *hovered_tile = get_tile_by_hex (
      controller->game_controller->preview_board->tile_array, hovered_hex);

  if (!hovered_tile)
    {
      return; // Do nothing if no tile is found for the hovered hex
    }

  // Define the pop-up table dimensions and position
  Vector2 popup_position
      = controller->screen_pos; // Use the mouse screen position
  float popup_width = 200.0f;
  float popup_height = 100.0f;

  // Adjust the position to avoid going off-screen
  if (popup_position.x + popup_width > GetScreenWidth ())
    {
      popup_position.x -= popup_width;
    }
  if (popup_position.y + popup_height > GetScreenHeight ())
    {
      popup_position.y -= popup_height;
    }

  // Draw the pop-up background
  DrawRectangle (popup_position.x, popup_position.y, popup_width, popup_height,
                 DARKGRAY);

  // Draw the border
  DrawRectangleLines (popup_position.x, popup_position.y, popup_width,
                      popup_height, WHITE);

  // Display the tile information
  const char *tile_type_name = get_tile_type_name (
      hovered_tile
          ->type); // Assuming a function to get the name of the tile type
  char value_text[32];
  snprintf (value_text, sizeof (value_text), "Value: %d", hovered_tile->value);

  // Draw the text
  DrawText ("Tile Info", popup_position.x + 10, popup_position.y + 10, 20,
            WHITE);
  DrawText (tile_type_name, popup_position.x + 10, popup_position.y + 40, 18,
            WHITE);
  DrawText (value_text, popup_position.x + 10, popup_position.y + 70, 18,
            WHITE);
}
