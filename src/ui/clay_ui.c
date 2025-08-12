#include "../../include/ui/clay_ui.h"
#include "../../include/render/clay_renderer_raylib.h"
#include "../../include/third_party/clay.h"
#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CLAY_MEMORY_SIZE_MB 1
#define CLAY_MEMORY_SIZE (CLAY_MEMORY_SIZE_MB * 1024 * 1024)

// Clay colors
const Clay_Color COLOR_BACKGROUND = (Clay_Color){ 245, 245, 245, 255 };
const Clay_Color COLOR_BUTTON_PRIMARY = (Clay_Color){ 100, 150, 200, 255 };
const Clay_Color COLOR_BUTTON_SECONDARY = (Clay_Color){ 120, 120, 120, 255 };
const Clay_Color COLOR_BUTTON_DANGER = (Clay_Color){ 200, 100, 100, 255 };
const Clay_Color COLOR_OVERLAY = (Clay_Color){ 0, 0, 0, 150 };
const Clay_Color COLOR_PANEL = (Clay_Color){ 80, 80, 80, 255 };
const Clay_Color COLOR_TEXT_DARK = (Clay_Color){ 0, 0, 0, 255 };
const Clay_Color COLOR_TEXT_LIGHT = (Clay_Color){ 255, 255, 255, 255 };

// Forward declarations
void HandleClayErrors (Clay_ErrorData errorData);

// Screen rendering functions (implemented in separate files)
extern void clay_ui_render_menu (clay_ui_t *ui);
extern void clay_ui_render_game (clay_ui_t *ui);
extern void clay_ui_render_settings (clay_ui_t *ui);
extern void clay_ui_render_game_end (clay_ui_t *ui);

bool
clay_ui_init (clay_ui_t *ui, float screen_width, float screen_height)
{
  if (!ui)
    return false;

  // Initialize UI state
  ui->current_screen = UI_SCREEN_MENU;
  ui->next_screen = UI_SCREEN_MENU;
  ui->screen_changed = false;
  ui->quit_requested = false;
  ui->screen_width = screen_width;
  ui->screen_height = screen_height;

  // Allocate Clay memory
  uint64_t totalMemorySize = Clay_MinMemorySize ();
  if (totalMemorySize < CLAY_MEMORY_SIZE)
    {
      totalMemorySize = CLAY_MEMORY_SIZE;
    }

  ui->clay_memory_size = totalMemorySize;
  void *memory = malloc (totalMemorySize);
  if (!memory)
    {
      return false;
    }

  ui->clay_memory
      = Clay_CreateArenaWithCapacityAndMemory (totalMemorySize, memory);

  // Initialize Clay
  Clay_Initialize (ui->clay_memory,
                   (Clay_Dimensions){ screen_width, screen_height },
                   (Clay_ErrorHandler){ HandleClayErrors });

  Clay_SetMeasureTextFunction (Raylib_MeasureText, NULL);

  return true;
}

void
clay_ui_cleanup (clay_ui_t *ui)
{
  if (!ui)
    return;

  if (ui->clay_memory.memory)
    {
      free (ui->clay_memory.memory);
      ui->clay_memory.memory = NULL;
    }
}

void
clay_ui_resize (clay_ui_t *ui, float width, float height)
{
  if (!ui)
    return;

  ui->screen_width = width;
  ui->screen_height = height;
  Clay_SetLayoutDimensions ((Clay_Dimensions){ width, height });
}

void
clay_ui_switch_to (clay_ui_t *ui, ui_screen_type_t screen)
{
  if (!ui)
    return;

  ui->next_screen = screen;
  ui->screen_changed = true;
}

bool
clay_ui_should_quit (clay_ui_t *ui)
{
  return ui ? ui->quit_requested : false;
}

void
clay_ui_handle_input (clay_ui_t *ui, input_state_t *input)
{
  if (!ui || !input)
    return;

  // Handle screen-specific input
  switch (ui->current_screen)
    {
    case UI_SCREEN_MENU:
      // Menu input handled by Clay buttons
      break;
    case UI_SCREEN_GAME:
      if (input->key_escape)
        {
          clay_ui_switch_to (ui, UI_SCREEN_SETTINGS);
        }
      break;
    case UI_SCREEN_SETTINGS:
      if (input->key_escape)
        {
          clay_ui_switch_to (ui, UI_SCREEN_GAME);
        }
      break;
    case UI_SCREEN_GAME_END:
      // Game end input handled by Clay buttons
      break;
    default:
      break;
    }
}

void
clay_ui_render (clay_ui_t *ui)
{
  if (!ui)
    return;

  // Handle screen transitions
  if (ui->screen_changed)
    {
      ui->current_screen = ui->next_screen;
      ui->screen_changed = false;
    }

  // Begin Clay layout
  Clay_BeginLayout ();

  // Render current screen
  switch (ui->current_screen)
    {
    case UI_SCREEN_MENU:
      clay_ui_render_menu (ui);
      break;
    case UI_SCREEN_GAME:
      clay_ui_render_game (ui);
      break;
    case UI_SCREEN_SETTINGS:
      clay_ui_render_settings (ui);
      break;
    case UI_SCREEN_GAME_END:
      clay_ui_render_game_end (ui);
      break;
    default:
      break;
    }

  // End Clay layout and render
  Clay_RenderCommandArray renderCommands = Clay_EndLayout ();

  // Use existing Clay-Raylib renderer
  Font defaultFont = GetFontDefault ();
  Clay_Raylib_Render (renderCommands, &defaultFont);
}

// Clay integration functions
void
HandleClayErrors (Clay_ErrorData errorData)
{
  printf ("Clay Error: %.*s\n", (int)errorData.errorText.length,
          errorData.errorText.chars);
}
