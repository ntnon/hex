#include "game_controller.h"
#include <stdio.h>
#include <stdlib.h>

game_controller *
game_controller_create (game_board *main_board)
{
  game_controller *controller = malloc (sizeof (game_controller));
  if (!controller)
    {
      fprintf (stderr,
               "Failed to allocate memory for game board controller.\n");
      return NULL;
    }

  controller->main_board = main_board;
  if (!controller->main_board)
    {
      fprintf (stderr, "Failed to create main game board.\n");
      free (controller);
      return NULL;
    }

  // Clone the main board to create the preview board
  controller->preview_board = game_board_clone (controller->main_board);
  if (!controller->preview_board)
    {
      fprintf (stderr, "Failed to create preview game board.\n");
      game_board_free (controller->main_board);
      free (controller);
      return NULL;
    }

  return controller;
}

void
game_controller_free (game_controller *controller)
{
  game_board_free (controller->main_board);
  game_board_free (controller->preview_board);
  free (controller);
}
void
game_controller_draw (game_controller *controller)
{
  if (!controller || !controller->main_board || !controller->preview_board)
    {
      printf ("Game board controller is not initialized.");
      return;
    }
  game_board_draw (controller->preview_board);
}

void
progress (game_controller *controller)
{
  if (!controller || !controller->main_board || !controller->preview_board)
    {
      printf ("Game board controller is not initialized.");
      return;
    }
  controller->generation++;
}
