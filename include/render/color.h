#ifndef color_t_H
#define color_t_H
#include "stdlib.h"

typedef struct color_t {
    uint8_t r, g, b, a;
} color_t;

#define M_LIGHTGRAY  (color_t){ 200, 200, 200, 255 }   // Light Gray
#define M_GRAY       (color_t){ 130, 130, 130, 255 }   // Gray
#define M_DARKGRAY   (color_t){ 80, 80, 80, 255 }      // Dark Gray
#define M_YELLOW     (color_t){ 253, 249, 0, 255 }     // Yellow
#define M_GOLD       (color_t){ 255, 203, 0, 255 }     // Gold
#define M_ORANGE     (color_t){ 255, 161, 0, 255 }     // Orange
#define M_PINK       (color_t){ 255, 109, 194, 255 }   // Pink
#define M_RED        (color_t){ 230, 41, 55, 255 }     // Red
#define M_MAROON     (color_t){ 190, 33, 55, 255 }     // Maroon
#define M_GREEN      (color_t){ 0, 228, 48, 255 }      // Green
#define M_LIME       (color_t){ 0, 158, 47, 255 }      // Lime
#define M_DARKGREEN  (color_t){ 0, 117, 44, 255 }      // Dark Green
#define M_SKYBLUE    (color_t){ 102, 191, 255, 255 }   // Sky Blue
#define M_BLUE       (color_t){ 0, 121, 241, 255 }     // Blue
#define M_DARKBLUE   (color_t){ 0, 82, 172, 255 }      // Dark Blue
#define M_PURPLE     (color_t){ 200, 122, 255, 255 }   // Purple
#define M_VIOLET     (color_t){ 135, 60, 190, 255 }    // Violet
#define M_DARKPURPLE (color_t){ 112, 31, 126, 255 }    // Dark Purple
#define M_BEIGE      (color_t){ 211, 176, 131, 255 }   // Beige
#define M_BROWN      (color_t){ 127, 106, 79, 255 }    // Brown
#define M_DARKBROWN  (color_t){ 76, 63, 47, 255 }      // Dark Brown

#define M_M_WHITE      (color_t){ 255, 255, 255, 255 }   // White
#define M_BLACK      (color_t){ 0, 0, 0, 255 }         // Black
#define M_BLANK      (color_t){ 0, 0, 0, 0 }           // Blank (Transparent)
#define M_MAGENTA    (color_t){ 255, 0, 255, 255 }     // Magenta
#define M_RAYWHITE   (color_t){ 245, 245, 245, 255 }   // My own White (raylib logo)

color_t color_t_from_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a);


#endif //color_t_H
