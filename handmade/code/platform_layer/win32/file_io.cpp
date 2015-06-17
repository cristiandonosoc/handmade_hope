/*  =====================================================================
    $File: platform_layerwin32win32_file_io.cpp
    $Creation Date: 2015-02-11
    $Last Modified: $
    $Revision: $
    $Creator: Cristián Donoso $
    $Notice: (c) Copyright 2015 Cristián Donoso $
    ===================================================================== */

#ifndef _WIN32_FILE_IO_CPP

#include "file_io.h"

// Platform Load File
DEBUG_PLATFORM_READ_ENTIRE_FILE(DEBUGPlatformReadEntireFile)
{
  game_file gameFile = {};
  HANDLE fileHandle = CreateFileA(fileName,
                                  GENERIC_READ,
                                  FILE_SHARE_READ,
                                  0,
                                  OPEN_EXISTING,
                                  0,
                                  0);
  if (fileHandle == INVALID_HANDLE_VALUE)
  {
    // TODO(Cristián): Better error code
    // TODO(Cristián): Loggin'
    goto LABEL_PLATFORM_READ_ENTIRE_FILE_RETURN;
  }

  LARGE_INTEGER fileSize64;
  if(!GetFileSizeEx(fileHandle, &fileSize64))
  {
    // TODO(Cristián): Loggin'
    goto LABEL_PLATFORM_READ_ENTIRE_FILE_CREATE_FILE_CLEANUP;
  }

  uint32 fileSize32 = SafeTruncateUInt64(fileSize64.QuadPart);
  gameFile.content = VirtualAlloc(0,
                                  fileSize32,
                                  MEM_RESERVE|MEM_COMMIT,
                                  PAGE_READWRITE);
  if (!gameFile.content)
  {
    // TODO(Cristián): Loggin'
    goto LABEL_PLATFORM_READ_ENTIRE_FILE_CREATE_FILE_CLEANUP;
  }


  DWORD bytesRead = 0;
  if(!ReadFile(fileHandle,
               gameFile.content,
               fileSize32,
               &bytesRead,
               0))
  {
    // TODO(Cristián): Loggin'
    goto LABEL_PLATFORM_READ_ENTIRE_FILE_GET_FILE_MEMORY_CLEANUP;
  }

  if (bytesRead != fileSize32)
  {
    // TODO(Cristián): Loggin'
    goto LABEL_PLATFORM_READ_ENTIRE_FILE_GET_FILE_MEMORY_CLEANUP;
  }

  // NOTE(Cristián): FileRead was successful.
  //                 Cleanup code is below the return
  gameFile.contentSize = fileSize32;
  gameFile.valid = true;
  goto LABEL_PLATFORM_READ_ENTIRE_FILE_CREATE_FILE_CLEANUP;

LABEL_PLATFORM_READ_ENTIRE_FILE_GET_FILE_MEMORY_CLEANUP:
  DEBUGPlatformFreeGameFile(&gameFile);
LABEL_PLATFORM_READ_ENTIRE_FILE_CREATE_FILE_CLEANUP:
  CloseHandle(fileHandle);
LABEL_PLATFORM_READ_ENTIRE_FILE_RETURN:
  return(gameFile);
}

DEBUG_PLATFORM_FREE_GAME_FILE(DEBUGPlatformFreeGameFile)
{
  if(gameFile->content)
  {
    VirtualFree(gameFile->content, 0, MEM_RELEASE);
  }
  *gameFile = {};
}

DEBUG_PLATFORM_WRITE_ENTIRE_FILE(DEBUGPlatformWriteEntireFile)
{
  bool32 successfulWrite = false;
  HANDLE fileHandle = CreateFileA(fileName,
      GENERIC_WRITE,
      0,
      0,
      CREATE_ALWAYS,
      0,
      0);
  if (fileHandle == INVALID_HANDLE_VALUE)
  {
    // TODO(Cristián): Better error code
    // TODO(Cristián): Loggin'
    goto LABEL_PLATFORM_WRITE_ENTIRE_FILE_RETURN;
  }

  DWORD bytesWritten;
  if(!WriteFile(fileHandle,
                fileMemory,
                memorySize,
                &bytesWritten,
                0))
  {
    // TODO(Cristián): Loggin'
    goto LABEL_PLATFORM_WRITE_ENTIRE_FILE_CREATE_FILE_CLEANUP;
  }

  if (bytesWritten != memorySize)
  {
    // TODO(Cristián): Loggin'
    goto LABEL_PLATFORM_WRITE_ENTIRE_FILE_CREATE_FILE_CLEANUP;
  }

  // NOTE(Cristián): WriteFile was successful.
  //                 Cleanup code is below the return
  successfulWrite = true;

LABEL_PLATFORM_WRITE_ENTIRE_FILE_CREATE_FILE_CLEANUP:
  CloseHandle(fileHandle);
LABEL_PLATFORM_WRITE_ENTIRE_FILE_RETURN:
  return(successfulWrite);
}

#define _WIN32_FILE_IO_CPP
#endif
