/*  =====================================================================
    $File: win32_direct_sound_wrapper.cpp
    $Creation Date: 2015-01-09
    $Last Modified: $
    $Revision: $
    $Creator: Cristián Donoso $
    $Notice: (c) Copyright 2015 Cristián Donoso $
    =====================================================================

    Wrapper for Direct Sound support

    ===================================================================== */

#ifndef _WIN32_DIRECT_SOUND_CPP

#include "common_types.h"
#include "win32_direct_sound_wrapper.h"

#include <windows.h>
#include <dsound.h>
#include <stdio.h>

/******** DIRECT SOUND CONFIG *******/
global_variable LPDIRECTSOUNDBUFFER gSecondaryBuffer;
global_variable win32_sound_output gSoundOutput;

/**
 * We create our DirectSound API handler pointer.
 */
#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);

/**
 * We initialize the Direct Sound buffers.
 * The first (or primary) is merely a handle and initialization of the memory in the sound card.
 * The second is the actual buffer in memory the system will be writing into to create sound.
 */
internal void
Win32InitDirectSound(HWND windowHandle,
                     win32_sound_output *soundOutput)
{
  // NOTE(Cristián): Load the library
  //                 DLL Injection
  HMODULE DirectSoundLibrary = LoadLibraryA("dsound.dll");
  if(!DirectSoundLibrary) { return; } //TODO(Cristián): Diagnostics

  // Get a DirectSound object
  direct_sound_create *directSoundCreate = (direct_sound_create *)
    GetProcAddress(DirectSoundLibrary, "DirectSoundCreate");

  // TODO(Cristián): Check that this works on XP - DirectSound 8 or 7
  /**
   * What happens here is that Windows gives us a LPDIRECTSOUND object that is defined
   * in the dsound.h file. Afterwards we don't need to call GetProcAddress
   * to get the calls to the DirectSound object methods
   * because we make use of its vTable.
   *
   * Basically, this object has it's methods defined as virtual methods.
   * This means that it has a method to a global table associated
   * with the class (or struct) that has all the pointers
   * to its virtual methods.
   * This way we can get the pointers to the methods
   * by referencing the vTable.
   *
   * This means that at RUNTIME we can get the pointers
   * to the functions of the object without having to
   * actually have its definitions compiled.
   * This is the standard way Microsoft implements
   * its COM (Component Object Model) layer in order
   * to send functions outside the 'dll boundary'
   */
  LPDIRECTSOUND directSound;
  if(!directSoundCreate || !SUCCEEDED(directSoundCreate(0, &directSound, 0)))
  {
    return; // TODO(Cristián): Diagnostics
  }

  if(!SUCCEEDED(directSound->SetCooperativeLevel(windowHandle, DSSCL_PRIORITY)))
  {
    return; // TODO(Cristián): Diagnostics
  }

  // We set the format for the buffers
  WAVEFORMATEX waveFormat = {};
  waveFormat.wFormatTag = WAVE_FORMAT_PCM;
  waveFormat.nChannels = (WORD)soundOutput->nChannels;
  waveFormat.wBitsPerSample = (WORD)(soundOutput->bytesPerBlock / soundOutput->nChannels) * 8;
  // Size (in bytes) of a sample block
  waveFormat.nBlockAlign = (WORD)soundOutput->bytesPerBlock;
  waveFormat.nSamplesPerSec = soundOutput->samplesPerSecond;
  waveFormat.nAvgBytesPerSec = waveFormat.nBlockAlign * waveFormat.nSamplesPerSec;
  waveFormat.cbSize = 0;

  // "Create" a primary buffer
  // This buffer is a handle to the sound card which Windows will write according to
  // our data in the secondary buffer
  DSBUFFERDESC bufferDescription = {};
  bufferDescription.dwSize = sizeof(bufferDescription);
  // TODO(Cristián): See if we need DSBCAPS_GLOBALFOCUS
  bufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;

  LPDIRECTSOUNDBUFFER primaryBuffer;
  if(!SUCCEEDED(directSound->CreateSoundBuffer(&bufferDescription,
                                               &primaryBuffer,
                                               0)))
  {
    return; // TODO(Cristián): Diagnostics
  }
  // TODO(Cristián): Diagnostics
  if(!SUCCEEDED(primaryBuffer->SetFormat(&waveFormat))) { return; }

  // "Create" a secondary buffer
  DSBUFFERDESC secBufferDescription = {};
  secBufferDescription.dwSize = sizeof(secBufferDescription);
  secBufferDescription.dwFlags = 0;
  secBufferDescription.dwBufferBytes = soundOutput->bufferSize;
  secBufferDescription.lpwfxFormat = &waveFormat;
  // The gSecondaryBuffer pointer is defined globally
  if(!SUCCEEDED(directSound->CreateSoundBuffer(&secBufferDescription,
                                               &gSecondaryBuffer,
                                               0)))
  {
    return; // TODO(Cristián): Diagnostics
  }
}

