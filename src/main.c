#include "raylib.h"
#include "raymath.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

//------------------------------------------------------------------------------------
// Basic Types and Constants
//------------------------------------------------------------------------------------

// Tile states
typedef enum
{
  TILE_EMPTY = 0,
  TILE_SPECIAL_MAGENTA,
  TILE_SPECIAL_CYAN,
  TILE_SPECIAL_YELLOW,
  TILE_BASE,
  TILE_RESOURCE_WOOD,
  TILE_RESOURCE_STONE,
  TILE_RESOURCE_WATER
} TileState;

// Inventory item types
typedef enum
{
  ITEM_SINGLE = 0,
  ITEM_DOMINO,
  ITEM_L_SHAPE,
  ITEM_STRAIGHT_LINE,
  ITEM_TRIANGLE,
  ITEM_BRIDGE,
  ITEM_DESTROYER,
  ITEM_COUNT
} ItemType;

// Basic hex tile structure
typedef struct
{
  int q, r;        // axial coordinates
  Vector2 center;  // pixel position
  Color color;     // tile color
  TileState state; // tile state
  int dotCount;    // accessibility dots
  bool isSelected; // selection state
} HexTile;

// Inventory slot structure
typedef struct
{
  ItemType itemType;
  int count;
  bool isSelected;
  Rectangle bounds;
  const char *name;
  Color color;
} InventorySlot;

// Game state structure
typedef struct
{
  bool isPaused;
  bool showDebugInfo;
  bool showInventory;
  HexTile *hoveredTile;
  HexTile *selectedTile;
  Vector2 mousePos;
  Vector2 cameraOffset;
  float cameraZoom;

  // Inventory system
  InventorySlot inventory[ITEM_COUNT];
  int selectedInventorySlot;
  bool placementMode;
  ItemType currentItem;
} GameState;

// Simple hex board
typedef struct
{
  HexTile *tiles;
  int tileCount;
  float hexSize;
  Vector2 origin;
  int radius;
} SimpleHexBoard;

//------------------------------------------------------------------------------------
// Function Declarations
//------------------------------------------------------------------------------------

void DrawHexagon (Vector2 center, float radius, Color fillColor,
                  Color outlineColor);
void InitializeBoard (SimpleHexBoard *board, int radius, float hexSize,
                      Vector2 origin);
void DrawBoard (SimpleHexBoard *board, GameState *gameState);
void InitializeGameState (GameState *gameState);
void UpdateGameState (GameState *gameState, SimpleHexBoard *board);
void DrawUI (GameState *gameState);
void DrawInventory (GameState *gameState);
HexTile *FindHoveredTile (SimpleHexBoard *board, Vector2 mousePos);
int HexDistance (int q1, int r1, int q2, int r2);
Vector2 AxialToPixel (int q, int r, float hexSize, Vector2 offset);
void PixelToAxial (Vector2 pos, float hexSize, Vector2 offset, int *q, int *r);
Color GetTileStateColor (TileState state);
const char *GetTileStateName (TileState state);
const char *GetItemTypeName (ItemType type);
Color GetItemTypeColor (ItemType type);

//------------------------------------------------------------------------------------
// Main Program
//------------------------------------------------------------------------------------

