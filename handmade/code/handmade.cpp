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

inline int32_point
CalculateTilePosition(world_map* world, world_coordinates* coords)
{
  // NOTE(Cristian): We substract the tile size when the coord
  // is negative in order to get a correct tile index
  if(coords->pX < 0.0f) { coords->pX -= world->tileWidth; }
  if(coords->pY < 0.0f) { coords->pY -= world->tileHeight; }
  int32_point res = {};
  res.x = UTILS::FloorReal32ToInt32((coords->pX - world->offsetX) / world->tileWidth);
  res.y = UTILS::FloorReal32ToInt32((coords->pY - world->offsetY) / world->tileHeight);
  return res;
}

inline void
CalculateTilePositionForCoordinates(world_map* world, world_coordinates* coords)
{
  int32_point tilePos = CalculateTilePosition(world, coords);
  coords->tileX = tilePos.x;
  coords->tileY = tilePos.y;
}

#define TILE_VALID 0
#define TILE_INVALID 1
#define TILE_OUT_LEFT 2
#define TILE_OUT_RIGHT 3
#define TILE_OUT_UP 4
#define TILE_OUT_DOWN 5

internal int32
PointValidInTileMap(world_map* world, world_coordinates* coords)
{
  // We check for out of tile movement
  if(coords->tileX < 0) { return TILE_OUT_LEFT; }
  if(coords->tileX >= world->tileMax) { return TILE_OUT_RIGHT; }
  if(coords->tileY < 0) { return TILE_OUT_UP; }
  if(coords->tileY >= world->tileMax) { return TILE_OUT_DOWN; }

  // See if we need the tileMap checking
  tile_map* tileMap = GetTileMap(world, coords);
  if(tileMap == nullptr) { return TILE_INVALID; }
  if(*(GetTile(world, coords)) == 0) // Being explicit
  {
    return TILE_VALID;
  }
  else
  {
    return TILE_INVALID;
  }
}

internal void
NormalizeCoordinates(world_map* world, world_coordinates* coords)
{
  // NOTE(Cristian): This assumes uniform tileMaps
  // if(coords->pX < 0 + world->offsetX)
  // {
  //   // NOTE(Cristian): pX is negative
  //   coords->pX = world->getTileMapWidth() + world->offsetX + coords->pX;
  // }
  // else if(coords->pX >= world->getTileMapWidth() + world->offsetX)
  // {
  //   coords->pX = coords->pX - (world->getTileMapWidth() + world->offsetX);
  // }
  // else if(coords->pY < 0 + world->offsetY)
  // {
  //   // NOTE(Cristian): pY is negative
  //   coords->pY = world->getTileMapHeight() + world->offsetY + coords->pY;
  // }
  // else if(coords->pY >= world->getTileMapHeight() + world->offsetY)
  // {
  //   coords->pY = coords->pY - (world->getTileMapHeight() + world->offsetY);
  // }

  CalculateTilePositionForCoordinates(world, coords);
}

