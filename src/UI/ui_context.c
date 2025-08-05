#include "../../include/ui/ui_context.h"
#include "../../include/screen/screen_manager.h"
#include "../../include/ui/menu_ui.h"
#include "../../include/ui/settings_ui.h"
#include "raylib.h"
#include <stdlib.h>

static void
register_screen_handlers (ui_context_t *ctx)
{
  // Menu screen handlers
  ctx->handlers[SCREEN_MENU]
      = (ui_screen_handlers_t){ .state = &ctx->menu,
                                .init = menu_ui_init,
                                .cleanup = menu_ui_cleanup,
                                .render = menu_ui_draw,
                                .update = menu_ui_update };

  // Settings screen handlers
  ctx->handlers[SCREEN_SETTINGS]
      = (ui_screen_handlers_t){ .state = &ctx->settings,
                                .init = settings_ui_init,
                                .cleanup = settings_ui_cleanup,
                                .render = settings_ui_draw,
                                .update = settings_ui_update };

  // Game screen handlers will be added later
  ctx->handlers[SCREEN_GAME]
      = (ui_screen_handlers_t){ .state = &ctx->game.base,
                                .init = NULL,
                                .cleanup = NULL,
                                .render = NULL,
                                .update = NULL };
}

void
ui_context_init (ui_context_t *ctx)
{
  if (!ctx)
    return;

  // Initialize Clay context with reasonable defaults
  Clay_ContextConfig config
      = { .initialMemory = 1024 * 1024, // 1MB initial memory
          .initialElementCapacity = 1024,
          .initialCommandCapacity = 1024 };

  ctx->context = Clay_CreateContext (&config);

  // Initialize all screen states
  ctx->menu = (ui_screen_state_t){ .is_open = false, .active_element = 0 };
  ctx->settings = (ui_screen_state_t){ .is_open = false, .active_element = 0 };
  ctx->game.base
      = (ui_screen_state_t){ .is_open = false, .active_element = 0 };

  // Set initial screen
  ctx->current_screen = SCREEN_MENU;

  // Register handlers for all screens
  register_screen_handlers (ctx);

  // Initialize the first screen
  if (ctx->handlers[ctx->current_screen].init)
    {
      ctx->handlers[ctx->current_screen].init (ctx);
    }
}

void
ui_context_cleanup (ui_context_t *ctx)
{
  if (!ctx)
    return;

  // Cleanup current screen
  ui_screen_handlers_t *handlers = &ctx->handlers[ctx->current_screen];
  if (handlers->cleanup)
    {
      handlers->cleanup (ctx);
    }

  // Cleanup Clay context
  if (ctx->context)
    {
      Clay_DestroyContext (ctx->context);
      ctx->context = NULL;
    }
}

void
ui_context_begin_frame (ui_context_t *ctx)
{
  if (!ctx || !ctx->context)
    return;

  Clay_BeginFrame (ctx->context,
                   (Clay_FrameConfig){ .screenWidth = GetScreenWidth (),
                                       .screenHeight = GetScreenHeight (),
                                       .deltaTime = GetFrameTime () });
}

void
ui_context_end_frame (ui_context_t *ctx)
{
  if (!ctx || !ctx->context)
    return;

  Clay_EndFrame (ctx->context);
}

void
ui_center_container (ui_context_t *ctx, Clay_Container *container)
{
  if (!ctx || !container)
    return;

  int screen_width = GetScreenWidth ();
  int screen_height = GetScreenHeight ();

  container->position.x
      = (screen_width - container->sizing[0].size.minMax[0]) / 2;
  container->position.y
      = (screen_height - container->sizing[1].size.minMax[0]) / 2;
}

void
ui_draw_button_list (ui_context_t *ctx, const char *title,
                     const ui_button_config_t *buttons, int button_count,
                     Clay_ID *active_button)
{
  if (!ctx || !ctx->context || !buttons || !active_button)
    return;

  // Create root container
  Clay_Container root
      = Clay_Container (.id = CLAY_ID ("button_list_root"),
                        .sizing
                        = { CLAY_SIZING_FIXED (200),   // Default width
                            CLAY_SIZING_FIXED (400) }, // Default height
                        .layout = CLAY_LAYOUT_VERTICAL, .spacing = 20,
                        .alignment = CLAY_ALIGNMENT_CENTER);

  ui_center_container (ctx, &root);
  Clay_PushContainer (ctx->context, &root);

  // Draw title if provided
  if (title)
    {
      Clay_Text (ctx->context,
                 Clay_TextConfig (.text = title, .fontSize = 48,
                                  .alignment = CLAY_TEXT_ALIGN_CENTER));
    }

  // Draw buttons
  for (int i = 0; i < button_count; i++)
    {
      Clay_Button button
          = Clay_Button (.id = buttons[i].id, .text = buttons[i].label,
                         .sizing = { CLAY_SIZING_FIXED (200),
                                     CLAY_SIZING_FIXED (50) });

      if (Clay_DoButton (ctx->context, &button))
        {
          *active_button = button.id;
        }
    }

  Clay_PopContainer (ctx->context);
}

void
ui_switch_screen (ui_context_t *ctx, screen_type_t new_screen)
{
  if (!ctx || new_screen >= SCREEN_MAX)
    return;

  // Cleanup current screen
  ui_screen_handlers_t *old_handlers = &ctx->handlers[ctx->current_screen];
  if (old_handlers->cleanup)
    {
      old_handlers->cleanup (ctx);
    }

  // Initialize new screen
  ui_screen_handlers_t *new_handlers = &ctx->handlers[new_screen];
  if (new_handlers->init)
    {
      new_handlers->init (ctx);
    }

  ctx->current_screen = new_screen;
}
