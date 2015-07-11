/*  =====================================================================
    $File: handmade.h
    $Creation Date: 2015-01-19
    $Last Modified: $
    $Revision: $
    $Creator: Cristián Donoso $
    $Notice: (c) Copyright 2015 Cristián Donoso $
    ===================================================================== */

/**
 * NOTE(Cristián):
 *
 * HANDMADE_INTERNAL:
 *  0 - Build for public release
 *  1 - Build for developer only
 *
 * HANDMADE_SLOW:
 *  0 - No slow code allowed
 *  1 - Slow code allowed
 */
#ifndef _HANDMADE_H_INCLUDED

#include "common_types.h"

#if HANDMADE_SLOW
#define ASSERT(expression) if(!(expression)) { *(int *)0 = 0; }
#else
#define ASSERT(expression) if((expression)) { *(int *)0 = 0; }
#endif

#define ARRAY_COUNT(Array) (sizeof(Array) / sizeof((Array)[0]))

// TODO(Cristián): Should this always use uint64?
#define KILOBYTES(amount) ((amount) * 1024LL)
#define MEGABYTES(amount) (KILOBYTES(amount) * 1024LL)
#define GIGABYTES(amount) (MEGABYTES(amount) * 1024LL)
#define TERABYTES(amount) (GIGABYTES(amount) * 1024LL)

inline uint32
SafeTruncateUInt64(uint64 value)
{
  // TODO(Cristián): DEFINES for maximum values
  ASSERT(value <= 0xFFFFFFFF);
  return (uint32)value;
}

// TODO(Cristián): In the future, rendering *specifically* will become a
// three-tier abstraction
struct game_offscreen_buffer
{
  void *memory;
  int width;
  int height;
  int pitch;
  int bytesPerPixel;
};

struct game_sound_output_buffer
{
  void *bufferMemory;
  int32 sampleCount;

  int32 toneVolume;
  int32 samplesPerSecond;
  int32 toneHz;
  bool32 valid;
};

struct game_button_state
{
  int halfTransitionCount;
  bool32 endedDown;
};

struct game_controller_input
{
  bool32 isConnected;
  bool32 isAnalog;

  real32 leftStickAverageX;
  real32 leftStickAverageY;
  real32 rightStickAverageX;
  real32 rightStickAverageY;

  /**
   * union allows for several ways to access the same memory.
   * In this case, the game_controller_input is always the same in memory,
   * but I can access it through the buttons array or directly into the struct.
   * As both are pointing to the *same* memory, any changes from one will be
   * reflected on the other.
   */
  union
  {
    game_button_state buttons[12];
    struct
    {
      game_button_state moveUp;
      game_button_state moveDown;
      game_button_state moveLeft;
      game_button_state moveRight;

      game_button_state actionUp;
      game_button_state actionDown;
      game_button_state actionLeft;
      game_button_state actionRight;

      game_button_state leftShoulder;
      game_button_state rightShoulder;

      game_button_state start;
      game_button_state back;

      // NOTE(Cristián): All buttons MUST be added
      //                 before this line
      game_button_state terminator;
    };
  };
};


/**
 * We have 5 controllers:
 * 0 = Keyboard controller
 * 1-4 = Gamepads
 */
struct game_input
{
  // Controller information
  game_button_state mouseButtons[5];
  uint32 mouseX;
  uint32 mouseY;
  uint32 mouseZ;

  game_controller_input controllers[5];

  // Time information
  real32 secondsToUpdate;
};

inline game_controller_input*
GetController(game_input *input, int controllerIndex)
{
  ASSERT(controllerIndex < ARRAY_COUNT(input->controllers));
  return(&input->controllers[controllerIndex]);
}

// THIS IS NOT FOR THE PLATFORM LAYER TO KNOW
struct game_state
{
  int32 xOffset;
  int32 yOffset;
  int32 toneHz;
  int32 toneVolume;
  real32 tSine;

  int playerX;
  int playerY;
};

struct game_clocks
{
  // TODO(Cristián): What do we want to pass?
  real32 secondsElapsed;
};

struct thread_context
{
  int placeholder;
};

/**
 * TODO(Cristián): Services that the game provides to the platform layer
 * (this may expand in the future (sound on a separate thread))
 */
#if HANDMADE_INTERNAL
/**
 * IMPORTANT(Cristián):
 *
 * These are not for doing anything in the shipping game.
 * Thay are blocking and the write doesn't protect against lost data.
 */
struct game_file
{
  bool32 valid;
  uint32 contentSize;
  void *content;
};

// File platform support
#define DEBUG_PLATFORM_READ_ENTIRE_FILE(name)\
  game_file name(thread_context *context, char *fileName)
typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(debug_platform_read_entire_file);
#define DEBUG_PLATFORM_FREE_GAME_FILE(name)\
  void name(thread_context *context, game_file *gameFile)
typedef DEBUG_PLATFORM_FREE_GAME_FILE(debug_platform_free_game_file);
#define DEBUG_PLATFORM_WRITE_ENTIRE_FILE(name)\
  bool32 name(thread_context *context,\
              char *fileName,\
              uint32 memorySize,\
              void *fileMemory)
typedef DEBUG_PLATFORM_WRITE_ENTIRE_FILE(debug_platform_write_entire_file);
#endif

struct game_memory
{
  bool32 graphicsInitialized;
  bool32 soundInitialized;
  uint64 permanentStorageSize;
  // NOTE(Cristián): REQUIRED to be cleared to 0 at startup
  void *permanentStorage;

  uint64 transientStorageSize;
  // NOTE(Cristián): REQUIRED to be cleared to 0 at startup
  void *transientStorage;

  // Functions Pointers to be filled by the platform layer
  debug_platform_read_entire_file *DEBUGPlatformReadEntireFileFunction;
  debug_platform_free_game_file *DEBUGPlatformFreeGameFileFunction;
  debug_platform_write_entire_file *DEBUGPlatformWriteEntireFileFunction;
};

/**
 * GAME FUNCTIONS
 */

#define GAME_UPDATE_AND_RENDER(name)\
  void name(thread_context *threadContext,\
            game_offscreen_buffer *offscreenBuffer,\
            game_memory *gameMemory,\
            game_input *gameInput)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render);
GAME_UPDATE_AND_RENDER(GameUpdateAndRenderStub)
{
  /* STUB */
}

// NOTE(Cristián): At the moment, this has to be a very fast
// function (1ms or so)
// TODO(Cristián): Reduce the pressure on this function by
// measuring or querying.
#define GAME_GET_SOUND(name)\
  void name(thread_context *threadContext,\
            game_sound_output_buffer *soundBuffer,\
            game_memory *gameMemory,\
            game_input *gameInput)
typedef GAME_GET_SOUND(game_get_sound);
GAME_GET_SOUND(GameGetSoundStub)
{
  /* STUB */
}


#define _HANDMADE_H_INCLUDED
#endif
