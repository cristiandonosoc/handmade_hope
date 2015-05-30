/*  =====================================================================
    $File: handmade.h
    $Creation Date: 2015-01-19
    $Last Modified: $
    $Revision: $
    $Creator: Cristi�n Donoso $
    $Notice: (c) Copyright 2015 Cristi�n Donoso $
    ===================================================================== */

/**
 * NOTE(Cristi�n):
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

#if HANDMADE_SLOW
#define ASSERT(expression) if(!(expression)) { *(int *)0 = 0; }
#else
#define ASSERT(expression) if((expression)) { *(int *)0 = 0; }
#endif

#define ARRAY_COUNT(Array) (sizeof(Array) / sizeof((Array)[0]))

// TODO(Cristi�n): Should this always use uint64?
#define KILOBYTES(amount) ((amount) * 1024LL)
#define MEGABYTES(amount) (KILOBYTES(amount) * 1024LL)
#define GIGABYTES(amount) (MEGABYTES(amount) * 1024LL)
#define TERABYTES(amount) (GIGABYTES(amount) * 1024LL)

inline uint32
SafeTruncateUInt64(uint64 value)
{
  // TODO(Cristi�n): DEFINES for maximum values
  ASSERT(value <= 0xFFFFFFFF);
  return (uint32)value;
}

/**
 * TODO(Cristi�n): Services that the platform layer provides to the game
 */

#if HANDMADE_INTERNAL
/**
 * IMPORTANT(Cristi�n):
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

internal game_file DEBUG_PlatformReadEntireFile(char *fileName);
internal void DEBUG_PlatformFreeGameFile(game_file *gameFile);
internal bool32 DEBUG_PlatformWriteEntireFile(char *fileName,
                                     uint32 memorySize,
                                     void *fileMemory);
#endif

/**
 * TODO(Cristi�n): Services that the game provides to the platform layer
 * (this may expand in the future (sound on a separate thread))
 */
// TODO(Cristi�n): In the future, rendering *specifically* will become a
// three-tier abstraction
struct game_offscreen_buffer
{
  void *memory;
  int width;
  int height;
  int pitch;
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

      // NOTE(Cristi�n): All buttons MUST be added
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
  game_controller_input controllers[5];
};
inline
game_controller_input *GetController(game_input *input, int controllerIndex)
{
  ASSERT(controllerIndex < ARRAY_COUNT(input->controllers));
  return(&input->controllers[controllerIndex]);
}

struct game_memory
{
  bool32 graphicsInitialized;
  bool32 soundInitialized;
  uint64 permanentStorageSize;
  // NOTE(Cristi�n): REQUIRED to be cleared to 0 at startup
  void *permanentStorage;

  uint64 transientStorageSize;
  // NOTE(Cristi�n): REQUIRED to be cleared to 0 at startup
  void *transientStorage;
};


internal void
GameUpdateAndRender(game_offscreen_buffer *offscreenBuffer,
                    game_memory *gameMemory,
                    game_input *gameInput);

// NOTE(Cristi�n): At the moment, this has to be a very fast
// function (1ms or so)
// TODO(Cristi�n): Reduce the pressure on this function by
// measuring or querying.
internal void
GameGetSound(game_sound_output_buffer *soundBuffer,
             game_memory *gameMemory,
             game_input *gameInput);

// THIS IS NOT FOR THE PLATFORM LAYER TO KNOW

struct game_state
{
  int32 xOffset;
  int32 yOffset;
  int32 toneHz;
  int32 toneVolume;
};

struct game_clocks
{
  // TODO(Cristi�n): What do we want to pass?
  real32 secondsElapsed;
};


#define _HANDMADE_H_INCLUDED
#endif