int
main (void)
{
  printf ("Starting Hex Board Game with Inventory System...\n");

  // Initialization with minimal flags to avoid GLFW issues
  const int screenWidth = 1200;
  const int screenHeight = 800;

  SetConfigFlags (FLAG_WINDOW_RESIZABLE);
  InitWindow (screenWidth, screenHeight,
              "Hex Board Game - Meta Pools & Inventory");

  // Check if window initialized properly
  if (!IsWindowReady ())
    {
      printf ("Failed to initialize window\n");
      return -1;
    }

  SetTargetFPS (60);
  printf ("Window initialized successfully!\n");

  // Initialize game systems
  GameState gameState;
  InitializeGameState (&gameState);

  SimpleHexBoard board;
  InitializeBoard (&board, 5, 25.0f,
                   (Vector2){ screenWidth / 2.0f, screenHeight / 2.0f });

  printf ("Game initialized with %d tiles.\n", board.tileCount);

  // Main game loop
  while (!WindowShouldClose ())
    {
      // Update
      UpdateGameState (&gameState, &board);

      // Draw
      BeginDrawing ();
      ClearBackground (RAYWHITE);

      // Apply camera transform
      Vector2 cameraTarget = Vector2Add (board.origin, gameState.cameraOffset);
      Camera2D camera
          = { .offset = { screenWidth / 2.0f, screenHeight / 2.0f },
              .target = cameraTarget,
              .rotation = 0.0f,
              .zoom = gameState.cameraZoom };

      BeginMode2D (camera);
      DrawBoard (&board, &gameState);
      EndMode2D ();

      // Draw UI (not affected by camera)
      DrawUI (&gameState);

      EndDrawing ();
    }

  // Cleanup
  if (board.tiles)
    free (board.tiles);
  CloseWindow ();

  printf ("Application ended successfully.\n");
  return 0;
}

//------------------------------------------------------------------------------------
// Hex Math Functions
//------------------------------------------------------------------------------------

Vector2
AxialToPixel (int q, int r, float hexSize, Vector2 offset)
{
  Vector2 result;
  result.x = hexSize * (3.0f / 2.0f * q) + offset.x;
  result.y = hexSize * (sqrtf (3.0f) / 2.0f * q + sqrtf (3.0f) * r) + offset.y;
  return result;
}

void
PixelToAxial (Vector2 pos, float hexSize, Vector2 offset, int *q, int *r)
{
  Vector2 relPos = Vector2Subtract (pos, offset);
  float qf = (2.0f / 3.0f * relPos.x) / hexSize;
  float rf
      = (-1.0f / 3.0f * relPos.x + sqrtf (3.0f) / 3.0f * relPos.y) / hexSize;

  int rq = roundf (qf);
  int rr = roundf (rf);
  int rs = roundf (-qf - rf);

  float q_diff = fabsf (rq - qf);
  float r_diff = fabsf (rr - rf);
  float s_diff = fabsf (rs + qf + rf);

  if (q_diff > r_diff && q_diff > s_diff)
    rq = -rr - rs;
  else if (r_diff > s_diff)
    rr = -rq - rs;

  *q = rq;
  *r = rr;
}

int
HexDistance (int q1, int r1, int q2, int r2)
{
  return (abs (q1 - q2) + abs (r1 - r2) + abs ((q1 + r1) - (q2 + r2))) / 2;
}

//------------------------------------------------------------------------------------
// Drawing Functions
//------------------------------------------------------------------------------------

void
DrawHexagon (Vector2 center, float radius, Color fillColor, Color outlineColor)
{
  DrawPoly (center, 6, radius, 0, fillColor);
  DrawPolyLines (center, 6, radius, 0, outlineColor);
}

//------------------------------------------------------------------------------------
// Board Functions
//------------------------------------------------------------------------------------

void
InitializeBoard (SimpleHexBoard *board, int radius, float hexSize,
                 Vector2 origin)
{
  board->radius = radius;
  board->hexSize = hexSize;
  board->origin = origin;
  board->tileCount = 0;

  // Calculate max tiles needed
  int maxTiles = 0;
  for (int q = -radius; q <= radius; q++)
    {
      int r1 = -radius > -q - radius ? -radius : -q - radius;
      int r2 = radius < -q + radius ? radius : -q + radius;
      maxTiles += (r2 - r1 + 1);
    }

  board->tiles = malloc (sizeof (HexTile) * maxTiles);
  if (!board->tiles)
    {
      printf ("Failed to allocate memory for tiles\n");
      return;
    }

  // Create tiles
  for (int q = -radius; q <= radius; q++)
    {
      int r1 = -radius > -q - radius ? -radius : -q - radius;
      int r2 = radius < -q + radius ? radius : -q + radius;

      for (int r = r1; r <= r2; r++)
        {
          HexTile *tile = &board->tiles[board->tileCount];
          tile->q = q;
          tile->r = r;
          tile->center = AxialToPixel (q, r, hexSize, origin);
          tile->state = TILE_EMPTY;
          tile->dotCount = 0;
          tile->isSelected = false;
          tile->color = LIGHTGRAY;

          // Add some test tiles
          int distance = HexDistance (q, r, 0, 0);
          if (distance == 0)
            {
              tile->state = TILE_BASE;
              tile->color = GREEN;
            }
          else if (distance == 2 && q % 2 == 0)
            {
              tile->state = TILE_SPECIAL_MAGENTA;
              tile->color = MAGENTA;
              tile->dotCount = 1;
            }
          else if (distance == 3 && r % 2 == 0)
            {
              tile->state = TILE_SPECIAL_CYAN;
              tile->color = SKYBLUE;
              tile->dotCount = 2;
            }

          board->tileCount++;
        }
    }
}