/**
 * Fills the given soundOutput's soundBuffer with 0
 */
internal void
Win32ClearBuffer(win32_sound_output *soundOutput)
{
  VOID *region1;
  DWORD region1Size;
  VOID *region2;
  DWORD region2Size;
  if(!SUCCEEDED(gSecondaryBuffer->Lock(0, soundOutput->bufferSize,
                                       &region1, &region1Size,
                                       &region2, &region2Size,
                                       0)))
  {
    // If we can't lock the buffer, then the buffer died
    return;
  }

  uint8 *destSample = (uint8 *)region1;
  for(uint32 sampleIndex = 0;
      sampleIndex < region1Size;
      sampleIndex++)
  {
    *destSample++ = 0;
  }

  destSample = (uint8 *)region2;
  for(uint32 sampleIndex = 0;
      sampleIndex < region2Size;
      sampleIndex++)
  {
    *destSample++ = 0;
  }

  gSecondaryBuffer->Unlock(region1, region1Size,
                           region2, region2Size);
}

// Fills the direct sound buffer with the information provided
// by the game sound output
internal void
Win32FillSoundBuffer(win32_sound_output *soundOutput,
                     game_sound_output_buffer *sourceOutput)
{
  VOID *region1;
  DWORD region1Size;
  VOID *region2;
  DWORD region2Size;
  if(!SUCCEEDED(gSecondaryBuffer->Lock(soundOutput->byteToLock,
                                       soundOutput->bytesToWrite,
                                       &region1, &region1Size,
                                       &region2, &region2Size,
                                       0)))
  {
    // If we can't lock the buffer, then the buffer died
    return;
  }

  /**
   * We write into the buffer by writing and advancing the output pointer
   * We make two writes because we created 2 channels,
   * which makes the buffer to look like this:
   * [int16 int16] [int16 int16] ...
   * [LEFT  RIGHT] [LEFT  RIGHT] ...
   * [  SAMPLE   ] [  SAMPLE   ] ...
   *
   */

  // We cast the region pointer into int16 pointers (it is a DWORD) so we can
  // write into each channel of the sound buffer
  int16 *destSample = (int16 *)region1;
  int16 *sourceSample = (int16 *)sourceOutput->bufferMemory;
  int32 region1SampleCount = region1Size / soundOutput->bytesPerBlock;
  // TODO(Cristián): Assert that region sizes are valid (sample multiple)
  for(int32 sampleIndex = 0;
      sampleIndex < region1SampleCount;
      sampleIndex++)
  {
    // *destSample++ = *sourceSample++;
    // *destSample++ = *sourceSample++;
    soundOutput->runningBlockIndex++;
  }

  destSample = (int16 *)region2;
  int32 region2SampleCount = region2Size / soundOutput->bytesPerBlock;
  // TODO(Cristián): Assert that region sizes are valid (sample multiple)
  for(int32 sampleIndex = 0;
      sampleIndex < region2SampleCount;
      sampleIndex++)
  {
    // *destSample++ = *sourceSample++;
    // *destSample++ = *sourceSample++;
    soundOutput->runningBlockIndex++;
  }

  gSecondaryBuffer->Unlock(region1, region1Size,
                           region2, region2Size);
}

