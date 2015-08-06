#ifndef _GAME_SOUND_CPP

internal void
OutputGameSound(game_sound_output_buffer *soundOutput,
                game_state *gameState)
{
  if(!soundOutput->valid) { return; }

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
  int16 *sampleOut = (int16 *)soundOutput->bufferMemory;
  //int32 region1SampleCount = region1Size / soundOutput->bytesPerBlock;
  // TODO(Cristián): Assert that region sizes are valid (sample multiple)
  for(int32 sampleIndex = 0;
      sampleIndex < soundOutput->sampleCount;
      sampleIndex++)
  {
    int16 sampleValue = 0; // Silence for now...

    *sampleOut++ = sampleValue;
    *sampleOut++ = sampleValue;
  }
}



#define _GAME_SOUND_CPP
#endif
