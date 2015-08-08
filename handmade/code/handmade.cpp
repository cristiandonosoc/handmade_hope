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
#include "game/coordinates.cpp"
#include "game/tiles.cpp"
#include "game/render.cpp"
#include "game/sound.cpp"
#include "game/memory.cpp"
#include "game/random.cpp"

#include "utils/float.cpp"
#include "utils/bit.cpp"

// // IMPORTANT(Cristian): OH GOD REMOVE THIS! THIS IS NOT PLATFORM INDEPENDENT!!!!
// #include <windows.h>
// #include <stdio.h>

internal bitmap_definition
DEBUGLoadBMP(thread_context* thread,
             debug_platform_read_entire_file* readEntireFileFunction,
             char* fileName)
{
  bitmap_definition result = {};

  game_file readResult = readEntireFileFunction(thread, fileName);
  if(readResult.contentSize != 0)
  {
    bitmap_header* header = (bitmap_header*)readResult.content;
    result.header = *header;
    result.pixels = (uint32*)((uint8*)readResult.content + header->bitmapStart);

    // We transform the pixels to the correct format because they come offset by the
    // "color masks", which tell where in the int32 the channels actually are.
    uint32 alphaMask = ~(header->redMask | header->greenMask | header->blueMask);
    UTILS::BIT::bit_scan_result redScan = UTILS::BIT::FindLeastSignificantSetBit(header->redMask);
    UTILS::BIT::bit_scan_result greenScan = UTILS::BIT::FindLeastSignificantSetBit(header->greenMask);
    UTILS::BIT::bit_scan_result blueScan = UTILS::BIT::FindLeastSignificantSetBit(header->blueMask);
    UTILS::BIT::bit_scan_result alphaScan = UTILS::BIT::FindLeastSignificantSetBit(alphaMask);
    ASSERT(redScan.found);
    ASSERT(greenScan.found);
    ASSERT(blueScan.found);
    ASSERT(alphaScan.found);

    int32 redShift = 16 - (int32)redScan.index;
    int32 greenShift = 8 - (int32)greenScan.index;
    int32 blueShift = 0 - (int32)blueScan.index;
    int32 alphaShift = 24 - (int32)alphaScan.index;

    // Now we have by how much each channel was shifted to the right.
    // So now we have to swivel them up
    uint32* head = result.pixels;
    uint32* tail = (uint32*)((uint8*)result.pixels + header->imageSize);
    for(int y = 0;
        y < header->height;
        y++)
    {
      for(int x = 0;
          x < header->width;
          x++)
      {
        // We shift down the colors by the shift calculated amount
        // and then place them in the correct value
#if 0
        *head++ = ((((*head >> alphaScan.index) & 0xFF) << 24) |
                   (((*head >> redScan.index) & 0xFF) << 16) |
                   (((*head >> greenScan.index) & 0xFF) << 8) |
                   (((*head >> blueScan.index) & 0xFF) << 0));
#else
        uint32 c = *head;
        *head++ = (UTILS::BIT::RotateLeft(c & header->redMask, redShift) |
                   UTILS::BIT::RotateLeft(c & header->greenMask, greenShift) |
                   UTILS::BIT::RotateLeft(c & header->blueMask, blueShift) |
                   UTILS::BIT::RotateLeft(c & alphaMask, blueShift));
#endif
      }
    }
  }

  return result;
}

#define PLAYER_WIDTH 0.6f
#define PLAYER_HEIGHT 1.0f

internal void
InitializeEntity(entity_def* entity)
{
  *entity = {};

  entity->exists = true;
  entity->pos.tile.x = 3;
  entity->pos.tile.y = 3;
  entity->width = PLAYER_WIDTH;
  entity->height = PLAYER_HEIGHT;
}

inline entity_def*
GetEntity(game_state* gameState, int32 index)
{

  if(index < 0) { return nullptr; }
  if(gameState->entityCount == 0) { return nullptr; }

  ASSERT(index < gameState->entityCount);

  entity_def* result = &gameState->entities[index];
  return result;
}