internal int32
PointValidInWorldMap(world_map* world, world_coordinates* coords)
{
  // We check the current tile
  int32 result = PointValidInTileMap(world, coords);

  if(result == TILE_VALID) { return TILE_VALID; }
  if(result == TILE_INVALID) { return TILE_INVALID; }

  // We take the coords of the new tile
  world_coordinates newCoords = *coords;
  NormalizeCoordinates(world, &newCoords);

  tile_map* currentTileMap = GetTileMap(world, &newCoords);
  if(currentTileMap == nullptr) { return TILE_INVALID; }

  // If it is a valid tile, we want to tell the caller what was the result.
  // A TILE_OUT_* means that there was a valid movement to such tile.
  // Otherwhise we just tell them that the movement was invalid.
  // TODO(Cristian): Warn about edge of world? Now we just return TILE_INVALID
  if(PointValidInTileMap(world, &newCoords) == TILE_VALID)
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

    // TODO(Cristian): Recheck coordinates offset working (packing and unpacking)
    // Now is all mingled up and it WILL bring problems
    gameState->coords.pX = 0.5f;
    gameState->coords.pY = 0.5f;
    gameState->coords.tileX = 6;
    gameState->coords.tileY = 6;

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

  world.tileWidth = 60;
  world.tileHeight = 60;
  world.offsetX = -30;
  world.offsetY = 0;

  // We create the tile_maps
  tile_map tileMaps[1];
  tileMaps[0] = {};
  // TODO(Cristian): See if we need a world reference in each tileMap
  // tileMaps[0].world = &world;
  tileMaps[0].tiles = (uint32*)worldTiles[0];

  // tileMaps[1] = tileMaps[0]; // TODO(Cristian): Don't Copy
  // tileMaps[1].tiles = (uint32*)worldTiles[1];
  //
  // tileMaps[2] = tileMaps[0]; // TODO(Cristian): Don't Copy
  // tileMaps[2].tiles = (uint32*)worldTiles[2];
  //
  // tileMaps[3] = tileMaps[0]; // TODO(Cristian): Don't Copy
  // tileMaps[3].tiles = (uint32*)worldTiles[3];

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

      real32 proposedX = coords->pX + dX;
      real32 proposedY = coords->pY + dY;

      bool32 updateX = true;
      bool32 updateY = true;

      world_coordinates leftLowerCorner = *coords;
      leftLowerCorner.pX = proposedX - PLAYER_WIDTH / 2;
      leftLowerCorner.pY = proposedY;
      CalculateTilePositionForCoordinates(&world, &leftLowerCorner);

      // We check left-lower corner
      // int32 moveResult = PointValidInWorldMap(&world, &leftLowerCorner);
      // if(moveResult == TILE_VALID)
      // {
      //   world_coordinates rightLowerCorner = *coords;
      //   rightLowerCorner.pX = proposedX + PLAYER_WIDTH / 2;
      //   rightLowerCorner.pY = proposedY;
      //   CalculateTilePositionForCoordinates(&world, &rightLowerCorner);
      //
      //   // We check ther right-lower corner
      //   moveResult = PointValidInWorldMap(&world, &rightLowerCorner);
      // }
      //
      // int32_point tilePos = CalculateTilePosition(&world, coords);
      //
      // char mbuffer[256];
      // //wsprintf(buffer, "ms / frame: %d ms\n", msPerFrame);
      // sprintf_s(mbuffer,
      //   "X: %f, Y: %f, TX: %d, TY: %d, WX: %d, WY: %d\n",
      //   proposedX, proposedY,
      //   tilePos.x, tilePos.y,
      //   0, 0
      //   // coords->tileMapX, coords->tileMapY
      // );
      // OutputDebugStringA(mbuffer);
      //

      int32 moveResult = TILE_VALID;
      if(moveResult == TILE_VALID)
      {
        coords->pX = proposedX;
        coords->pY = proposedY;
      }
      else if(moveResult != TILE_INVALID)
      {

        // We switch the tile_map!
        if(moveResult == TILE_OUT_LEFT)
        {
          // coords->tileMapX--;
          //
          // // TODO(Cristian): Improve (unharcode) this!
          // coords->pX = offscreenBuffer->width - 20;
        }
        else if(moveResult == TILE_OUT_RIGHT)
        {
          // coords->tileMapX++;
          //
          // // TODO(Cristian): Improve (unharcode) this!
          // coords->pX = 20;
        }
        else if(moveResult == TILE_OUT_UP)
        {
          // coords->tileMapY--;
          //
          // // TODO(Cristian): Improve (unharcode) this!
          // coords->pY = offscreenBuffer->height - 20;
        }
        else if(moveResult == TILE_OUT_DOWN)
        {
          // coords->tileMapY++;
          //
          // // TODO(Cristian): Improve (unharcode) this!
          // coords->pY = 20;
        }
      }

    }

  }

  /*** RENDERING ***/
  ClearScreenBuffer(offscreenBuffer, 1.0f, 0.0f, 1.0f);

#define TOTAL_X 17
#define TOTAL_Y 9
  int totalHeight = TOTAL_Y * world.tileHeight;

  // TODO(Cristian): TILEMAP RENDERING!!!!
  for(int32 row = 0;
      row < TOTAL_Y;
      row++)
  {
    for(int32 col = 0;
        col < TOTAL_X;
        col++)
      {



        int currentTile = 0;

        int32 playerTileX =
          UTILS::FloorReal32ToInt32((coords->pX - world.offsetX) / world.tileWidth);
        int32 playerTileY =
          UTILS::FloorReal32ToInt32((coords->pY - world.offsetY) / world.tileHeight);

        if (col == playerTileX && row == playerTileY) { currentTile = 1; }

        world_coordinates rectCoords = {};
        rectCoords.tileX = col;
        rectCoords.tileY = row;
        int32 tile = *GetTile(&world, &rectCoords);

        DrawRectangle(offscreenBuffer,
                      world.offsetX + (col * world.tileWidth),
                      world.offsetY + (totalHeight - world.tileHeight * (row + 1)),
                      world.offsetX + (col * world.tileWidth) + world.tileWidth - 1,
                      world.offsetY + (totalHeight - world.tileHeight * (row + 1)) + world.tileHeight - 1,
                      currentTile * 0.8f,
                      tile * 0.5f,
                      0.7f);
      }
  }

  // Draw Player
  DrawRectangle(offscreenBuffer,
                world.offsetX + (coords->tileX + coords->pX) * world.tileWidth - PLAYER_WIDTH / 2,
                world.offsetY + (totalHeight - (coords->tileY + coords->pY) * world.tileHeight)  - PLAYER_HEIGHT,
                world.offsetX + (coords->tileX + coords->pX) * world.tileWidth + PLAYER_WIDTH / 2,
                world.offsetY + (totalHeight - (coords->tileY + coords->pY) * world.tileHeight),
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

