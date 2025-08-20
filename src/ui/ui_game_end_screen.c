// #include "../../include/third_party/clay.h"
// #include "../../include/ui/clay_ui.h"
// #include "raylib.h"

// // External color definitions from clay_ui.c
// extern const Clay_Color COLOR_BUTTON_PRIMARY;
// extern const Clay_Color COLOR_BUTTON_SECONDARY;
// extern const Clay_Color COLOR_TEXT_LIGHT;

// void
// clay_ui_render_game_end (clay_ui_t *ui)
// {
//   CLAY ({ .id = CLAY_ID ("GameEndRoot"),
//           .layout
//           = { .sizing = { CLAY_SIZING_GROW (), CLAY_SIZING_GROW () },
//               .layoutDirection = CLAY_TOP_TO_BOTTOM,
//               .childAlignment = { CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER
//               }, .childGap = 30 },
//           .backgroundColor = (Clay_Color){ 40, 40, 40, 200 } })
//   {

//     // Game Over title
//     CLAY_TEXT (CLAY_STRING ("Game Over"),
//                CLAY_TEXT_CONFIG ({ .fontSize = 48,
//                                    .textColor = COLOR_TEXT_LIGHT,
//                                    .letterSpacing = 2 }));

//     // Play again button
//     CLAY (
//         { .id = CLAY_ID ("PlayAgainButton"),
//           .layout
//           = { .sizing = { CLAY_SIZING_FIXED (200), CLAY_SIZING_FIXED (60) },
//               .childAlignment = { CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER }
//               },
//           .backgroundColor = COLOR_BUTTON_PRIMARY })
//     {
//       CLAY_TEXT (CLAY_STRING ("Play Again"),
//                  CLAY_TEXT_CONFIG ({ .fontSize = 24,
//                                      .textColor = COLOR_TEXT_LIGHT,
//                                      .letterSpacing = 1 }));

//       if (Clay_PointerOver (CLAY_ID ("PlayAgainButton"))
//           && IsMouseButtonPressed (MOUSE_BUTTON_LEFT))
//         {
//           clay_ui_switch_to (ui, UI_SCREEN_GAME);
//         }
//     }

//     // Main menu button
//     CLAY (
//         { .id = CLAY_ID ("EndMainMenuButton"),
//           .layout
//           = { .sizing = { CLAY_SIZING_FIXED (200), CLAY_SIZING_FIXED (60) },
//               .childAlignment = { CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER }
//               },
//           .backgroundColor = COLOR_BUTTON_SECONDARY })
//     {
//       CLAY_TEXT (CLAY_STRING ("Main Menu"),
//                  CLAY_TEXT_CONFIG ({ .fontSize = 24,
//                                      .textColor = COLOR_TEXT_LIGHT,
//                                      .letterSpacing = 1 }));

//       if (Clay_PointerOver (CLAY_ID ("EndMainMenuButton"))
//           && IsMouseButtonPressed (MOUSE_BUTTON_LEFT))
//         {
//           clay_ui_switch_to (ui, UI_SCREEN_MENU);
//         }
//     }
//   }
// }
