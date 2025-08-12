// #include <stdbool.h>
// #include <stdio.h>

// #include "controller/input_state.h"
// #include "raylib.h"
// #include "ui/clay_ui.h"

// // Simple input state getter for this test
// void
// get_input_state_test (input_state_t *input)
// {
//   if (!input)
//     return;

//   // Clear previous state
//   input->key_escape = false;
//   input->key_enter = false;
//   input->mouse_left_pressed = false;
//   input->mouse_right_pressed = false;

//   // Update with current frame input
//   input->key_escape = IsKeyPressed (KEY_ESCAPE);
//   input->key_enter = IsKeyPressed (KEY_ENTER);
//   input->mouse_left_pressed = IsMouseButtonPressed (MOUSE_BUTTON_LEFT);
//   input->mouse_right_pressed = IsMouseButtonPressed (MOUSE_BUTTON_RIGHT);

//   input->mouse_x = GetMouseX ();
//   input->mouse_y = GetMouseY ();
// }

// int
// clay_ui_test_main (void)
// {
//   // Window setup
//   const int initial_width = 1024;
//   const int initial_height = 768;

//   SetConfigFlags (FLAG_WINDOW_RESIZABLE);
//   InitWindow (initial_width, initial_height, "HexHex - Clay UI Test");
//   SetTargetFPS (60);

//   // Initialize Clay UI system
//   clay_ui_t ui;
//   if (!clay_ui_init (&ui, initial_width, initial_height))
//     {
//       printf ("Failed to initialize Clay UI\n");
//       CloseWindow ();
//       return -1;
//     }

//   printf ("Clay UI Test initialized successfully!\n");
//   printf ("Use mouse to interact with buttons, ESC to navigate\n");

//   // Main loop
//   while (!WindowShouldClose () && !clay_ui_should_quit (&ui))
//     {
//       // Handle window resize
//       if (IsWindowResized ())
//         {
//           int width = GetScreenWidth ();
//           int height = GetScreenHeight ();
//           clay_ui_resize (&ui, width, height);
//         }

//       // Get input state
//       input_state_t input;
//       get_input_state_test (&input);

//       // Update Clay pointer state
//       Clay_SetPointerState ((Clay_Vector2){ GetMouseX (), GetMouseY () },
//                             IsMouseButtonDown (MOUSE_BUTTON_LEFT));

//       // Update UI
//       clay_ui_handle_input (&ui, &input);

//       // Render frame
//       BeginDrawing ();
//       ClearBackground (RAYWHITE);

//       // Render UI
//       clay_ui_render (&ui);

//       // Optional: Draw FPS and instructions
//       DrawFPS (10, 10);
//       DrawText ("Clay UI Test - ESC to quit", 10, 30, 20, DARKGRAY);

//       char screen_text[64];
//       snprintf (screen_text, sizeof (screen_text), "Current Screen: %d",
//                 ui.current_screen);
//       DrawText (screen_text, 10, 55, 16, DARKGRAY);

//       EndDrawing ();
//     }

//   // Cleanup
//   clay_ui_cleanup (&ui);
//   CloseWindow ();

//   printf ("Clay UI Test completed successfully!\n");
//   return 0;
// }

// // Function to integrate Clay UI test into existing main.c
// // You can call this from your main game loop when needed
// bool
// run_clay_ui_test_mode (void)
// {
//   printf ("Starting Clay UI test mode...\n");
//   return clay_ui_test_main () == 0;
// }
