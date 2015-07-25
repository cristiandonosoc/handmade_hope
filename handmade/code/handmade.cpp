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
#include "game\render.cpp"

// IMPORTANT(Cristian): OH GOD REMOVE THIS! THIS IS NOT PLATFORM INDEPENDENT!!!!
#include <windows.h>
#include <stdio.h>

#include <math.h> // TODO(Cristián): Implement our own sine function

struct int32_point
{
  int32 x;
  int32 y;
};

struct uint32_point
{
  uint32 x;
  uint32 y;
};

inline tile_chunk*
GetTileMap(tile_map* tileMap, tile_coordinates* coords)
{
  int32 x = coords->tileX >> tileMap->tileShift;
  int32 y = coords->tileY >> tileMap->tileShift;

  if((x >= 0 && x < tileMap->tileChunkCountX) &&
     (y >= 0 || y < tileMap->tileChunkCountY))
  {
    tile_chunk* result = tileMap->tileChunks + (y * tileMap->tileChunkCountY) + x;
    return result;
  }

  return nullptr;
}

inline uint32*
GetTile(tile_map* tileMap, tile_coordinates* coords)
{
  // TODO(Cristian): REMOVE THIS FIX CODE!
  if(coords->tileX < 0 || coords->tileY < 0) { return 0; }

  int32 tileChunkX = coords->tileX >> tileMap->tileShift;
  int32 tileChunkY = coords->tileY >> tileMap->tileShift;

  ASSERT(tileChunkX >= 0 || tileChunkX < tileMap->tileChunkCountX);
  ASSERT(tileChunkY >= 0 || tileChunkY < tileMap->tileChunkCountY);

  int32 tileX = coords->tileX & tileMap->tileMask;
  int32 tileY = coords->tileY & tileMap->tileMask;

  ASSERT(tileX >= 0 || tileX < tileMap->tileMax);
  ASSERT(tileY >= 0 || tileY < tileMap->tileMax);

  tile_chunk* tileChunk = GetTileMap(tileMap, coords);
  uint32* res = tileChunk->tiles + (tileY * tileMap->tileMax) + tileX;
  uint32 a = *res;
  return res;
}

/**
 * Modifies the coordinates so that pX and pY, which represent the offset from the tile,
 * are within the tile bounds. If they're not, we must move the tile into the correct offset.
 */
internal void
NormalizeCoordinates(tile_map* tileMap, tile_coordinates* coords)
{
  // We normalize the real coordinates of the point within the tile
  real32 divX = UTILS::FloorReal32ToInt32(coords->pX / tileMap->tileInMeters);
  real32 divY = UTILS::FloorReal32ToInt32(coords->pY / tileMap->tileInMeters);
  // We move the tile offset
  coords->tileX += divX;
  coords->tileY += divY;

  // We correct the correct offset
  coords->pX -= divX * tileMap->tileInMeters;
  coords->pY -= divY * tileMap->tileInMeters;
}

internal int32_point
GetTileCoordinates(tile_map* tileMap, tile_coordinates* coords)
{
  int32_point point = {};
  point.x = coords->tileX & tileMap->tileMask;
  point.y = coords->tileY & tileMap->tileMask;

  return point;
}

internal int32_point
GetTileMapCoordinates(tile_map* tileMap, tile_coordinates* coords)
{
  int32_point point = {};
  point.x = coords->tileX >> tileMap->tileShift;
  point.y = coords->tileY >> tileMap->tileShift;

  return point;
}

internal tile_coordinates
ModifyCoordinates(tile_map* tileMap, tile_coordinates coords, real32 dX, real32 dY)
{
  coords.pX += dX;
  coords.pY += dY;

  NormalizeCoordinates(tileMap, &coords);

  return coords;
}


