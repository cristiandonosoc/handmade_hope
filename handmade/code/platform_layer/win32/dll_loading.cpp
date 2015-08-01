/*  =====================================================================
    $File: platform_layerwin32win32_dll_loading.cpp
    $Creation Date: 2015-06-15
    $Last Modified: $
    $Revision: $
    $Creator: Cristián Donoso $
    $Notice: (c) Copyright 2015 Cristián Donoso $
    ===================================================================== */

#ifndef _WIN32_DLL_LOADING_CPP

internal inline FILETIME
Win32GetLastWriteTime(char *filename)
{
  FILETIME lastWriteTime = {};

  WIN32_FILE_ATTRIBUTE_DATA data;
  if(GetFileAttributesEx(filename,
                         GetFileExInfoStandard,
                         &data))
  {
    lastWriteTime = data.ftLastWriteTime;
  }

  return lastWriteTime;
}

internal win32_game_code
Win32LoadGameCode(char *sourceDllPath, char* targetDllPath, char* lockFilename)
{
  win32_game_code gameCode = {};

  // We check if the lock file exists
  WIN32_FILE_ATTRIBUTE_DATA ignored;
  if(GetFileAttributesEx(lockFilename, GetFileExInfoStandard, &ignored))
  {
    return gameCode;
  }
  BOOL result = CopyFileA(sourceDllPath, targetDllPath, false);
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