void
DrawBoard (SimpleHexBoard *board, GameState *gameState)
{
  for (int i = 0; i < board->tileCount; i++)
    {
      HexTile *tile = &board->tiles[i];

      // Determine tile color
      Color tileColor = tile->color;
      Color outlineColor = DARKGRAY;

      if (tile == gameState->hoveredTile)
        {
          outlineColor = BLUE;
        }
      if (tile == gameState->selectedTile)
        {
          outlineColor = RED;
          tileColor = ColorBrightness (tileColor, 0.2f);
        }

      // Draw hexagon
      DrawHexagon (tile->center, board->hexSize - 2, tileColor, outlineColor);

      // Draw dots for accessibility
      if (tile->dotCount > 0)
        {
          float dotRadius = board->hexSize * 0.08f;
          Color dotColor = BLACK;

          if (tile->dotCount == 1)
            {
              DrawCircleV (tile->center, dotRadius, dotColor);
            }
          else if (tile->dotCount == 2)
            {
              float spacing = board->hexSize * 0.2f;
              DrawCircleV (
                  (Vector2){ tile->center.x - spacing, tile->center.y },
                  dotRadius, dotColor);
              DrawCircleV (
                  (Vector2){ tile->center.x + spacing, tile->center.y },
                  dotRadius, dotColor);
            }
          else if (tile->dotCount == 3)
            {
              float spacing = board->hexSize * 0.15f;
              DrawCircleV (
                  (Vector2){ tile->center.x, tile->center.y - spacing },
                  dotRadius, dotColor);
              DrawCircleV ((Vector2){ tile->center.x - spacing,
                                      tile->center.y + spacing },
                           dotRadius, dotColor);
              DrawCircleV ((Vector2){ tile->center.x + spacing,
                                      tile->center.y + spacing },
                           dotRadius, dotColor);
            }
        }
    }
}

HexTile *
FindHoveredTile (SimpleHexBoard *board, Vector2 mousePos)
{
  int q, r;
  PixelToAxial (mousePos, board->hexSize, board->origin, &q, &r);

  for (int i = 0; i < board->tileCount; i++)
    {
      if (board->tiles[i].q == q && board->tiles[i].r == r)
        {
          return &board->tiles[i];
        }
    }
  return NULL;
}

//------------------------------------------------------------------------------------
// Game State Functions
//------------------------------------------------------------------------------------

void
InitializeGameState (GameState *gameState)
{
  gameState->isPaused = false;
  gameState->showDebugInfo = false;
  gameState->showInventory = true;
  gameState->hoveredTile = NULL;
  gameState->selectedTile = NULL;
  gameState->mousePos = (Vector2){ 0, 0 };
  gameState->cameraOffset = (Vector2){ 0, 0 };
  gameState->cameraZoom = 1.0f;
  gameState->selectedInventorySlot = -1;
  gameState->placementMode = false;
  gameState->currentItem = ITEM_SINGLE;

  // Initialize inventory
  const char *itemNames[] = { "Single",   "Domino", "L-Shape",  "Line",
                              "Triangle", "Bridge", "Destroyer" };
  Color itemColors[]
      = { GRAY, BROWN, ORANGE, PURPLE, YELLOW, BEIGE, DARKPURPLE };
  int itemCounts[] = { 5, 3, 2, 2, 1, 2, 1 };

  for (int i = 0; i < ITEM_COUNT; i++)
    {
      gameState->inventory[i].itemType = (ItemType)i;
      gameState->inventory[i].count = itemCounts[i];
      gameState->inventory[i].isSelected = false;
      gameState->inventory[i].name = itemNames[i];
      gameState->inventory[i].color = itemColors[i];
    }
}

