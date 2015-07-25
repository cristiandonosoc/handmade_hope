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
#include "handmade_coordinates.cpp"
#include "handmade_tile.cpp"
#include "game/utils.cpp"
#include "game/render.cpp"
#include "game/sound.cpp"
#include "game/memory.cpp"
#include "game/random.cpp"

// IMPORTANT(Cristian): OH GOD REMOVE THIS! THIS IS NOT PLATFORM INDEPENDENT!!!!
#include <windows.h>
#include <stdio.h>

#include <math.h> // TODO(Cristián): Implement our own sine function

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
    tileMap->tileChunkCountX = 128;
    tileMap->tileChunkCountY = 128;

    tileMap->tileShift = 4;
    tileMap->tileMask = (1 << tileMap->tileShift) - 1;
    tileMap->tileSide = (1 << tileMap->tileShift);


    tileMap->offsetX = -30;
    tileMap->offsetY = 0;

    // We append the tile_chunks
    tileMap->tileChunks = PushArray(&gameState->memoryManager,
                                    tileMap->tileChunkCountX * tileMap->tileChunkCountY,
                                    tile_chunk);
    // // We append the actual tiles
    // for(int tileChunkY = 0;
    //     tileChunkY < tileMap->tileChunkCountY;
    //     tileChunkY++)
    // {
    //   for(int tileChunkX = 0;
    //       tileChunkX < tileMap->tileChunkCountX;
    //       tileChunkX++)
    //   {
    //     tileMap->tileChunks[tileChunkY * tileMap->tileChunkCountX + tileChunkX].tiles =
    //       PushArray(&gameState->memoryManager,
    //                 GetTileChunkDim(tileMap),
    //                 uint32);
    //   }
    // }

#define TILES_PER_WIDTH 17
#define TILES_PER_HEIGHT 9
    uint32 tilesPerWidth = TILES_PER_WIDTH;
    uint32 tilesPerHeight = TILES_PER_HEIGHT;
    uint32 screens = 5;
    for(uint32 screenY = 0;
        screenY < screens;
        )
    {
      for(uint32 screenX = 0;
          screenX < screens;
          )
      {
        for(uint32 tileY = 0;
            tileY < tilesPerHeight;
            tileY++)
        {
          for(uint32 tileX = 0;
              tileX < tilesPerWidth;
              tileX++)
          {

            uint32 value = 0;
            if(tileX == 0 || tileX == TILES_PER_WIDTH - 1)
            {
              value = 1;
              if(tileY == TILES_PER_HEIGHT / 2)
              {
                value = 0;
              }
            }
            if(tileY == 0 || tileY == TILES_PER_HEIGHT - 1)
            {
              value = 1;
              if(tileX == TILES_PER_WIDTH / 2)
              {
                value = 0;
              }
            }

            tile_coordinates coord = {};
            coord.tileX = (screenX * tilesPerWidth) + tileX;
            coord.tileY = (screenY * tilesPerHeight) + tileY;

            SetTileValue(&gameState->memoryManager, tileMap, &coord, value);
          }
        }

        if(RANDOM::GetRandomUint32() % 2)
        {
          screenY++;
        }
        else
        {
          screenX++;
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
  // Outside initialization so we can live-change it
  tileMap->tileInMeters = 1.0f;
  tileMap->tileInPixels = 6;

  // We get the current tileChunk
  tile_chunk* currentTileMap = GetTileChunk(tileMap, coords);

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

      if(input->actionUp.endedDown)
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
      uint32 proposedTile = GetTileValue(tileMap, &leftLowerCorner);
      if(proposedTile == 0)
      {
        tile_coordinates rightLowerCorner = ModifyCoordinates(tileMap,
                                                              proposedCoords,
                                                              (PLAYER_WIDTH / 2), 0.0f);
        // We check ther right-lower corner
        proposedTile = GetTileValue(tileMap, &rightLowerCorner);
      }

      int32_point tileCoords = GetTileCoordinates(tileMap, &proposedCoords);
      int32_point tileChunkCoords = GetTileChunkCoordinates(tileMap, &proposedCoords);
      char mbuffer[256];
      //wsprintf(buffer, "ms / frame: %d ms\n", msPerFrame);
      sprintf_s(mbuffer,
        "X: %f, Y: %f, TX: %d, TY: %d, WX: %d, WY: %d\n",
        proposedCoords.pX, proposedCoords.pY,
        tileCoords.x, tileCoords.y,
        tileChunkCoords.x, tileChunkCoords.y
      );
      OutputDebugStringA(mbuffer);

      if(proposedTile == 0)
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

  real32 renderOffsetX = tileMap->offsetX + centerX;
  real32 renderOffsetY = tileMap->offsetY + centerY;

  int32 tileChunkX = 0xFFFFFFFF;
  int32 tileChunkY = 0xFFFFFFFF;

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

      uint32 tileValue = GetTileValue(tileMap, &rectCoords);
      if(tileValue != TILE_INVALID) { tile = tileValue; }
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

  // We want to draw the tile chunks
  // NOTE(Cristian): This code (dependending on how the passes are made) can
  // render many times the same tile chunk
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

      tile_chunk* tileChunk = GetTileChunk(tileMap, &rectCoords);
      if(tileChunk != nullptr)
      {
        int32_point tileChunkCoords = GetTileChunkCoordinates(tileMap, &rectCoords);
        if(tileChunkCoords.x != tileChunkX || tileChunkCoords.y != tileChunkY)
        {
          tileChunkX = tileChunkCoords.x;
          tileChunkY = tileChunkCoords.y;
          int32 currentTileChunkX = tileChunkX << tileMap->tileShift;
          int32 currentTileChunkY = tileChunkY << tileMap->tileShift;

          real32 gray = 0.0f;
          if(tileChunk->initialized)
          {
            gray = 1.0f;
          }

          DrawHollowRectangle(
            offscreenBuffer,
            renderOffsetX - (coords->tileX + coords->pX - currentTileChunkX) * tileMap->tileInPixels,
            renderOffsetY + (coords->tileY + coords->pY - currentTileChunkY) * tileMap->tileInPixels - tileMap->tileSide * tileMap->tileInPixels,
            renderOffsetX - (coords->tileX + coords->pX - (currentTileChunkX + tileMap->tileSide)) * tileMap ->tileInPixels,
            renderOffsetY + (coords->tileY + coords->pY - currentTileChunkY) * tileMap->tileInPixels,
            gray, gray, gray);
        }
      }
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

