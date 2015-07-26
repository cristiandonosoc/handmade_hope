#ifndef _GAME_RENDER_CPP

internal void
ClearScreenBuffer(game_offscreen_buffer *buffer, real32 R,
                                                 real32 G,
                                                 real32 B)
{
  // TODO(Cristián): Let's see what the optimizer does if it is passed by reference
  // instead of a pointer.
  int bitmapWidth = buffer->width;
  int bitmapHeight = buffer->height;
  int pitch = buffer->pitch;
  void *memory = buffer->memory;

  uint32 color = UTILS::RealRGBToUInt32(R, G, B);

  uint8* row = (uint8 *)memory;
  for(int y = 0;
      y < bitmapHeight;
      y++)
  {
    uint32 *pixel = (uint32 *)row;
    for(int x = 0;
        x < bitmapWidth;
        x++)
    {
      /*
        Pixel in Memory: BB GG RR xx
        Little Endian Architecture
        Pixel in Register: 0xXXRRGGBB
       */

      *pixel++ = color;
    }
    row += pitch;
  }

}

/**
 * Writes a 'weird' gradient into a memory buffer
 * @param *buffer A pointer to the buffer info struct. We can pass it by reference also
 *                because the struct does not contain the buffer itself, but a pointer, so
 *                this method does not ACTUALLY modify the struct, but the memory it points to.
 *                We pass a pointer in order to not copy a fairly big structure into memory via
 *                the stack.
 */
internal void
RenderWeirdGradient(game_offscreen_buffer *buffer, int blueOffset, int greenOffset)
{
  // TODO(Cristián): Let's see what the optimizer does if it is passed by reference
  // instead of a pointer.
  int bitmapWidth = buffer->width;
  int bitmapHeight = buffer->height;
  int pitch = buffer->pitch;
  void *memory = buffer->memory;

  uint8* row = (uint8 *)memory;
  for(int y = 0;
      y < bitmapHeight;
      y++)
  {
    uint32 *pixel = (uint32 *)row;
    for(int x = 0;
        x < bitmapWidth;
        x++)
    {
      /*
        Pixel in Memory: BB GG RR xx
        Little Endian Architecture
        Pixel in Register: 0xXXRRGGBB
       */
      uint8 blue = (uint8)(x + blueOffset);
      uint8 green = (uint8)(y + greenOffset);

      *pixel++ = blue | (green << 16);
    }
    row += pitch;
  }
}

internal void
DrawRectangle(game_offscreen_buffer* buffer, real32 realMinX, real32 realMinY,
                                             real32 realMaxX, real32 realMaxY,
                                             real32 R, real32 G, real32 B)
{
  int32 minX = UTILS::RoundReal32ToUInt32(realMinX);
  int32 maxX = UTILS::RoundReal32ToUInt32(realMaxX);
  int32 minY = UTILS::RoundReal32ToUInt32(realMinY);
  int32 maxY = UTILS::RoundReal32ToUInt32(realMaxY);


  // We make the boundaries safe
  if(minX < 0) { minX = 0; }
  if(minY < 0) { minY = 0; }
  if(maxX > buffer->width) { maxX = buffer->width; }
  if(maxY > buffer->height) { maxY = buffer->height; }


  uint32 color = UTILS::RealRGBToUInt32(R, G, B);

  for(int y = minY;
      y < maxY;
      y++)
  {
    uint8 *pixel = (uint8 *)buffer->memory +
                   y * buffer->pitch +
                   buffer->bytesPerPixel * minX;

    for (int x = minX;
         x < maxX;
         x++)
    {
      *(uint32 *)pixel = color;
      pixel += buffer->bytesPerPixel;
    }
  }
}

internal void
DrawHollowRectangle(game_offscreen_buffer* buffer,
                    real32 realMinX, real32 realMinY,
                    real32 realMaxX, real32 realMaxY,
                    real32 R, real32 G, real32 B)
{
  int32 minX = UTILS::RoundReal32ToUInt32(realMinX);
  int32 maxX = UTILS::RoundReal32ToUInt32(realMaxX);
  int32 minY = UTILS::RoundReal32ToUInt32(realMinY);
  int32 maxY = UTILS::RoundReal32ToUInt32(realMaxY);


  // We make the boundaries safe
  if(minX < 0) { minX = 0; }
  if(minY < 0) { minY = 0; }
  if(maxX > buffer->width) { maxX = buffer->width; }
  if(maxY > buffer->height) { maxY = buffer->height; }


  uint32 color = UTILS::RealRGBToUInt32(R, G, B);

  for(int y = minY;
      y < maxY;
      y++)
  {
    uint8 *pixel = (uint8 *)buffer->memory +
                   y * buffer->pitch +
                   buffer->bytesPerPixel * minX;

    for (int x = minX;
         x < maxX;
         x++)
    {
      bool32 draw = false;
      if(y == minY || y == maxY - 1)
      {
        draw = true;
      }
      else
      {
        if(x == minX || x == maxX - 1)
        {
          draw = true;
        }
      }
      if(draw)
      {
        *(uint32 *)pixel = color;
      }
      pixel += buffer->bytesPerPixel;
    }
  }
}
inline void
DrawTileRelativeToCenter(game_offscreen_buffer* buffer,
                         real32 screenOffsetX, real32 screenOffsetY,
                         int32 tileOffsetX, int32 tileOffsetY,
                         real32 realOffsetX, real32 realOffsetY,
                         real32 tileSizeX, real32 tileSizeY,
                         int32 pixelPaddingX, int32 pixelPaddingY,
                         real32 tileInMeters, real32 metersToPixels,
                         real32 R, real32 G, real32 B)
{
  DrawRectangle(buffer,
    screenOffsetX + (tileOffsetX * tileInMeters - realOffsetX) * metersToPixels,
    screenOffsetY - ((tileOffsetY * tileInMeters + tileSizeY - realOffsetY) * metersToPixels + pixelPaddingY),
    screenOffsetX + (tileOffsetX * tileInMeters + tileSizeX - realOffsetX) * metersToPixels + pixelPaddingX,
    screenOffsetY - (tileOffsetY * tileInMeters - realOffsetY) * metersToPixels,
    R, G, B);
}

#define _GAME_RENDER_CPP
#endif
