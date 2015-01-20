/*  =====================================================================
    $File: handmade.h
    $Creation Date: 2015-01-19
    $Last Modified: $
    $Revision: $
    $Creator: Cristián Donoso $
    $Notice: (c) Copyright 2015 Cristián Donoso $
    ===================================================================== */

#if !defined(HANDMADE_H)

  /**
   * TODO(Cristián): Services that the platform layer provides to the game
   */

  /**
   * TODO(Cristián): Services that the game provides to the platform layer
   * (this may expand in the future (sound on a separate thread))
   */
  struct game_offscreen_buffer
  {
    BITMAPINFO info;
    void *memory;
    int width;
    int height;
    int pitch;
  };


  internal void GameUpdateAndRender(game_offscreen_buffer *buffer);

#define HANDMADE_H
#endif