void
UpdateGameState (GameState *gameState, SimpleHexBoard *board)
{
  // Update mouse position
  gameState->mousePos = GetMousePosition ();

  // Camera controls
  float cameraSpeed = 200.0f * GetFrameTime ();
  if (IsKeyDown (KEY_W) || IsKeyDown (KEY_UP))
    gameState->cameraOffset.y += cameraSpeed;
  if (IsKeyDown (KEY_S) || IsKeyDown (KEY_DOWN))
    gameState->cameraOffset.y -= cameraSpeed;
  if (IsKeyDown (KEY_A) || IsKeyDown (KEY_LEFT))
    gameState->cameraOffset.x += cameraSpeed;
  if (IsKeyDown (KEY_D) || IsKeyDown (KEY_RIGHT))
    gameState->cameraOffset.x -= cameraSpeed;

  // Zoom controls
  float wheel = GetMouseWheelMove ();
  if (wheel != 0)
    {
      gameState->cameraZoom += wheel * 0.1f;
      if (gameState->cameraZoom < 0.5f)
        gameState->cameraZoom = 0.5f;
      if (gameState->cameraZoom > 3.0f)
        gameState->cameraZoom = 3.0f;
    }

  // Keyboard shortcuts
  if (IsKeyPressed (KEY_F1))
    gameState->showDebugInfo = !gameState->showDebugInfo;
  if (IsKeyPressed (KEY_TAB))
    gameState->showInventory = !gameState->showInventory;
  if (IsKeyPressed (KEY_SPACE))
    gameState->isPaused = !gameState->isPaused;
  if (IsKeyPressed (KEY_R))
    {
      gameState->cameraOffset = (Vector2){ 0, 0 };
      gameState->cameraZoom = 1.0f;
    }

  // Convert mouse to world coordinates
  Vector2 cameraTarget = Vector2Add (board->origin, gameState->cameraOffset);
  Camera2D camera
      = { .offset = { GetScreenWidth () / 2.0f, GetScreenHeight () / 2.0f },
          .target = cameraTarget,
          .rotation = 0.0f,
          .zoom = gameState->cameraZoom };
  Vector2 worldMouse = GetScreenToWorld2D (gameState->mousePos, camera);

  // Find hovered tile
  gameState->hoveredTile = FindHoveredTile (board, worldMouse);

  // Handle tile interaction
  if (IsMouseButtonPressed (MOUSE_BUTTON_LEFT))
    {
      if (gameState->hoveredTile)
        {
          gameState->selectedTile = gameState->hoveredTile;

          // Cycle through tile states for testing
          switch (gameState->hoveredTile->state)
            {
            case TILE_EMPTY:
              gameState->hoveredTile->state = TILE_SPECIAL_MAGENTA;
              gameState->hoveredTile->color = MAGENTA;
              gameState->hoveredTile->dotCount = 1;
              break;
            case TILE_SPECIAL_MAGENTA:
              gameState->hoveredTile->state = TILE_SPECIAL_CYAN;
              gameState->hoveredTile->color = SKYBLUE;
              gameState->hoveredTile->dotCount = 2;
              break;
            case TILE_SPECIAL_CYAN:
              gameState->hoveredTile->state = TILE_SPECIAL_YELLOW;
              gameState->hoveredTile->color = YELLOW;
              gameState->hoveredTile->dotCount = 3;
              break;
            default:
              gameState->hoveredTile->state = TILE_EMPTY;
              gameState->hoveredTile->color = LIGHTGRAY;
              gameState->hoveredTile->dotCount = 0;
              break;
            }
        }
      else
        {
          gameState->selectedTile = NULL;
        }
    }

  // Handle inventory interaction
  if (gameState->showInventory)
    {
      for (int i = 0; i < ITEM_COUNT; i++)
        {
          if (CheckCollisionPointRec (gameState->mousePos,
                                      gameState->inventory[i].bounds))
            {
              if (IsMouseButtonPressed (MOUSE_BUTTON_LEFT))
                {
                  if (gameState->selectedInventorySlot == i)
                    {
                      gameState->selectedInventorySlot = -1;
                      gameState->placementMode = false;
                    }
                  else
                    {
                      gameState->selectedInventorySlot = i;
                      gameState->currentItem = (ItemType)i;
                      gameState->placementMode = true;
                    }

                  // Update selection states
                  for (int j = 0; j < ITEM_COUNT; j++)
                    {
                      gameState->inventory[j].isSelected
                          = (j == gameState->selectedInventorySlot);
                    }
                }
            }
        }
    }
}

