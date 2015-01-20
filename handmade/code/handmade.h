/*  =====================================================================
    $File: handmade.h
    $Creation Date: 2015-01-19
    $Last Modified: $
    $Revision: $
    $Creator: Cristi�n Donoso $
    $Notice: (c) Copyright 2015 Cristi�n Donoso $
    ===================================================================== */

#if !defined(HANDMADE_H)

  /**
   * TODO(Cristi�n): Services that the platform layer provides to the game
   */

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


  internal void GameUpdateAndRender(game_offscreen_buffer *buffer);

#define HANDMADE_H
#endif
