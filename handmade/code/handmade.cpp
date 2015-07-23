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

inline tile_map*
GetTileMap(world_map* world, world_coordinates* coords)
{
  int32 x = coords->tileX >> world->tileShift;
  int32 y = coords->tileY >> world->tileShift;

  if((x >= 0 && x < world->tileMapCountX) &&
     (y >= 0 || y < world->tileMapCountY))
  {
    tile_map* result = world->tileMaps + (y * world->tileMapCountY) + x;
    return result;
  }

  return nullptr;
}

inline uint32*
GetTile(world_map* world, world_coordinates* coords)
{
  // TODO(Cristian): REMOVE THIS FIX CODE!
  if(coords->tileX < 0 || coords->tileY < 0) { return 0; }

  int32 tileMapX = coords->tileX >> world->tileShift;
  int32 tileMapY = coords->tileY >> world->tileShift;

  ASSERT(tileMapX >= 0 || tileMapX < world->tileMapCountX);
  ASSERT(tileMapY >= 0 || tileMapY < world->tileMapCountY);

  int32 tileX = coords->tileX & world->tileMask;
  int32 tileY = coords->tileY & world->tileMask;

  ASSERT(tileX >= 0 || tileX < world->tileMax);
  ASSERT(tileY >= 0 || tileY < world->tileMax);

  tile_map* tileMap = GetTileMap(world, coords);
  uint32* res = tileMap->tiles + (tileY * world->tileMax) + tileX;
  uint32 a = *res;
  return res;
}

/**
 * Modifies the coordinates so that pX and pY, which represent the offset from the tile,
 * are within the tile bounds. If they're not, we must move the tile into the correct offset.
 */
internal void
NormalizeCoordinates(world_map* world, world_coordinates* coords)
{
  // We normalize the real coordinates of the point within the tile
  real32 divX = UTILS::FloorReal32ToInt32(coords->pX / world->tileInMeters);
  real32 divY = UTILS::FloorReal32ToInt32(coords->pY / world->tileInMeters);
  // We move the tile offset
  coords->tileX += divX;
  coords->tileY += divY;

  // We correct the correct offset
  coords->pX -= divX * world->tileInMeters;
  coords->pY -= divY * world->tileInMeters;
}

internal int32_point
GetTileCoordinates(world_map* world, world_coordinates* coords)
{
  int32_point point = {};
  point.x = coords->tileX & world->tileMask;
  point.y = coords->tileY & world->tileMask;

  return point;
}

internal int32_point
GetTileMapCoordinates(world_map* world, world_coordinates* coords)
{
  int32_point point = {};
  point.x = coords->tileX >> world->tileShift;
  point.y = coords->tileY >> world->tileShift;

  return point;
}

internal world_coordinates
ModifyCoordinates(world_map* world, world_coordinates coords, real32 dX, real32 dY)
{
  coords.pX += dX;
  coords.pY += dY;

  NormalizeCoordinates(world, &coords);

  return coords;
}


internal bool32
PointValid(world_map* world, world_coordinates* coords)
{
  uint32 tileValue = *GetTile(world, coords);

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

#define PLAYER_WIDTH 0.6f
#define PLAYER_HEIGHT 1.0f

    // TODO(Cristian): Recheck coordinates offset working (packing and unpacking)
    // Now is all mingled up and it WILL bring problems
    gameState->coords.pX = 0.5f;
    gameState->coords.pY = 0.5f;
    gameState->coords.tileX = 2;
    gameState->coords.tileY = 2;

    // TODO(Cristián): This may be more appropiate to do in the platform layer
    gameMemory->graphicsInitialized = true;
  }

