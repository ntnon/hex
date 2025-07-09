#include "UI/menu.h"
#include "game/game.h"
#include "game/screen_manager.h"
#include "raylib.h"
#include <stdio.h>
int
main (void)
{

  // Initialization
  int screen_width = 800;
  int screen_height = 400;
  SetConfigFlags (FLAG_WINDOW_HIGHDPI);
  SetConfigFlags (FLAG_WINDOW_RESIZABLE);
  SetTargetFPS (60);
  InitWindow (screen_width, screen_height, "hex");

  game_t *game;
  game_init (game);

  screen_manager_t mgr;
  screen_manager_init (&mgr);

  menu_screen_t menu;
  menu_screen_init (&menu);

  while (!WindowShouldClose ())
    {
      screen_width = GetScreenWidth ();
      screen_height = GetScreenHeight ();

      board_input_controller_update (game->&input_ctrl, game->board, screen_width,
                                     screen_height);
      BeginDrawing ();

      ClearBackground (RAYWHITE);
      switch (mgr.current)
        {
        case SCREEN_MENU:
          menu_action_t action = menu_screen_update (
              &menu, GetMousePosition (), IsMouseButtonDown (0));
          menu_screen_draw (&menu);
          menu_screen_update (&menu, GetMousePosition (),
                              IsMouseButtonDown (0));
          menu_screen_draw (&menu);
          if (action == MENU_ACTION_START)
            mgr.current = SCREEN_GAME;
          else if (action == MENU_ACTION_QUIT)
            CloseWindow (); // Or set a running flag to false
          break;
        case SCREEN_GAME:
          render_game (game);

          // draw_game();
          break;
        case SCREEN_PAUSE:
          // update_pause();
          // draw_pause();
          break;
        case SCREEN_GAMEOVER:
          // update_gameover();
          // draw_gameover();
          break;
        default:
          break;
        }

      EndDrawing ();
    }

  //
  free_game (game);

  CloseWindow ();

  return 0;
}
