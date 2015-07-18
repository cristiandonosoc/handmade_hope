/*  =====================================================================
    $File: handmade.cpp
    $Creation Date: 2015-01-19
    $Last Modified: $
    $Revision: $
    $Creator: Cristián Donoso $
    $Notice: (c) Copyright 2015 Cristián Donoso $
    ===================================================================== */

#ifndef _HANDMADE_CPP_INCLUDED
#include "handmade.h"
#include "game\utils.cpp"

// TODO(Cristian): OH GOD REMOVE THIS!!!!!
#include <windows.h>
#include <stdio.h>

#include <math.h> // TODO(Cristián): Implement our own sine function

internal void
ClearScreenBuffer(game_offscreen_buffer *buffer, real32 R,
                                                 real32 G,
                                                 real32 B)
{
  // TODO(Cristián): Let's see what the optimizer does if it is passed by reference
  // instead of a pointer.
  int bitmapWidth = buffer->width;
  int bitmapHeight = buffer->height;
  int pitch = buffer->pitch;
  void *memory = buffer->memory;

  uint32 color = UTILS::RealRGBToUInt32(R, G, B);

  uint8* row = (uint8 *)memory;
  for(int y = 0;
      y < bitmapHeight;
      y++)
  {
    uint32 *pixel = (uint32 *)row;
    for(int x = 0;
        x < bitmapWidth;
        x++)
    {
      /*
        Pixel in Memory: BB GG RR xx
        Little Endian Architecture
        Pixel in Register: 0xXXRRGGBB
       */

      *pixel++ = color;
    }
    row += pitch;
  }

}

/**
 * Writes a 'weird' gradient into a memory buffer
 * @param *buffer A pointer to the buffer info struct. We can pass it by reference also
 *                because the struct does not contain the buffer itself, but a pointer, so
 *                this method does not ACTUALLY modify the struct, but the memory it points to.
 *                We pass a pointer in order to not copy a fairly big structure into memory via
 *                the stack.
 */
internal void
RenderWeirdGradient(game_offscreen_buffer *buffer, int blueOffset, int greenOffset)
{
  // TODO(Cristián): Let's see what the optimizer does if it is passed by reference
  // instead of a pointer.
  int bitmapWidth = buffer->width;
  int bitmapHeight = buffer->height;
  int pitch = buffer->pitch;
  void *memory = buffer->memory;

  uint8* row = (uint8 *)memory;
  for(int y = 0;
      y < bitmapHeight;
      y++)
  {
    uint32 *pixel = (uint32 *)row;
    for(int x = 0;
        x < bitmapWidth;
        x++)
    {
      /*
        Pixel in Memory: BB GG RR xx
        Little Endian Architecture
        Pixel in Register: 0xXXRRGGBB
       */
      uint8 blue = (uint8)(x + blueOffset);
      uint8 green = (uint8)(y + greenOffset);

      *pixel++ = blue | (green << 16);
    }
    row += pitch;
  }
}

internal void
DrawRectangle(game_offscreen_buffer* buffer, real32 realMinX, real32 realMinY,
                                             real32 realMaxX, real32 realMaxY,
                                             real32 R, real32 G, real32 B)
{
  int32 minX = UTILS::RoundReal32ToUInt32(realMinX);
  int32 maxX = UTILS::RoundReal32ToUInt32(realMaxX);
  int32 minY = UTILS::RoundReal32ToUInt32(realMinY);
  int32 maxY = UTILS::RoundReal32ToUInt32(realMaxY);


  // We make the boundaries safe
  if(minX < 0) { minX = 0; }
  if(minY < 0) { minY = 0; }
  if(maxX > buffer->width) { maxX = buffer->width; }
  if(maxY > buffer->height) { maxY = buffer->height; }


  uint32 color = UTILS::RealRGBToUInt32(R, G, B);

  for(int y = minY;
      y < maxY;
      y++)
  {
    uint8 *pixel = (uint8 *)buffer->memory +
                   y * buffer->pitch +
                   buffer->bytesPerPixel * minX;

    for (int x = minX;
         x < maxX;
         x++)
    {
      *(uint32 *)pixel = color;
      pixel += buffer->bytesPerPixel;
    }
  }
}

struct int32_point
{
  int32 x;
  int32 y;
};