#define TILE_MAP_SIZE 256
  // TODO(Cristian): CHANGE THE TILEMAP INTO THE CANNONICAL PACKING!
  // We create the tiles arrays
  uint32 worldTiles[TILE_MAP_SIZE][TILE_MAP_SIZE] =
  {
    {1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1},
    {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1,  1, 1, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
    {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
    {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
    {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
    {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
    {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
    {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
    {1, 1, 1, 1,  1, 0, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1,  1,  1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1,  1},
    {1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1,  1,  1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1,  1},
    {1, 1, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
    {1, 1, 1, 1,  1, 1, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1,  1, 0, 1, 1,  1, 1, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
    {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 1, 1, 0,  1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 1, 1, 0,  1},
    {1, 0, 0, 0,  0, 1, 1, 1,  0, 0, 1, 0,  1, 1, 0, 0,  0,  0, 0, 0, 0,  0, 1, 1, 1,  0, 0, 1, 0,  1, 1, 0, 0,  1},
    {1, 0, 0, 0,  0, 1, 0, 1,  0, 0, 1, 0,  1, 0, 0, 0,  1,  1, 0, 0, 0,  0, 1, 0, 0,  0, 0, 1, 0,  1, 0, 0, 0,  1},
    {1, 0, 0, 0,  0, 1, 0, 1,  1, 1, 1, 0,  1, 0, 0, 0,  1,  1, 0, 0, 0,  0, 1, 0, 0,  1, 1, 1, 0,  1, 0, 0, 0,  1},
    {1, 0, 0, 0,  0, 0, 0, 1,  1, 1, 1, 1,  1, 1, 0, 0,  1,  1, 0, 1, 0,  0, 0, 0, 0,  0, 1, 1, 0,  1, 0, 0, 0,  1},
    {1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1},
  };

  // We create the world
  world_map world = {};
  world.tileMapCountX = 1;
  world.tileMapCountY = 1;

  world.tileShift = 8;
  world.tileMask = (1 << world.tileShift) - 1;
  world.tileMax = (1 << world.tileShift);

  world.tileInMeters = 1.0f;
  world.tileInPixels = 60;

  world.offsetX = -30;
  world.offsetY = 0;

  // We create the tile_maps
  tile_map tileMaps[1];
  tileMaps[0] = {};
  // TODO(Cristian): See if we need a world reference in each tileMap
  // tileMaps[0].world = &world;
  tileMaps[0].tiles = (uint32*)worldTiles[0];

  // We assign the tile maps
  world.tileMaps = (tile_map*)tileMaps;

  // We create the player coordinates
  world_coordinates* coords = &(gameState->coords);

  // We get the current tileMap
  tile_map* currentTileMap = GetTileMap(&world, coords);

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

      world_coordinates proposedCoords = ModifyCoordinates(&world, *coords, dX, dY);
      bool32 updateX = true;
      bool32 updateY = true;

      world_coordinates leftLowerCorner = ModifyCoordinates(&world,
                                                            proposedCoords,
                                                            -(PLAYER_WIDTH / 2), 0.0f);

      // We check left-lower corner
      bool32 moveValid = PointValid(&world, &leftLowerCorner);
      if(moveValid)
      {
        world_coordinates rightLowerCorner = ModifyCoordinates(&world,
                                                               proposedCoords,
                                                               (PLAYER_WIDTH / 2), 0.0f);
        // We check ther right-lower corner
        moveValid = PointValid(&world, &rightLowerCorner);
      }

      int32_point tileCoords = GetTileCoordinates(&world, &proposedCoords);
      char mbuffer[256];
      //wsprintf(buffer, "ms / frame: %d ms\n", msPerFrame);
      sprintf_s(mbuffer,
        "X: %f, Y: %f, TX: %d, TY: %d, WX: %d, WY: %d\n",
        proposedCoords.pX, proposedCoords.pY,
        tileCoords.x, tileCoords.y,
        0, 0
        // coords->tileMapX, coords->tileMapY
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

#define TOTAL_X 17
#define TOTAL_Y 9
  int totalHeight = TOTAL_Y * world.tileInPixels;
  int32_point playerTilePos = GetTileCoordinates(&world, coords);

  // TODO(Cristian): TILEMAP RENDERING!!!!
  for(int32 row = -1;
      row < TOTAL_Y + 1;
      row++)
  {
    for(int32 col = -1;
        col < TOTAL_X + 1;
        col++)
      {


        world_coordinates rectCoords = {};
        rectCoords.tileX = coords->tileX - (TOTAL_X / 2) + col;
        rectCoords.tileY = coords->tileY - (TOTAL_Y / 2) + row;
        uint32 tile = 0;
        uint32* tilePtr = GetTile(&world, &rectCoords);
        if(tilePtr) { tile = *tilePtr; }
        else { tile = 0.2; }

        int currentTile = 0;
        if (rectCoords.tileX == coords->tileX &&
            rectCoords.tileY == coords->tileY)
        {
          currentTile = 1;
        }

        DrawRectangle(offscreenBuffer,
                      world.offsetX + ((col - coords->pX + PLAYER_WIDTH / 2) * world.tileInPixels),
                      world.offsetY + (totalHeight - world.tileInPixels * (row + 1 - coords->pY + PLAYER_HEIGHT)),
                      world.offsetX + ((col - coords->pX + PLAYER_WIDTH / 2) * world.tileInPixels) + world.tileInPixels - 1,
                      world.offsetY + (totalHeight - world.tileInPixels * (row + 1 - coords->pY + PLAYER_HEIGHT)) + world.tileInPixels - 1,
                      currentTile * 0.8f,
                      tile * 0.5f,
                      0.7f);
      }
  }

  real32 playerPixelX = (coords->tileX * world.tileInMeters) + coords->pX;
  real32 playerPixelY = (coords->tileY * world.tileInMeters) + coords->pY;

  // Draw Player
  // // This is movement player
  // DrawRectangle(offscreenBuffer,
  //               world.offsetX + (playerPixelX - (PLAYER_WIDTH / 2)) * world.tileInPixels,
  //               world.offsetY + (totalHeight - ((playerPixelY + PLAYER_HEIGHT) * world.tileInPixels)),
  //               world.offsetX + (playerPixelX + (PLAYER_WIDTH / 2)) * world.tileInPixels,
  //               world.offsetY + (totalHeight - ((playerPixelY) * world.tileInPixels)),
  //               1.0f, 1.0f, 0.0f);
  real32 centerX = offscreenBuffer->width / 2;
  real32 centerY = offscreenBuffer->height / 2;
  DrawRectangle(offscreenBuffer,
                centerX - (PLAYER_WIDTH / 2) * world.tileInPixels,
                centerY - PLAYER_HEIGHT * world.tileInPixels,
                centerX + (PLAYER_WIDTH / 2) * world.tileInPixels,
                centerY,
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