//------------------------------------------------------------------------------------
// UI Functions
//------------------------------------------------------------------------------------

void
DrawInventory (GameState *gameState)
{
  if (!gameState->showInventory)
    return;

  int inventoryY = GetScreenHeight () - 120;
  int slotSize = 80;
  int spacing = 10;
  int startX = 20;

  // Draw background
  Rectangle inventoryBg
      = { 10, inventoryY - 10, ITEM_COUNT * (slotSize + spacing) + 10, 110 };
  DrawRectangleRounded (inventoryBg, 0.1f, 8, (Color){ 40, 40, 40, 200 });
  DrawRectangleRoundedLines (inventoryBg, 0.1f, 8, WHITE);

  DrawText ("Inventory", 20, inventoryY - 35, 16, WHITE);

  // Draw inventory slots
  for (int i = 0; i < ITEM_COUNT; i++)
    {
      int x = startX + i * (slotSize + spacing);
      int y = inventoryY;

      gameState->inventory[i].bounds = (Rectangle){ x, y, slotSize, slotSize };

      Color slotColor = gameState->inventory[i].color;
      Color borderColor = gameState->inventory[i].isSelected ? YELLOW : WHITE;

      // Draw slot background
      DrawRectangleRounded (gameState->inventory[i].bounds, 0.1f, 8,
                            slotColor);
      DrawRectangleRoundedLines (gameState->inventory[i].bounds, 0.1f, 8,
                                 borderColor);

      // Draw count
      char countText[16];
      snprintf (countText, sizeof (countText), "%d",
                gameState->inventory[i].count);
      DrawText (countText, x + 5, y + 5, 16, BLACK);

      // Draw item name
      int textWidth = MeasureText (gameState->inventory[i].name, 10);
      int textX = x + (slotSize - textWidth) / 2;
      DrawText (gameState->inventory[i].name, textX, y + slotSize - 20, 10,
                BLACK);
    }

  // Draw placement mode indicator
  if (gameState->placementMode)
    {
      const char *modeText = "PLACEMENT MODE - Click to place item";
      int textWidth = MeasureText (modeText, 14);
      DrawText (modeText, GetScreenWidth () - textWidth - 20, inventoryY - 35,
                14, YELLOW);
    }
}

