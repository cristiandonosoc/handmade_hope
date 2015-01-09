/*  =====================================================================
    $File: win32_direct_sound_wrapper.cpp
    $Creation Date: 2015-01-09
    $Last Modified: $
    $Revision: $
    $Creator: Cristi�n Donoso $
    $Notice: (c) Copyright 2015 Cristi�n Donoso $
    =====================================================================

    Wrapper for Direct Sound support

    ===================================================================== */

#ifndef _WIN32_DIRECT_SOUND_WRAPPER_INCLUDED
#define _WIN32_DIRECT_SOUND_WRAPPER_INCLUDED

#include <windows.h>
#include <dsound.h>
#include "common_types.h"

/******** GLOBAL VARIABLES **********/
// TODO(Cristi�n): Remove this as a global variable
global_variable int32 gToneHz = 440;


/******** DIRECT SOUND CONFIG *******/
global_variable LPDIRECTSOUNDBUFFER gSecondaryBuffer;
global_variable uint32 gRunningBlockIndex;
global_variable int32 gBytesPerBlock;
global_variable int32 gBufferSize;
global_variable int32 gSamplesPerSecond;

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
                     int32 samplesPerSecond,
                     int32 bytesPerSample,
                     int32 nChannels,
                     int32 bufferLength)
{
  // NOTE(Cristi�n): Load the library
  HMODULE DirectSoundLibrary = LoadLibraryA("dsound.dll");
  if(!DirectSoundLibrary) { return; } //TODO(Cristi�n): Diagnostics

  // Get a DirectSound object
  direct_sound_create *directSoundCreate = (direct_sound_create *)
    GetProcAddress(DirectSoundLibrary, "DirectSoundCreate");

  // TODO(Cristi�n): Check that this works on XP - DirectSound 8 or 7
  /**
   * What happens here is that Windows gives us a LPDIRECTSOUND object that is defined
   * in the dsound.h file. Afterwards we don't need to call GetProcAddress to get the calls
   * to the DirectSound object methods because we make use of its vTable.
   *
   * Basically, this object has it's methods defined as virtual methods. This means that
   * it has a method to a global table associated with the class (or struct) that has all the
   * pointers to its virtual methods. This way we can get the pointers to the methods by referencing
   * the vTable.
   *
   * This means that at RUNTIME we can get the pointers to the functions of the object without
   * having to actually have its definitions compiled. This is the standard way Microsoft implements
   * its COM (Component Object Model) layer in order to send functions outside the 'dll boundary'
   */
  LPDIRECTSOUND directSound;
  if(!directSoundCreate || !SUCCEEDED(directSoundCreate(0, &directSound, 0)))
  {
    return; // TODO(Cristi�n): Diagnostics
  }

  if(!SUCCEEDED(directSound->SetCooperativeLevel(windowHandle, DSSCL_PRIORITY)))
  {
    return; // TODO(Cristi�n): Diagnostics
  }

  // We set the global variables
  gSamplesPerSecond = samplesPerSecond;
  gBytesPerBlock = nChannels * bytesPerSample;
  gBufferSize = bufferLength * nChannels * samplesPerSecond * bytesPerSample;

  // We set the format for the buffers
  WAVEFORMATEX waveFormat = {};
  waveFormat.wFormatTag = WAVE_FORMAT_PCM;
  waveFormat.nChannels = nChannels;
  waveFormat.wBitsPerSample = bytesPerSample << 3; // *8
  // Size (in bytes) of a sample block
  waveFormat.nBlockAlign = gBytesPerBlock;
  waveFormat.nSamplesPerSec = samplesPerSecond;
  waveFormat.nAvgBytesPerSec = waveFormat.nBlockAlign * waveFormat.nSamplesPerSec;
  waveFormat.cbSize = 0;

  // "Create" a primary buffer
  // This buffer is a handle to the sound card which Windows will write according to
  // our data in the secondary buffer
  DSBUFFERDESC bufferDescription = {};
  bufferDescription.dwSize = sizeof(bufferDescription);
  // TODO(Cristi�n): See if we need DSBCAPS_GLOBALFOCUS
  bufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;

  LPDIRECTSOUNDBUFFER primaryBuffer;
  if(!SUCCEEDED(directSound->CreateSoundBuffer(&bufferDescription, &primaryBuffer, 0)))
  {
    return; // TODO(Cristi�n): Diagnostics
  }
  if(!SUCCEEDED(primaryBuffer->SetFormat(&waveFormat))) { return; } // TODO(Cristi�n): Diagnostics

  // "Create" a secondary buffer
  DSBUFFERDESC secBufferDescription = {};
  secBufferDescription.dwSize = sizeof(secBufferDescription);
  secBufferDescription.dwFlags = 0;
  secBufferDescription.dwBufferBytes = gBufferSize;
  secBufferDescription.lpwfxFormat = &waveFormat;
  // The gSecondaryBuffer pointer is defined globally
  if(!SUCCEEDED(directSound->CreateSoundBuffer(&secBufferDescription, &gSecondaryBuffer, 0)))
  {
    return; // TODO(Cristi�n): Diagnostics
  }

  // NOTE(Cristi�n): Start it playing
}