inline uint32*
GetTileFromTileMap(tile_map* tileMap, int32 x, int32 y)
{
  ASSERT(x >= 0 || x < tileMap->world->tileCountX);
  ASSERT(y >= 0 || y < tileMap->world->tileCountY);

  return tileMap->tiles + (y * tileMap->world->tileCountX) + x;
}

inline int32_point
PlayerTilePosition(world_map* world, real32 pX, real32 pY)
{
  // NOTE(Cristian): We substract the tile size when the coord
  // is negative in order to get a correct tile index
  if(pX < 0.0f) { pX -= world->tileWidth; }
  if(pY < 0.0f) { pY -= world->tileHeight; }
  int32_point res = {};
  res.x = UTILS::TruncateReal32ToInt32((pX - world->offsetX) / world->tileWidth);
  res.y = UTILS::TruncateReal32ToInt32((pY - world->offsetY) / world->tileHeight);
  return res;
}

#define TILE_VALID 0
#define TILE_INVALID 1
#define TILE_OUT_LEFT 2
#define TILE_OUT_RIGHT 3
#define TILE_OUT_UP 4
#define TILE_OUT_DOWN 5

internal int32
PointValidInTileMap(tile_map* tileMap, real32 pX, real32 pY)
{
  world_map* world = tileMap->world;
  int32_point tilePos = PlayerTilePosition(world, pX, pY);

  // We check for out of tile movement
  if(tilePos.x < 0) { return TILE_OUT_LEFT; }
  if(tilePos.x >= world->tileCountX) { return TILE_OUT_RIGHT; }
  if(tilePos.y < 0) { return TILE_OUT_UP; }
  if(tilePos.y >= world->tileCountY) { return TILE_OUT_DOWN; }

  if((*(GetTileFromTileMap(tileMap, tilePos.x, tilePos.y))) == 0) // Being explicit
  {
    return TILE_VALID;
  }
  else
  {
    return TILE_INVALID;
  }
}

internal int32
PointValidInWorldMap(world_map* world, int32 tileMapX, int32 tileMapY,
                                       real32 pX, real32 pY)
{
  // We check the current tile
  tile_map* currentTileMap = world->getTileMap(tileMapX, tileMapY);
  int32 result = PointValidInTileMap(currentTileMap, pX, pY);

  if(result == TILE_VALID) { return TILE_VALID; }
  if(result == TILE_INVALID) { return TILE_INVALID; }

  // We need to check the other tile
  switch(result)
  {
    case TILE_OUT_LEFT:
      tileMapX--;
      // NOTE(Cristian): pX is negative
      pX = world->getTileMapWidth() + pX; // TODO(Cristian): This assumes uniform tileMaps!
      break;
    case TILE_OUT_RIGHT:
      tileMapX++;
      pX = pX - world->getTileMapWidth(); // TODO(Cristian): This assumes uniform tileMaps!
      break;
    case TILE_OUT_UP:
      tileMapY--;
      // NOTE(Cristian): pY is negative
      pY = world->getTileMapHeight() + pY; // TODO(Cristian): This assumes uniform tileMaps!
      break;
    case TILE_OUT_DOWN:
      tileMapY++;
      pY = pY - world->getTileMapHeight(); // TODO(Cristian): This assumes uniform tileMaps!
      break;
  }

  currentTileMap = world->getTileMap(tileMapX, tileMapY);
  // TODO(Cristian): Warn about edge of world? Now we just return TILE_INVALID
  if(currentTileMap == nullptr) { return TILE_INVALID; }

  // If it is a valid tile, we want to tell the caller what was the result.
  // A TILE_OUT_* means that there was a valid movement to such tile.
  // Otherwhise we just tell them that the movement was invalid.
  if(PointValidInTileMap(currentTileMap, pX, pY) == TILE_VALID)
  {
    return result;
  }
  else
  {
    return TILE_INVALID;
  }

}