// Setups the next writeCursor position to be called in the next
// FillSoundBuffer call
//
// NOTE(Cristián): Here is how the sound output works
//
// We define a safety value (in blocks) that represents the amount
// by which we estimate our game loop can vary when querying the
// sound card (say,up to 2ms).
//
// Some defined time before the page flip, we query the playCursor
// and the writeCursor (by calling this method). We then can
// forecast where the writeCursor will be in the next query time.
// If that PWC (Projected Write Cursor) is before the next frame boundary,
// (before the second page flip to come), by more that our safety value,
// it means that we have a low latency sound card and
// we can perfectly synchronize audio.
//
// If the PWC is *after* the next frame boundary minus the safety value,
// then we assume that we can never perfectly sync audio because
// the sound card is inherently latent.
// This means that we merely write a frame's worth of audio
// plus a number of guard samples (should be correlated by the safety value).
internal bool32
Win32SetupSoundBuffer(win32_sound_output *soundOutput,
                      real32 secondsExpectedToFrameFlip,
                      bool32 firstRun)
{
  // We get the cursor IN BYTES of where the system is playing and writing in the
  // sound buffer
  // TODO(Cristián): Tighten up sound logic so that we know where we should be
  // writing to and to anticipate time spent in the game output.
  if(!SUCCEEDED(gSecondaryBuffer->GetCurrentPosition(&soundOutput->playCursor,
                                                     &soundOutput->writeCursor)))
  {
    // If we can't get the current position, then the buffer died
    return false;
  }

  if(firstRun)
  {
    soundOutput->runningBlockIndex = soundOutput->writeCursor /
                                     soundOutput->bytesPerBlock;
  }

  // We basically write from the last point we wrote until the playCursor
  DWORD byteToLock =
    (soundOutput->runningBlockIndex * soundOutput->bytesPerBlock) %
    soundOutput->bufferSize;

  DWORD safeWriteCursor = soundOutput->writeCursor;
  if (safeWriteCursor < soundOutput->playCursor)
  {
    safeWriteCursor += soundOutput->bufferSize;
  }
  safeWriteCursor += soundOutput->safetyBytes;

  ASSERT(safeWriteCursor >= soundOutput->playCursor);

  DWORD expectedFrameBoundaryByte = (soundOutput->playCursor +
                                     (secondsExpectedToFrameFlip *
                                      soundOutput->samplesPerSecond *
                                      soundOutput->bytesPerBlock));

  bool32 NonLatentAudioCard =
    (safeWriteCursor < expectedFrameBoundaryByte);

  DWORD targetCursor;
  if(NonLatentAudioCard)
  {
    targetCursor = expectedFrameBoundaryByte +
                   soundOutput->expectedSoundBytesPerFrame;
  }
  else
  {
    targetCursor = soundOutput->writeCursor +
                   soundOutput->expectedSoundBytesPerFrame +
                   soundOutput->safetyBytes;
  }
  targetCursor = targetCursor % soundOutput->bufferSize;

  DWORD bytesToWrite;
  if(byteToLock <= targetCursor)
  {
    bytesToWrite = targetCursor - byteToLock;
  }
  else
  {
    bytesToWrite = soundOutput->bufferSize - (byteToLock - targetCursor);
  }

  //Win32FillSoundBuffer(soundOutput, byteToLock, bytesToWrite, sourceOutput);

  soundOutput->byteToLock = byteToLock;
  soundOutput->bytesToWrite = bytesToWrite;

  DWORD unwrappedWriteCursor = soundOutput->writeCursor;
  if (soundOutput->writeCursor < soundOutput->playCursor)
  {
    unwrappedWriteCursor += soundOutput->bufferSize;
  }
  DWORD minLatency = unwrappedWriteCursor - soundOutput->playCursor;
  soundOutput->latencySeconds = ((real32)minLatency /
                                 (real32)soundOutput->bytesPerBlock) /
                                (real32)soundOutput->samplesPerSecond;

#if 0
  char buffer[256];
  sprintf_s(buffer,
          "PC: %u, WC: %u, Latency (bytes): %u, Latency (s): %f\n",
          soundOutput->playCursor,
          soundOutput->writeCursor,
          minLatency,
          soundOutput->latencySeconds);
  OutputDebugStringA(buffer);
#endif

  return true;
}

internal void
Win32PlayDirectSound()
{
  gSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);
}

#define _WIN32_DIRECT_SOUND_CPP
#endif