internal void
Win32RunDirectSoundSample(int32 toneHz, uint32 toneVolume)
{
  // We get the cursor IN BYTES of where the system is playing and writing in the
  // sound buffer
  DWORD playCursor;
  DWORD writeCursor;
  if(!SUCCEEDED(gSecondaryBuffer->GetCurrentPosition(&playCursor, &writeCursor)))
  {
    // If we can't get the current position, then the buffer died
    return;
  }

  DWORD bytesToWrite;
  DWORD byteToLock = (gRunningBlockIndex * gBytesPerBlock) % gBufferSize;
  if(byteToLock <= playCursor) { bytesToWrite = playCursor - byteToLock; }
  else { bytesToWrite = gBufferSize - (byteToLock - playCursor); }

  VOID *region1;
  DWORD region1Size;
  VOID *region2;
  DWORD region2Size;
  if(!SUCCEEDED(gSecondaryBuffer->Lock(byteToLock, bytesToWrite,
          &region1, &region1Size,
          &region2, &region2Size,
          0)))
  {
    // If we can't lock the buffer, then the buffer died
    return;
  }

  /**
   * We write into the buffer by writing and advancing the output pointer
   * We make two writes because we created 2 channels, which makes the buffer to look like this:
   * [int16 int16] [int16 int16] ...
   * [LEFT  RIGHT] [LEFT  RIGHT] ...
   * [  SAMPLE   ] [  SAMPLE   ] ...
   *
   */

  // TODO(Cristi�n): Remove this from the loop
  int32 squareWavePeriod = gSamplesPerSecond / gToneHz;
  int32 halfSquareWavePeriod = squareWavePeriod >> 1;

  // We cast the region pointer into int16 pointers (it is a DWORD) so we can
  // write into each channel of the sound buffer
  int16 *sampleOut = (int16 *)region1;
  int32 region1SampleCount = region1Size / gBytesPerBlock;
  // TODO(Cristi�n): Assert that region sizes are valid (sample multiple)
  for(int32 sampleIndex = 0;
      sampleIndex < region1SampleCount;
      sampleIndex++)
  {
    // We check into which part of the square Cycle we are
    int16 sampleValue = ((gRunningBlockIndex / halfSquareWavePeriod) % 2) ? toneVolume : -toneVolume;

    *sampleOut++ = sampleValue;
    *sampleOut++ = sampleValue;
    gRunningBlockIndex++;
  }
  sampleOut = (int16 *)region2;
  int32 region2SampleCount = region2Size / gBytesPerBlock;
  // TODO(Cristi�n): Assert that region sizes are valid (sample multiple)
  for(int32 sampleIndex = 0;
      sampleIndex < region2SampleCount;
      sampleIndex++)
  {
    // We check into which part of the square Cycle we are
    int16 sampleValue = ((gRunningBlockIndex / halfSquareWavePeriod) % 2) ? toneVolume : -toneVolume;

    *sampleOut++ = sampleValue;
    *sampleOut++ = sampleValue;
    gRunningBlockIndex++;
  }

  gSecondaryBuffer->Unlock(region1, region1Size, region2, region2Size);
}

internal void
Win32PlayDirectSound()
{
  gSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);
}

#endif