void
DrawUI (GameState *gameState)
{
  // Draw inventory
  DrawInventory (gameState);

  // Draw tile info panel
  if (gameState->hoveredTile || gameState->selectedTile)
    {
      HexTile *infoTile = gameState->selectedTile ? gameState->selectedTile
                                                  : gameState->hoveredTile;

      Rectangle infoPanel = { 10, 10, 300, 160 };
      DrawRectangleRounded (infoPanel, 0.1f, 8, (Color){ 40, 40, 40, 200 });
      DrawRectangleRoundedLines (infoPanel, 0.1f, 8, WHITE);

      int textY = 25;
      DrawText ("Tile Information", 20, textY, 16, WHITE);
      textY += 25;

      char coordText[64];
      snprintf (coordText, sizeof (coordText), "Coordinates: (%d, %d)",
                infoTile->q, infoTile->r);
      DrawText (coordText, 20, textY, 12, LIGHTGRAY);
      textY += 20;

      char stateText[64];
      snprintf (stateText, sizeof (stateText), "State: %d", infoTile->state);
      DrawText (stateText, 20, textY, 12, LIGHTGRAY);
      textY += 20;

      char dotText[64];
      snprintf (dotText, sizeof (dotText), "Dots: %d", infoTile->dotCount);
      DrawText (dotText, 20, textY, 12, LIGHTGRAY);
      textY += 20;

      int distance = HexDistance (infoTile->q, infoTile->r, 0, 0);
      char distanceText[64];
      snprintf (distanceText, sizeof (distanceText),
                "Distance from center: %d", distance);
      DrawText (distanceText, 20, textY, 12, LIGHTGRAY);
    }

  // Draw controls help
  Rectangle helpPanel = { GetScreenWidth () - 320, 10, 310, 160 };
  DrawRectangleRounded (helpPanel, 0.1f, 8, (Color){ 40, 40, 40, 200 });
  DrawRectangleRoundedLines (helpPanel, 0.1f, 8, WHITE);

  int helpY = 25;
  DrawText ("Controls", GetScreenWidth () - 310, helpY, 16, WHITE);
  helpY += 25;
  DrawText ("Left Click: Select/Modify tile", GetScreenWidth () - 310, helpY,
            10, LIGHTGRAY);
  helpY += 15;
  DrawText ("WASD/Arrows: Move camera", GetScreenWidth () - 310, helpY, 10,
            LIGHTGRAY);
  helpY += 15;
  DrawText ("Mouse Wheel: Zoom", GetScreenWidth () - 310, helpY, 10,
            LIGHTGRAY);
  helpY += 15;
  DrawText ("R: Reset camera", GetScreenWidth () - 310, helpY, 10, LIGHTGRAY);
  helpY += 15;
  DrawText ("TAB: Toggle inventory", GetScreenWidth () - 310, helpY, 10,
            LIGHTGRAY);
  helpY += 15;
  DrawText ("F1: Debug info", GetScreenWidth () - 310, helpY, 10, LIGHTGRAY);
  helpY += 15;
  DrawText ("SPACE: Pause game", GetScreenWidth () - 310, helpY, 10,
            LIGHTGRAY);

  // Debug information
  if (gameState->showDebugInfo)
    {
      Rectangle debugPanel = { 10, GetScreenHeight () - 120, 300, 110 };
      DrawRectangleRounded (debugPanel, 0.1f, 8, (Color){ 40, 40, 40, 200 });
      DrawRectangleRoundedLines (debugPanel, 0.1f, 8, WHITE);

      int debugY = GetScreenHeight () - 105;
      DrawText ("Debug Information", 20, debugY, 16, WHITE);
      debugY += 20;

      char fpsText[32];
      snprintf (fpsText, sizeof (fpsText), "FPS: %d", GetFPS ());
      DrawText (fpsText, 20, debugY, 12, LIGHTGRAY);
      debugY += 15;

      char zoomText[32];
      snprintf (zoomText, sizeof (zoomText), "Zoom: %.2f",
                gameState->cameraZoom);
      DrawText (zoomText, 20, debugY, 12, LIGHTGRAY);
      debugY += 15;

      char mouseText[64];
      snprintf (mouseText, sizeof (mouseText), "Mouse: (%.1f, %.1f)",
                gameState->mousePos.x, gameState->mousePos.y);
      DrawText (mouseText, 20, debugY, 10, LIGHTGRAY);
    }

  // Status bar
  char statusText[128];
  if (gameState->isPaused)
    {
      snprintf (statusText, sizeof (statusText),
                "PAUSED - Press SPACE to resume");
    }
  else
    {
      snprintf (statusText, sizeof (statusText),
                "Hex Board Game - Inventory: %s | Camera Zoom: %.1fx",
                gameState->showInventory ? "ON" : "OFF",
                gameState->cameraZoom);
    }
  DrawText (statusText, 10, GetScreenHeight () - 25, 12, DARKGRAY);
}