// Returns the index of the entity in out entity array
inline uint32
CreateEntity(game_state* gameState)
{
  ASSERT(gameState->entityCount < ARRAY_COUNT(gameState->entities));

  entity_def* entity = &gameState->entities[gameState->entityCount];
  *entity = {}; // Safety clear

  uint32 result = gameState->entityCount;
  gameState->entityCount++;

  return result;
}

inline bool32
ValidEntity(entity_def* entity)
{
  if(!entity) { return false; }

  bool32 result = entity->exists;
  return result;
}


/**
 * Test when a collision would occur against an axis (the sub axis)
 *
 * The algorithm has an origin (rel) and a distance to advance in the whole step
 * (t is [0, 1]). What we do is check what the value would be against the check axis (sub),
 * and check if it falls inside a range we define.
 *
 * We also assign the time required to make that collision
 */
internal bool32
TestWall(real32 testValue, real32 relMain, real32 relSub, real32 deltaMain, real32 deltaSub,
         real32 testMin, real32 testMax, real32* tMin)
{
  bool32 hit = false;
  real32 tEpsilon = 0.0000f; // We create this so the player moves a little bit back than the collision
  if(deltaMain != 0.0f)
  {
    real32 t = (testValue - relMain) / deltaMain;
    if((t >= 0.0f) && (t < *tMin))
    {
      real32 sub = relSub + t * deltaSub;  // This is the y of the point where we cross the wall line
      if((sub >= testMin) && (sub <= testMax))
      {
        // We collided
        *tMin = t - tEpsilon;
        hit = true;
      }
    }
  }

  return hit;
}