internal void
OutputGameSound(game_sound_output_buffer *soundOutput,
                game_state *gameState)
{
  if(!soundOutput->valid) { return; }

  /**
   * We write into the buffer by writing and advancing the output pointer
   * We make two writes because we created 2 channels, which makes the buffer to look like this:
   * [int16 int16] [int16 int16] ...
   * [LEFT  RIGHT] [LEFT  RIGHT] ...
   * [  SAMPLE   ] [  SAMPLE   ] ...
   *
   */

  int32 wavePeriod = soundOutput->samplesPerSecond /
                     gameState->toneHz;

  // We cast the region pointer into int16 pointers (it is a DWORD) so we can
  // write into each channel of the sound buffer
  int16 *sampleOut = (int16 *)soundOutput->bufferMemory;
  //int32 region1SampleCount = region1Size / soundOutput->bytesPerBlock;
  // TODO(Cristián): Assert that region sizes are valid (sample multiple)
  for(int32 sampleIndex = 0;
      sampleIndex < soundOutput->sampleCount;
      sampleIndex++)
  {
#if 0
    // This is the logic required to output a sine wave
    real32 sineValue = sinf(gameState->tSine);
    int16 sampleValue = (int16)(sineValue * gameState->toneVolume);

    gameState->tSine += 2 * PI32 / (real32)wavePeriod;
    while(gameState->tSine > 2 * PI32)
    {
      gameState->tSine -= 2 * PI32;
    }

#else
    int16 sampleValue = 0; // Silence for now...
#endif

    *sampleOut++ = sampleValue;
    *sampleOut++ = sampleValue;

  }
}


// void
// GameUpdateAndRender(game_offscreen_buffer *offscreenBuffer,
//                     game_memory *gameMemory,
//                     game_input *gameInput)
extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
  ASSERT(((&gameInput->controllers[0].terminator) -
          (&gameInput->controllers[0].buttons[0])) ==
         ARRAY_COUNT(gameInput->controllers[0].buttons));
  ASSERT(sizeof(game_state) <= gameMemory->permanentStorageSize);