internal bool32
PointValid(tile_map* tileMap, tile_coordinates* coords)
{
  uint32 tileValue = *GetTile(tileMap, coords);

  return tileValue == 0;
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

internal void
InitializeMemoryManager(memory_manager* memoryManager, size_t size, uint8* base)
{
  memoryManager->size = size;
  memoryManager->base = base;
  memoryManager->used = 0;
}

/**
 * Pushes certain size into the memory manager stack.
 * Returns the current free address and then advances the pointer,
 * thus working as a stack push
 */
void*
_PushSize(memory_manager* memoryManager, size_t size)
{
  ASSERT((memoryManager->used + size) <= memoryManager->size);

  void* result = memoryManager->base + memoryManager->used;
  memoryManager->used += size;

  return result;
}
/**
 * Macros define to use the underlying _PushSize function. As the macros receive the type
 * and make the case to the pointer type, they work as a template
 */
#define PushStruct(memoryManager, type) (type*)_PushSize(memoryManager, sizeof(type))
#define PushArray(memoryManager, count, type) (type*)_PushSize(memoryManager, (count) * sizeof(type))

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
    // We initialize the memory manager right after the gamestate struct
    // in the permament storage
    // NOTE(Cristian): For now, the memory manager has all the permament storage
    // after the game state.
    InitializeMemoryManager(&gameState->memoryManager,
                            gameMemory->permanentStorageSize - sizeof(game_state),
                            (uint8*)gameMemory->permanentStorage + sizeof(game_state));

    // We generate the world and store it in the permanent storage
    gameState->world = PushStruct(&gameState->memoryManager, world_definition);
    world_definition* world = gameState->world;

    // We create the tileMap
    world->tileMap = PushStruct(&gameState->memoryManager, tile_map);
    tile_map* tileMap = world->tileMap;
    tileMap->tileChunkCountX = 1;
    tileMap->tileChunkCountY = 1;

    tileMap->tileShift = 8;
    tileMap->tileMask = (1 << tileMap->tileShift) - 1;
    tileMap->tileMax = (1 << tileMap->tileShift);

    tileMap->tileInMeters = 1.0f;
    tileMap->tileInPixels = 6;

    tileMap->offsetX = -30;
    tileMap->offsetY = 0;


    // We append the tile_chunks
    tileMap->tileChunks = PushArray(&gameState->memoryManager,
                                    tileMap->tileChunkCountX * tileMap->tileChunkCountY,
                                    tile_chunk);
    // We append the actual tiles
    for(int tileChunkY = 0;
        tileChunkY < tileMap->tileChunkCountY;
        tileChunkY++)
    {
      for(int tileChunkX = 0;
          tileChunkX < tileMap->tileChunkCountX;
          tileChunkX++)
      {
        tileMap->tileChunks[tileChunkY * tileMap->tileChunkCountX + tileChunkX].tiles =
          PushArray(&gameState->memoryManager,
                    tileMap->tileMax * tileMap->tileMax,
                    uint32);
      }
    }

#define TILES_PER_WIDTH 17
#define TILES_PER_HEIGHT 9
    for(uint32 screenY = 0;
        screenY < 32;
        screenY++)
    {
      for(uint32 screenX = 0;
          screenX < 32;
          screenX++)
      {
        for(uint32 tileY = 0;
            tileY < TILES_PER_WIDTH;
            tileY++)
        {
          for(uint32 tileX = 0;
              tileX < TILES_PER_HEIGHT;
              tileX++)
          {

          }
        }
      }
    }

#define PLAYER_WIDTH 0.6f
#define PLAYER_HEIGHT 1.0f

    // TODO(Cristian): Recheck coordinates offset working (packing and unpacking)
    // Now is all mingled up and it WILL bring problems
    gameState->coords.pX = 0.0f;
    gameState->coords.pY = 0.0f;
    gameState->coords.tileX = 2;
    gameState->coords.tileY = 2;

    // TODO(Cristián): This may be more appropiate to do in the platform layer
    gameMemory->graphicsInitialized = true;
  }

  // We create the player coordinates
  tile_coordinates* coords = &(gameState->coords);

  // We obtain the world data from the gameState
  world_definition* world = gameState->world;
  tile_map* tileMap = world->tileMap;

  // We get the current tileChunk
  tile_chunk* currentTileMap = GetTileMap(tileMap, coords);

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
      real32 speed = gameInput->secondsToUpdate * 2.0f;

      if(input->actionUp.endedDown)\
      {
        speed *= 5;
      }
      if(input->moveLeft.endedDown)
      {
        dX -= speed;
      }
      if(input->moveRight.endedDown)
      {
        dX += speed;
      }
      if(input->moveDown.endedDown)
      {
        dY -= speed;
      }
      if(input->moveUp.endedDown)
      {
        dY += speed;
      }

      tile_coordinates proposedCoords = ModifyCoordinates(tileMap, *coords, dX, dY);
      bool32 updateX = true;
      bool32 updateY = true;

      tile_coordinates leftLowerCorner = ModifyCoordinates(tileMap,
                                                           proposedCoords,
                                                           -(PLAYER_WIDTH / 2), 0.0f);

      // We check left-lower corner
      bool32 moveValid = PointValid(tileMap, &leftLowerCorner);
      if(moveValid)
      {
        tile_coordinates rightLowerCorner = ModifyCoordinates(tileMap,
                                                              proposedCoords,
                                                              (PLAYER_WIDTH / 2), 0.0f);
        // We check ther right-lower corner
        moveValid = PointValid(tileMap, &rightLowerCorner);
      }

      int32_point tileCoords = GetTileCoordinates(tileMap, &proposedCoords);
      char mbuffer[256];
      //wsprintf(buffer, "ms / frame: %d ms\n", msPerFrame);
      sprintf_s(mbuffer,
        "X: %f, Y: %f, TX: %d, TY: %d, WX: %d, WY: %d\n",
        proposedCoords.pX, proposedCoords.pY,
        tileCoords.x, tileCoords.y,
        0, 0
        // coords->tileChunkX, coords->tileChunkY
      );
      OutputDebugStringA(mbuffer);

      if(moveValid)
      {
        *coords = proposedCoords;
      }
    }
  }

  /*** RENDERING ***/
  ClearScreenBuffer(offscreenBuffer, 1.0f, 0.0f, 1.0f);

  int totalHeight = TILES_PER_HEIGHT * tileMap->tileInPixels;
  int32_point playerTilePos = GetTileCoordinates(tileMap, coords);


  real32 centerX = offscreenBuffer->width / 2;
  real32 centerY = offscreenBuffer->height / 2;

  // TODO(Cristian): TILEMAP RENDERING!!!!
  int32 renderSize = 30;
  int32 minX = coords->tileX - (TILES_PER_WIDTH / 2 + 1 + renderSize);
  int32 maxX = coords->tileX + (TILES_PER_WIDTH / 2 + 2 + renderSize);
  int32 minY = coords->tileY - (TILES_PER_HEIGHT / 2 + 1 + renderSize);
  int32 maxY = coords->tileY + (TILES_PER_HEIGHT / 2 + 2 + renderSize);
  for(int32 tileY = minY;
      tileY < maxY;
      tileY++)
  {
    for(int32 tileX = minX;
        tileX < maxX;
        tileX++)
      {


        tile_coordinates rectCoords = {};
        rectCoords.tileX = tileX;
        rectCoords.tileY = tileY;
        real32 tile = 0;
        uint32* tilePtr = GetTile(tileMap, &rectCoords);
        if(tilePtr) { tile = *tilePtr; }
        else { tile = 0.5f; }

        int currentTile = 0;
        if (tileX == coords->tileX &&
            tileY == coords->tileY)
        {
          currentTile = 1;
        }


        // NOTE(Cristian): We substract one because we are also rendering one extra tile in
        // every direction
        int32 tileOffsetX = tileX - coords->tileX;
        int32 tileOffsetY = -(tileY - coords->tileY);
        DrawRectangle(offscreenBuffer,
                      tileMap->offsetX + centerX + ((tileOffsetX - coords->pX) * tileMap->tileInPixels),
                      tileMap->offsetY + centerY + ((tileOffsetY + coords->pY) * tileMap->tileInPixels) - tileMap->tileInPixels,
                      tileMap->offsetX + centerX + ((tileOffsetX - coords->pX) * tileMap->tileInPixels) + tileMap->tileInPixels - 1,
                      tileMap->offsetY + centerY + (((tileOffsetY + coords->pY) * tileMap->tileInPixels) - 1),
                      currentTile * 0.8f,
                      tile * 0.5f,
                      0.7f);
      }
  }

  // Draw Player
  DrawRectangle(offscreenBuffer,
                tileMap->offsetX + centerX - (PLAYER_WIDTH / 2) * tileMap->tileInPixels,
                tileMap->offsetY + centerY - PLAYER_HEIGHT * tileMap->tileInPixels,
                tileMap->offsetX + centerX + (PLAYER_WIDTH / 2) * tileMap->tileInPixels,
                tileMap->offsetY + centerY,
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