internal void
UpdateControlledEntity(entity_def* entity, game_controller_input* input,
    tile_map* tileMap,
    game_input* gameInput, game_state* gameState)
{
  // NOTE(Cristian): Use digital movement tuning
  vector2D<real32> ddPlayerPos = {};

  // TODO(Cristian): Check W+S+RIGHT broken combination
  // Is it the platform layer o a keyboard failure????
  if(input->moveLeft.endedDown)
  {
    ddPlayerPos.x -= 1.0f;
  }
  if(input->moveRight.endedDown)
  {
    ddPlayerPos.x += 1.0f;
  }
  if(input->moveDown.endedDown)
  {
    ddPlayerPos.y -= 1.0f;
  }
  if(input->moveUp.endedDown)
  {
    ddPlayerPos.y += 1.0f;
  }

  real32 moveAccel = 20.0f;
  if(input->actionRight.endedDown)
  {
    moveAccel = 70.0f;
  }

  // We want only key-release
  if(!input->actionUp.endedDown && (input->actionUp.halfTransitionCount > 0))
  {
    entity->pos.tile.z ^= 1;
  }

  if(ddPlayerPos.x != 0.0f || ddPlayerPos.y != 0.0f)
  {
    // We update the facing direction
    if(UTILS::FLOAT::AbsoluteReal32(ddPlayerPos.y) > UTILS::FLOAT::AbsoluteReal32(ddPlayerPos.x))
    {
      if(ddPlayerPos.y > 0) { entity->facingDirection = 1; }
      else { entity->facingDirection = 3; }
    }
    else
    {
      if(ddPlayerPos.x > 0) { entity->facingDirection = 2; }
      else { entity->facingDirection = 0; }
    }
  }


  // We only normalize for bigger vectors. This way we support
  // lower vectors from gamepads
  if(MATH::LengthSq(ddPlayerPos) > 1.0f)
  {
    ddPlayerPos = NormalizeVector(ddPlayerPos);
  }


  // We create a simple drag
  // NOTE(Cristian): Learn and use ODE (Ordinary Differential Equations)
  ddPlayerPos *= moveAccel;
  ddPlayerPos -= 5.25f * entity->dPos;

  vector2D<real32> playerPos = {entity->pos.pX, entity->pos.pY};
  vector2D<real32> delta = (((ddPlayerPos * Square(gameInput->secondsToUpdate)) / 2) +
                             (entity->dPos * gameInput->secondsToUpdate));

  // We calculate the velocity
  entity->dPos += ddPlayerPos * gameInput->secondsToUpdate;

#if 0
  tile_coordinates proposedCoords = ModifyCoordinates(tileMap, entity->pos, diff.x, diff.y);
  tile_coordinates colCoords = {};
  tile_coordinates leftLowerCorner = ModifyCoordinates(tileMap,
      proposedCoords,
      -(entity->width/ 2), 0.0f);

  // We check left-lower corner
  uint32 proposedTile = GetTileValue(tileMap, &leftLowerCorner);
  if(proposedTile != 0) // COLLISION
  {
    colCoords = leftLowerCorner;
  }
  else
  {
    tile_coordinates rightLowerCorner = ModifyCoordinates(tileMap,
        proposedCoords,
        (entity->width/ 2), 0.0f);
    // We check ther right-lower corner
    proposedTile = GetTileValue(tileMap, &rightLowerCorner);
    if(proposedTile != 0) // COLLISION
    {
      colCoords = rightLowerCorner;
    }
  }
#endif

  tile_coordinates pos = entity->pos;
  tile_coordinates proposedCoords = ModifyCoordinates(tileMap, entity->pos, delta.x, delta.y);

#if 1
  // We need to check all the tiles where we could have collision
  vector2D<int32> minTile = {MIN(entity->pos.tile.x, proposedCoords.tile.x),
                             MIN(entity->pos.tile.y, proposedCoords.tile.y)};

  vector2D<int32> maxTile = {MAX(entity->pos.tile.x, proposedCoords.tile.x),
                             MAX(entity->pos.tile.y, proposedCoords.tile.y)};
  vector2D<real32> dist = Distance(tileMap, pos, proposedCoords);
  int32 tileZ = entity->pos.tile.z;
  real32 tRemaining = 1.0f;

  // We do four iteration in order to make the entity correctly respond
  // to a collision and not get "stuck"
  for(int iteration = 0;
      ((iteration < 4) && (tRemaining > 0.0f));
      iteration++)
  {
    real32 tMin = 1.0f;       // The minimum collision we detected

    // We check all the tiles
    vector2D<real32> wallNormal = {};
    for(int32 tileY = minTile.y;
        tileY <= maxTile.y;
        tileY++)
    {
      for(int32 tileX = minTile.x;
          tileX <= maxTile.x;
          tileX++)
      {
        uint32 tileValue = GetTileValue(tileMap, tileX, tileY, tileZ);
        if(tileValue != 0)
        {
          // We check collision against the left wall
          tile_coordinates testTile = GenerateCoords(tileX, tileY, tileZ);
          vector2D<real32> rel = Distance(tileMap, testTile, pos);
          vector2D<real32> minCorner = {0.0f, 0.0f};
          vector2D<real32> maxCorner = {tileMap->tileInMeters, tileMap->tileInMeters};

          // We check all four walls
          // LEFT
          if(TestWall(minCorner.x, rel.x, rel.y, delta.x, delta.y, minCorner.y, maxCorner.y, &tMin))
          {
            wallNormal = vector2D<real32>{-1, 0};
          }
          // RIGHT
          if(TestWall(maxCorner.x, rel.x, rel.y, delta.x, delta.y, minCorner.y, maxCorner.y, &tMin))
          {
            wallNormal = vector2D<real32>{1, 0};
          }
          // BOTTOM
          if(TestWall(minCorner.y, rel.y, rel.x, delta.y, delta.x, minCorner.x, maxCorner.x, &tMin))
          {
            wallNormal = vector2D<real32>{0, -1};
          }
          // TOP
          if(TestWall(maxCorner.y, rel.y, rel.x, delta.y, delta.x, minCorner.x, maxCorner.x, &tMin))
          {
            wallNormal = vector2D<real32>{0, 1};
          }
        }
      }
    }

    entity->pos = ModifyCoordinates(tileMap, entity->pos, tMin * delta.x, tMin * delta.y);
    if(tMin < 1.0f)
    {
      // NOTE(Cristian): If we collide, we move the entity an epsilon back so that we have
      // a safe margin with floating point imprecision
      real32 moveEpsilon = -0.001f;
      vector2D<real32> nDelta = NormalizeVector(delta);
      entity->pos = ModifyCoordinates(tileMap, entity->pos, moveEpsilon*nDelta.x, moveEpsilon*nDelta.y);

      entity->dPos = entity->dPos - 1.0f*InnerProduct(entity->dPos, wallNormal)*wallNormal;
      delta = delta - 1.0f*InnerProduct(delta, wallNormal)*wallNormal;
    }
    tRemaining -= tMin*tRemaining; // We remove how much of the much left we had left
  }

#else
  uint32 proposedTile = GetTileValue(tileMap, &proposedCoords);

  if(proposedTile == 0)
  {
    entity->pos = proposedCoords;
  }
  else
  {
    // We had collision, so we go and do a bounce
    // For this we need to know the movement direction
    vector2D<real32> r = {};
    if(proposedCoords.tile.x < entity->pos.tile.x)
    {
      r = vector2D<real32>{1, 0};
    }
    if(proposedCoords.tile.x > entity->pos.tile.x)
    {
      r = vector2D<real32>{-1, 0};
    }
    if(proposedCoords.tile.y < entity->pos.tile.y)
    {
      r = vector2D<real32>{0, 1};
    }
    if(proposedCoords.tile.y > entity->pos.tile.y)
    {
      r = vector2D<real32>{0, -1};
    }

    entity->dPos = entity->dPos - 2 * InnerProduct(entity->dPos, r) * r;
  }
#endif

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
    // Debug loading of bitmaps
    gameState->background = DEBUGLoadBMP(nullptr,
                                         gameMemory->DEBUGPlatformReadEntireFileFunction,
                                         "test/test_background.bmp");

    hero_bitmap* heroBitmap = gameState->heroBitmaps;

    debug_platform_read_entire_file* readFunction = gameMemory->DEBUGPlatformReadEntireFileFunction;

    heroBitmap->torso = DEBUGLoadBMP(nullptr, readFunction, "test/test_hero_left_torso.bmp");
    heroBitmap->cape = DEBUGLoadBMP(nullptr, readFunction, "test/test_hero_left_cape.bmp");
    heroBitmap->head = DEBUGLoadBMP(nullptr, readFunction, "test/test_hero_left_head.bmp");
    heroBitmap->offsetX = 72;
    heroBitmap->offsetY = 184;
    heroBitmap++;

    heroBitmap->torso = DEBUGLoadBMP(nullptr, readFunction, "test/test_hero_back_torso.bmp");
    heroBitmap->cape = DEBUGLoadBMP(nullptr, readFunction, "test/test_hero_back_cape.bmp");
    heroBitmap->head = DEBUGLoadBMP(nullptr, readFunction, "test/test_hero_back_head.bmp");
    heroBitmap->offsetX = 72;
    heroBitmap->offsetY = 184;
    heroBitmap++;

    heroBitmap->torso = DEBUGLoadBMP(nullptr, readFunction, "test/test_hero_right_torso.bmp");
    heroBitmap->cape = DEBUGLoadBMP(nullptr, readFunction, "test/test_hero_right_cape.bmp");
    heroBitmap->head = DEBUGLoadBMP(nullptr, readFunction, "test/test_hero_right_head.bmp");
    heroBitmap->offsetX = 72;
    heroBitmap->offsetY = 184;
    heroBitmap++;

    heroBitmap->torso = DEBUGLoadBMP(nullptr, readFunction, "test/test_hero_front_torso.bmp");
    heroBitmap->cape = DEBUGLoadBMP(nullptr, readFunction, "test/test_hero_front_cape.bmp");
    heroBitmap->head = DEBUGLoadBMP(nullptr, readFunction, "test/test_hero_front_head.bmp");
    heroBitmap->offsetX = 72;
    heroBitmap->offsetY = 184;


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
    tileMap->tileChunkCountZ = 2;

    tileMap->tileShift = 4;
    tileMap->tileMask = (1 << tileMap->tileShift) - 1;
    tileMap->tileSide = (1 << tileMap->tileShift);

    // We append the tile_chunks
    tileMap->tileChunks = PushArray(&gameState->memoryManager,
        tileMap->tileChunkCountZ * tileMap->tileChunkCountX * tileMap->tileChunkCountY,
        tile_chunk);

#define TILES_PER_WIDTH 17
#define TILES_PER_HEIGHT 9

    /**
     * We set the tile_chunk data for SOME tile_chunk
     */
    uint32 tilesPerWidth = TILES_PER_WIDTH;
    uint32 tilesPerHeight = TILES_PER_HEIGHT;
    uint32 screens = 32;
    for(int32 screenZ = 0;
        screenZ < tileMap->tileChunkCountZ;
        screenZ++)
    {
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
              coord.tile.x = (screenX * tilesPerWidth) + tileX;
              coord.tile.y = (screenY * tilesPerHeight) + tileY;
              coord.tile.z = screenZ;

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
    }

    // We initialize the controllers as not having any entities
    for(uint32 controllerIndex = 0;
        controllerIndex < ARRAY_COUNT(gameState->entityIndexForController);
        controllerIndex++)
    {
      gameState->entityIndexForController[controllerIndex] = -1;
    }

    gameState->cameraPos = {2, 2};
    gameState->cameraPos.pX = 0.0f;
    gameState->cameraPos.pY = 0.0f;

    // TODO(Cristián): This may be more appropiate to do in the platform layer
    gameMemory->graphicsInitialized = true;
  }

  // We obtain the world data from the gameState
  world_definition* world = gameState->world;
  tile_map* tileMap = world->tileMap;
  // Outside initialization so we can live-change it
  tileMap->tileInMeters = 1.0f;
  real32 tileInPixels = 60;
  real32 metersToPixels = tileInPixels / tileMap->tileInMeters;

  // Temporal entity variable to be used throughout the code
  entity_def* entity = nullptr;

  /**
   * CONTROLLER UPDATE LOOP
   */
  for(int controllerIndex = 0;
      controllerIndex < ARRAY_COUNT(gameInput->controllers);
      controllerIndex++)
  {
    game_controller_input *input =
      GetController(gameInput, controllerIndex);

    if(!input->isConnected) { continue; }

    entity = GetEntity(gameState, gameState->entityIndexForController[controllerIndex]);
    if(!input->start.endedDown && (input->start.halfTransitionCount > 0)) // release
    {
      if(ValidEntity(entity))
      {
        // We clear the entity
        *entity = {};
        entity = nullptr;
        gameState->entityIndexForController[controllerIndex] = -1;
      }
      else
      {
        // We create the entity (player comes in to play)
        gameState->entityIndexForController[controllerIndex] = CreateEntity(gameState);
        entity = GetEntity(gameState, gameState->entityIndexForController[controllerIndex]);

        InitializeEntity(entity);

        // We attach the camera to the entity
        gameState->cameraFollowingEntityIndex = gameState->entityIndexForController[controllerIndex];
      }
    }

    if(input->isAnalog)
    {
      // NOTE(Cristián): Use analog movement tuning
    }
    else
    {
      if(ValidEntity(entity))
      {
        UpdateControlledEntity(entity, input, tileMap, gameInput, gameState);
      }
    }
  }

  /**
   * ENTITY UPDATE LOOP
   */
  entity = gameState->entities;
  for(int32 entityIndex = 0;
      entityIndex < gameState->entityCount;
      entityIndex++, entity++)
  {
    if(entity->exists)
    {

    }
  }

  // vector2D<int32> tileCoords = GetTileCoordinates(tileMap, &proposedCoords);
  // vector3D<int32> tileChunkCoords = GetTileChunkCoordinates(tileMap, &proposedCoords);
  // char mbuffer[256];
  // //wsprintf(buffer, "ms / frame: %d ms\n", msPerFrame);
  // sprintf_s(mbuffer,
  //     "X: %f, Y: %f, TX: %d, TY: %d, WX: %d, WY: %d, WZ: %d\ndX:%f, dY:%f, ddX:%f, ddY:%f\n",
  //     proposedCoords.pX, proposedCoords.pY,
  //     tileCoords.x, tileCoords.y,
  //     tileChunkCoords.x, tileChunkCoords.y, tileChunkCoords.z,
  //     currentEntity->dPos.x, currentEntity->dPos.y,
  //     ddPlayerPos.x, ddPlayerPos.y
  //     );
  // OutputDebugStringA(mbuffer);



  // We update the camera position to the plater Position
  entity_def* cameraFollowingEntity = GetEntity(gameState, gameState->cameraFollowingEntityIndex);
  if(ValidEntity(cameraFollowingEntity))
  {
    gameState->cameraPos = cameraFollowingEntity->pos;
  }


  // We get the current tileChunk from the camera
  tile_chunk* currentTileMap = GetTileChunk(tileMap, &gameState->cameraPos);

  /*** RENDERING ***/

  ClearScreenBuffer(offscreenBuffer, 1.0f, 0.0f, 1.0f);

  DrawBitmap(offscreenBuffer, gameState->background, 0, 0, 0, 0, true);

  int totalHeight = TILES_PER_HEIGHT * tileInPixels;
  vector2D<int32> playerTilePos = GetTileCoordinates(tileMap, &gameState->cameraPos);

  real32 offsetX = -30.0f;
  real32 offsetY = 0;

  real32 centerX = offscreenBuffer->width / 2;
  real32 centerY = offscreenBuffer->height / 2;

  real32 renderOffsetX = offsetX + centerX;
  real32 renderOffsetY = offsetY + centerY;

  int32 tileChunkX = 0xFFFFFFFF;
  int32 tileChunkY = 0xFFFFFFFF;
  int32 tileChunkZ = 0xFFFFFFFF;

  int32 renderSize = 1;
  int32 minX = gameState->cameraPos.tile.x - (TILES_PER_WIDTH / 2 + 1 + renderSize);
  int32 maxX = gameState->cameraPos.tile.x + (TILES_PER_WIDTH / 2 + 2 + renderSize);
  int32 minY = gameState->cameraPos.tile.y - (TILES_PER_HEIGHT / 2 + 1 + renderSize);
  int32 maxY = gameState->cameraPos.tile.y + (TILES_PER_HEIGHT / 2 + 2 + renderSize);
  for(int32 tileY = minY;
      tileY < maxY;
      tileY++)
  {
    for(int32 tileX = minX;
        tileX < maxX;
        tileX++)
    {
      tile_coordinates rectCoords = {};
      rectCoords.tile.x = tileX;
      rectCoords.tile.y = tileY;
      rectCoords.tile.z = gameState->cameraPos.tile.z;
      real32 tile = 0;

      uint32 tileValue = GetTileValue(tileMap, &rectCoords);
      if(tileValue != TILE_INVALID)
      {
        if(tileValue == 0) { continue; }
        tile = tileValue;
      }
      else
      {
        // tile = 0.5f;
        continue;
      }

      int currentTile = 0;
      if (tileX == gameState->cameraPos.tile.x &&
          tileY == gameState->cameraPos.tile.y)
      {
        currentTile = 1;
      }

      // NOTE(Cristian): We substract one because we are also rendering one extra tile in
      // every direction
      int32 tileOffsetX = tileX - gameState->cameraPos.tile.x;
      int32 tileOffsetY = -(tileY - gameState->cameraPos.tile.y);
      DrawTileRelativeToCenter(offscreenBuffer,
          renderOffsetX, renderOffsetY,
          tileX - gameState->cameraPos.tile.x, tileY - gameState->cameraPos.tile.y,                 // tile offset
          gameState->cameraPos.pX, gameState->cameraPos.pY,                                       // real offset
          tileMap->tileInMeters, tileMap->tileInMeters,                 // tile size
          -1, -1,                                                       // pixel padding
          tileMap->tileInMeters, metersToPixels,                        // tile-to-pixel transforms
          currentTile * 0.8f, tile * 0.5f, 0.7f);                       // color data
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
      rectCoords.tile.x = tileX;
      rectCoords.tile.y = tileY;
      rectCoords.tile.z = gameState->cameraPos.tile.z;

      tile_chunk* tileChunk = GetTileChunk(tileMap, &rectCoords);
      if(tileChunk != nullptr)
      {
        vector3D<int32> tileChunkCoords = GetTileChunkCoordinates(tileMap, &rectCoords);
        if(tileChunkCoords.x != tileChunkX ||
           tileChunkCoords.y != tileChunkY ||
           tileChunkCoords.z != tileChunkZ)
        {
          tileChunkX = tileChunkCoords.x;
          tileChunkY = tileChunkCoords.y;
          tileChunkZ = tileChunkCoords.z;
          int32 currentTileChunkX = tileChunkCoords.x << tileMap->tileShift;
          int32 currentTileChunkY = tileChunkCoords.y << tileMap->tileShift;
          int32 currentTileChunkZ = tileChunkCoords.z;

          real32 gray = 0.0f;
          if(tileChunk->initialized)
          {
            gray = 1.0f;
          }

          vector2D<real32> tileChunkMin = {
            renderOffsetX - ((gameState->cameraPos.tile.x - currentTileChunkX) * tileMap->tileInMeters + gameState->cameraPos.pX ) * metersToPixels,
            renderOffsetY + (gameState->cameraPos.tile.y + gameState->cameraPos.pY - currentTileChunkY) * metersToPixels - tileMap->tileSide * tileMap->tileInMeters * metersToPixels,
          };

          vector2D<real32> tileChunkMax = {
            renderOffsetX - ((gameState->cameraPos.tile.x - currentTileChunkX - tileMap->tileSide) + gameState->cameraPos.pX) * metersToPixels,
            renderOffsetY + (gameState->cameraPos.tile.y + gameState->cameraPos.pY - currentTileChunkY) * metersToPixels,
          };

          // TODO(Cristian): Pass this draw call to a DrawTileRelativeToCenter call
          DrawHollowRectangle(offscreenBuffer, tileChunkMin, tileChunkMax, gray, gray, gray);
        }
      }
    }
  }

  /**
   * ENTITY DRAW
   */

  entity = gameState->entities;
  for(int32 entityIndex = 0;
      entityIndex < gameState->entityCount;
      entityIndex++, entity++)
  {
    if(!ValidEntity(entity)) { continue; }

    // TODO(Cristian): Do bitmap draw relative to the camera, in order
    // to be able to render multiple bitmaps
    DrawRectangle(offscreenBuffer,
        vector2D<real32>{renderOffsetX - (PLAYER_WIDTH / 2) * metersToPixels,
        renderOffsetY - PLAYER_HEIGHT * metersToPixels},
        vector2D<real32>{renderOffsetX + (PLAYER_WIDTH / 2) * metersToPixels,
        renderOffsetY},
        1.0f, 1.0f, 0.0f);

    hero_bitmap heroBitmap = gameState->heroBitmaps[entity->facingDirection];
    DrawBitmap(offscreenBuffer, heroBitmap.torso,
        renderOffsetX, renderOffsetY,
        heroBitmap.offsetX, heroBitmap.offsetY, true);
    DrawBitmap(offscreenBuffer, heroBitmap.cape,
        renderOffsetX, renderOffsetY,
        heroBitmap.offsetX, heroBitmap.offsetY, true);
    DrawBitmap(offscreenBuffer, heroBitmap.head,
        renderOffsetX, renderOffsetY,
        heroBitmap.offsetX, heroBitmap.offsetY, true);

  }

 // Draw Mouse
  DrawRectangle(offscreenBuffer,
               vector2D<real32>{(real32)gameInput->mouseX, (real32)gameInput->mouseY},
               vector2D<real32>{(real32)(gameInput->mouseX + 10), (real32)(gameInput->mouseY + 10)},
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
    // TODO(Cristián): This may be more appropiate to do in the platform layer
    gameMemory->soundInitialized = true;
  }

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