game_state *gameState = (game_state *)gameMemory->permanentStorage;
  if(!gameMemory->graphicsInitialized)
  {

#define PLAYER_WIDTH 20
#define PLAYER_HEIGHT 20
    gameState->playerX = 400;
    gameState->playerY = 400;
    gameState->tileMapX = 0;
    gameState->tileMapY = 0;

    // TODO(Cristián): This may be more appropiate to do in the platform layer
    gameMemory->graphicsInitialized = true;
  }

  // We create the tiles arrays
  uint32 worldTiles[4][TILE_ROWS][TILE_COLUMNS] =
  {
    {
      {1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1},
      {1, 1, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
      {1, 1, 1, 1,  1, 1, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
      {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 1, 1, 0,  1},
      {1, 0, 0, 0,  0, 1, 1, 1,  0, 0, 1, 0,  1, 1, 0, 0,  0},
      {1, 0, 0, 0,  0, 1, 0, 0,  0, 0, 1, 0,  1, 0, 0, 0,  1},
      {1, 0, 0, 0,  0, 1, 0, 0,  1, 1, 1, 0,  1, 0, 0, 0,  1},
      {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  1},
      {1, 1, 1, 1,  1, 0, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1,  1},
    },
    {
      {1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1},
      {1, 1, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
      {1, 1, 1, 1,  1, 1, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
      {1, 0, 1, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 1, 1, 0,  1},
      {0, 0, 1, 0,  0, 0, 0, 1,  0, 0, 0, 0,  1, 1, 0, 0,  1},
      {1, 0, 1, 0,  0, 0, 0, 0,  0, 0, 1, 1,  1, 0, 0, 0,  1},
      {1, 0, 1, 0,  0, 0, 0, 0,  1, 1, 1, 0,  1, 0, 0, 0,  1},
      {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  1},
      {1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1,  1},
    },
    {
      {1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1,  1},
      {1, 1, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
      {1, 1, 1, 1,  1, 1, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
      {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 1, 1, 0,  1},
      {1, 0, 0, 0,  0, 1, 1, 1,  0, 0, 1, 0,  1, 1, 0, 0,  0},
      {1, 0, 0, 0,  0, 1, 0, 1,  0, 0, 1, 0,  1, 0, 0, 0,  1},
      {1, 0, 0, 0,  0, 1, 0, 1,  1, 1, 1, 0,  1, 0, 0, 0,  1},
      {1, 0, 0, 0,  0, 0, 0, 1,  1, 1, 1, 1,  1, 1, 0, 0,  1},
      {1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1},
    },
    {
      {1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1,  1},
      {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
      {1, 0, 1, 1,  1, 1, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
      {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 1, 1, 0,  1},
      {0, 0, 0, 0,  0, 1, 1, 1,  0, 0, 1, 0,  1, 1, 0, 0,  1},
      {1, 0, 0, 0,  0, 1, 0, 0,  0, 0, 1, 0,  1, 0, 0, 0,  1},
      {1, 0, 0, 0,  0, 1, 0, 0,  1, 1, 1, 0,  1, 0, 0, 0,  1},
      {1, 0, 1, 0,  0, 0, 0, 0,  0, 1, 1, 0,  1, 0, 0, 0,  1},
      {1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1},
    },


  };

  // We create the world
  world_map world = {};
  world.tileMapCountX = 2;
  world.tileMapCountY = 2;
  world.tileCountX = TILE_COLUMNS;
  world.tileCountY = TILE_ROWS;
  world.tileWidth = 60;
  world.tileHeight = 60;
  world.offsetX = -30;
  world.offsetY = 0;

  // We create the tile_maps
  tile_map tileMaps[4];
  tileMaps[0] = {};
  tileMaps[0].world = &world;
  tileMaps[0].tiles = (uint32*)worldTiles[0];

  tileMaps[1] = tileMaps[0]; // TODO(Cristian): Don't Copy
  tileMaps[1].tiles = (uint32*)worldTiles[1];

  tileMaps[2] = tileMaps[0]; // TODO(Cristian): Don't Copy
  tileMaps[2].tiles = (uint32*)worldTiles[2];

  tileMaps[3] = tileMaps[0]; // TODO(Cristian): Don't Copy
  tileMaps[3].tiles = (uint32*)worldTiles[3];



  world.tileMaps = (tile_map*)tileMaps;

  // We get the current tileMap
  tile_map* currentTileMap = world.getTileMap(gameState->tileMapX, gameState->tileMapY);

  for(int controllerIndex = 0;
      controllerIndex < ARRAY_COUNT(gameInput->controllers);
      controllerIndex++)
  {
    game_controller_input *input =
      GetController(gameInput, controllerIndex);

    if(!input->isConnected) { continue; }

    if(input->isAnalog)
    {
      // NOTE(Cristián): Use analog movement tuning
    }
    else
    {
      // NOTE(Cristian): Use digital movement tuning
      real32 dX = 0.0f;
      real32 dY = 0.0f;
      // NOTE(Cristian): The speed is pixels/second
      real32 speed = gameInput->secondsToUpdate * 128.0f;

      if(input->moveLeft.endedDown)
      {
        dX -= speed;
      }
      if(input->moveRight.endedDown)
      {
        dX += speed;
      }
      if(input->moveUp.endedDown)
      {
        dY -= speed;
      }
      if(input->moveDown.endedDown)
      {
        dY += speed;
      }

      real32 proposedX = gameState->playerX + dX;
      real32 proposedY = gameState->playerY + dY;

      bool32 updateX = true;
      bool32 updateY = true;

      // We check left-lower corner
      int32 moveResult = PointValidInWorldMap(&world, gameState->tileMapX, gameState->tileMapY,
                                                              proposedX - PLAYER_WIDTH / 2, proposedY);
      if(moveResult == TILE_VALID)
      {
        // We check ther right-lower corner
        moveResult = PointValidInWorldMap(&world, gameState->tileMapX, gameState->tileMapY,
                                                  proposedX + PLAYER_WIDTH / 2, proposedY);
      }


      int32_point tilePos = PlayerTilePosition(&world, proposedX, proposedY);

      char mbuffer[256];
      //wsprintf(buffer, "ms / frame: %d ms\n", msPerFrame);
      sprintf_s(mbuffer,
        "X: %f, Y: %f, TX: %d, TY: %d, WX: %d, WY: %d\n",
        proposedX, proposedY,
        tilePos.x, tilePos.y,
        gameState->tileMapX, gameState->tileMapY);
      OutputDebugStringA(mbuffer);


      if(moveResult == TILE_VALID)
      {
        gameState->playerX = proposedX;
        gameState->playerY = proposedY;
      }
      else if(moveResult != TILE_INVALID)
      {

        // We switch the tile_map!
        if(moveResult == TILE_OUT_LEFT)
        {
          gameState->tileMapX--;

          // TODO(Cristian): Improve (unharcode) this!
          gameState->playerX = offscreenBuffer->width - 20;
        }
        else if(moveResult == TILE_OUT_RIGHT)
        {
          gameState->tileMapX++;

          // TODO(Cristian): Improve (unharcode) this!
          gameState->playerX = 20;
        }
        else if(moveResult == TILE_OUT_UP)
        {
          gameState->tileMapY--;

          // TODO(Cristian): Improve (unharcode) this!
          gameState->playerY = offscreenBuffer->height - 20;
        }
        else if(moveResult == TILE_OUT_DOWN)
        {
          gameState->tileMapY++;

          // TODO(Cristian): Improve (unharcode) this!
          gameState->playerY = 20;
        }
      }

    }

  }

  /*** RENDERING ***/
  ClearScreenBuffer(offscreenBuffer, 1.0f, 0.0f, 1.0f);

  for(int32 row = 0;
      row < world.tileCountY;
      row++)
  {
    for(int32 col = 0;
        col < world.tileCountX;
        col++)
      {

        int currentTile = 0;

        int32 playerTileX =
          UTILS::TruncateReal32ToInt32((gameState->playerX - world.offsetX) / world.tileWidth);
        int32 playerTileY =
          UTILS::TruncateReal32ToInt32((gameState->playerY - world.offsetY) / world.tileHeight);

        if (col == playerTileX && row == playerTileY) { currentTile = 1; }
        DrawRectangle(offscreenBuffer,
                      world.offsetX + (col * world.tileWidth),
                      world.offsetY + (row * world.tileHeight),
                      world.offsetX + (col * world.tileWidth) + world.tileWidth - 1,
                      world.offsetY + (row * world.tileHeight) + world.tileHeight - 1,
                      currentTile * 0.8f,
                      *(GetTileFromTileMap(currentTileMap, col, row)) * 0.5f,
                      0.7f);
      }
  }

  // Draw Player
  DrawRectangle(offscreenBuffer,
                gameState->playerX - PLAYER_WIDTH / 2,
                gameState->playerY - PLAYER_HEIGHT,
                gameState->playerX + PLAYER_WIDTH / 2,
                gameState->playerY,
                1.0f, 1.0f, 0.0f);

  // Draw Mouse
  DrawRectangle(offscreenBuffer,
               gameInput->mouseX, gameInput->mouseY,
               gameInput->mouseX + 10, gameInput->mouseY + 10,
               1.0f, 1.0f, 1.0f);
}

// void
// GameGetSound(game_sound_output_buffer *soundBuffer,
//              game_memory *gameMemory,
//              game_input *gameInput)
extern "C" GAME_GET_SOUND(GameGetSound)
{

  ASSERT(((&gameInput->controllers[0].terminator) -
          (&gameInput->controllers[0].buttons[0])) ==
         ARRAY_COUNT(gameInput->controllers[0].buttons));
  ASSERT(sizeof(game_state) <= gameMemory->permanentStorageSize);

  game_state *gameState = (game_state *)gameMemory->permanentStorage;
  if(!gameMemory->soundInitialized)
  {
    gameState->toneHz = 440;
    gameState->toneVolume = 7000;

    // TODO(Cristián): This may be more appropiate to do in the platform layer
    gameMemory->soundInitialized = true;
  }

  gameState->toneHz = 256;

  for(int controllerIndex = 0;
      controllerIndex < ARRAY_COUNT(gameInput->controllers);
      controllerIndex++)
  {
    game_controller_input *input =
      GetController(gameInput, controllerIndex);

    if(input->isAnalog)
    {
      // NOTE(Cristián): Use analog movement tuninY
    }
    else
    {
      if (input->moveRight.endedDown)
      {
        gameState->toneHz = 256 + (int32)(120.0f * input->moveRight.endedDown);
      }
    }
  }
  OutputGameSound(soundBuffer, gameState);
}

#if HANDMADE_WIN32
// DLL ENTRY POINT

#include "windows.h"
BOOL WINAPI DllMain(HINSTANCE hinstDLL,
                    DWORD     fdwReason,
                    LPVOID    lpvReserved)
{
  return(true);
}

#endif

#define _HANDMADE_CPP_INCLUDED
#endif

