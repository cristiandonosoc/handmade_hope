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

#include "common_types.h"

#include <windows.h>
#include <dsound.h>
#include <stdio.h>

/******** DIRECT SOUND CONFIG *******/
global_variable LPDIRECTSOUNDBUFFER gSecondaryBuffer;

struct win32_sound_output
{
  private:
  int32 toneHz = 440;
  int32 samplesPerSecond = 48000;
  int32 wavePeriod = 48000 / 440;

  public:
  // Variables
  int16 toneVolume = 16000;

  // Buffer Definition
  int32 nChannels;
  int32 bytesPerBlock;
  int32 bufferSize;
  int32 latency;

  // Buffer
  void *bufferMemory;

  // Get & Set Methods
  int32 GetWavePeriod() { return this->wavePeriod; }
  int32 GetSamplesPerSecond() { return this->samplesPerSecond; }
  int32 GetToneHz() { return this->toneHz; }
  void SetSamplesPerSecond(int32 samplesPerSecond)
  {
    this->samplesPerSecond = samplesPerSecond;
    this->wavePeriod = this->samplesPerSecond / this->toneHz;
  }

  void SetBufferToneHz(int32 toneHz)
  {
    this->toneHz = toneHz;
    this->wavePeriod = this->samplesPerSecond / this->toneHz;
  }
  void ModifyBufferToneHz(int32 diff)
  {
    this->SetBufferToneHz(this->toneHz + diff);
  }

  // VARIABLE DATA
  real32 tSine;
  int32 runningBlockIndex;
  uint32 byteToLock;
  uint32 bytesToWrite;
};

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

  // We set the format for the buffers
  WAVEFORMATEX waveFormat = {};
  waveFormat.wFormatTag = WAVE_FORMAT_PCM;
  waveFormat.nChannels = soundOutput->nChannels;
  waveFormat.wBitsPerSample = (soundOutput->bytesPerBlock / soundOutput->nChannels) * 8;
  // Size (in bytes) of a sample block
  waveFormat.nBlockAlign = soundOutput->bytesPerBlock;
  waveFormat.nSamplesPerSec = soundOutput->GetSamplesPerSecond();
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
  secBufferDescription.dwBufferBytes = soundOutput->bufferSize;
  secBufferDescription.lpwfxFormat = &waveFormat;
  // The gSecondaryBuffer pointer is defined globally
  if(!SUCCEEDED(directSound->CreateSoundBuffer(&secBufferDescription, &gSecondaryBuffer, 0)))
  {
    return; // TODO(Cristi�n): Diagnostics
  }

  // NOTE(Cristi�n): Start it playing
}

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
  for(int32 sampleIndex = 0;
      sampleIndex < region1Size;
      sampleIndex++)
  {
    *destSample++ = 0;
  }

  destSample = (uint8 *)region2;
  for(int32 sampleIndex = 0;
      sampleIndex < region2Size;
      sampleIndex++)
  {
    *destSample++ = 0;
  }

  gSecondaryBuffer->Unlock(region1, region1Size, region2, region2Size);
}

internal void
Win32FillSoundBuffer(win32_sound_output *soundOutput,
                     game_sound_ouput_buffer *sourceOutput)
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
   * We make two writes because we created 2 channels, which makes the buffer to look like this:
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
  // TODO(Cristi�n): Assert that region sizes are valid (sample multiple)
  for(int32 sampleIndex = 0;
      sampleIndex < region1SampleCount;
      sampleIndex++)
  {
    *destSample++ = *sourceSample++;
    *destSample++ = *sourceSample++;
    soundOutput->runningBlockIndex++;
  }

  destSample = (int16 *)region2;
  int32 region2SampleCount = region2Size / soundOutput->bytesPerBlock;
  // TODO(Cristi�n): Assert that region sizes are valid (sample multiple)
  for(int32 sampleIndex = 0;
      sampleIndex < region2SampleCount;
      sampleIndex++)
  {
    *destSample++ = *sourceSample++;
    *destSample++ = *sourceSample++;
    soundOutput->runningBlockIndex++;
  }

  gSecondaryBuffer->Unlock(region1, region1Size, region2, region2Size);
}

internal bool32
Win32SetupSoundBuffer(win32_sound_output *soundOutput)
{
  // We get the cursor IN BYTES of where the system is playing and writing in the
  // sound buffer
  DWORD playCursor;
  DWORD writeCursor;
  // TODO(Cristi�n): Tighten up sound logic so that we know where we should be
  // writing to and to anticipate time spent in the game output.
  if(!SUCCEEDED(gSecondaryBuffer->GetCurrentPosition(&playCursor, &writeCursor)))
  {
    // If we can't get the current position, then the buffer died
    return false;
  }

  // We basically write from the last point we wrote until the playCursor
  DWORD bytesToWrite;
  DWORD byteToLock =
    (soundOutput->runningBlockIndex * soundOutput->bytesPerBlock) % soundOutput->bufferSize;

  DWORD targetCursor = (playCursor +
    (soundOutput->latency * soundOutput->bytesPerBlock)) %
    soundOutput->bufferSize;

  // TODO(Cristi�n): Change to a lower latenxy offset from the playCursor
  //                 Right now we have 1 buffer latency
  if(byteToLock <= targetCursor) { bytesToWrite = targetCursor - byteToLock; }
  else { bytesToWrite = soundOutput->bufferSize - (byteToLock - targetCursor); }

  //Win32FillSoundBuffer(soundOutput, byteToLock, bytesToWrite, sourceOutput);

  soundOutput->byteToLock = byteToLock;
  soundOutput->bytesToWrite = bytesToWrite;

  return true;
}

internal void
Win32PlayDirectSound()
{
  gSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);
}

#define _WIN32_DIRECT_SOUND_WRAPPER_INCLUDED
#endif
