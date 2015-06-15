/*  =====================================================================
    $File: platform_layerwin32win32_dll_loading.cpp
    $Creation Date: 2015-06-15
    $Last Modified: $
    $Revision: $
    $Creator: Cristi�n Donoso $
    $Notice: (c) Copyright 2015 Cristi�n Donoso $
    ===================================================================== */

#ifndef _WIN32_DLL_LOADING_CPP

internal inline FILETIME
Win32GetLastWriteTime(char *filename)
{
  FILETIME lastWriteTime = {};

  WIN32_FIND_DATA findData;
  HANDLE findHandle = FindFirstFileA(filename, &findData);
  if(findHandle != INVALID_HANDLE_VALUE)
  {
    lastWriteTime = findData.ftLastWriteTime;
    FindClose(findHandle);
  }

  return lastWriteTime;
}

internal win32_game_code
Win32LoadGameCode(char *sourceDllPath, char* targetDllPath)
{
  win32_game_code gameCode = {};

  BOOL result = CopyFileA(sourceDllPath,
            targetDllPath,
            false);
  if(!result)
  {
    DWORD r = GetLastError();
    r++;
  }

  gameCode.gameCodeDll = LoadLibraryA(targetDllPath);
  if(gameCode.gameCodeDll)
  {
    gameCode.lastWriteTime = Win32GetLastWriteTime(sourceDllPath);

    // We load the exported functions
    gameCode.updateAndRenderFunction =
      (game_update_and_render *)GetProcAddress(gameCode.gameCodeDll,
                                               "GameUpdateAndRender");
    gameCode.getSoundFunction =
      (game_get_sound *)GetProcAddress(gameCode.gameCodeDll, "GameGetSound");

    gameCode.valid = (gameCode.updateAndRenderFunction &&
                      gameCode.getSoundFunction);
  }

  if(!gameCode.valid)
  {
    gameCode.updateAndRenderFunction = GameUpdateAndRenderStub;
    gameCode.getSoundFunction = GameGetSoundStub;
  }

  return gameCode;
}

internal void
Win32UnloadGameCode(win32_game_code *gameCode)
{
  if(gameCode->gameCodeDll)
  {
    FreeLibrary(gameCode->gameCodeDll);
  }
  gameCode->valid = false;
  gameCode->updateAndRenderFunction = GameUpdateAndRenderStub;
  gameCode->getSoundFunction = GameGetSoundStub;
}


#define _WIN32_DLL_LOADING_CPP
#endif
