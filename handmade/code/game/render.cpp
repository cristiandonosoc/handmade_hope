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

  uint32 color = UTILS::FLOAT::RealRGBToUInt32(R, G, B);

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
DrawRectangle(game_offscreen_buffer* buffer,
              v2<real32> min,
              v2<real32> max,
              real32 R, real32 G, real32 B)
{
  int32 minX = UTILS::FLOAT::RoundReal32ToUInt32(min.x);
  int32 maxX = UTILS::FLOAT::RoundReal32ToUInt32(max.x);
  int32 minY = UTILS::FLOAT::RoundReal32ToUInt32(min.y);
  int32 maxY = UTILS::FLOAT::RoundReal32ToUInt32(max.y);


  // We make the boundaries safe
  if(minX < 0) { minX = 0; }
  if(minY < 0) { minY = 0; }
  if(maxX > buffer->width) { maxX = buffer->width; }
  if(maxY > buffer->height) { maxY = buffer->height; }


  uint32 color = UTILS::FLOAT::RealRGBToUInt32(R, G, B);

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
                    v2<real32> min,
                    v2<real32> max,
                    real32 R, real32 G, real32 B)
{
  int32 minX = UTILS::FLOAT::RoundReal32ToUInt32(min.x);
  int32 maxX = UTILS::FLOAT::RoundReal32ToUInt32(max.x);
  int32 minY = UTILS::FLOAT::RoundReal32ToUInt32(min.y);
  int32 maxY = UTILS::FLOAT::RoundReal32ToUInt32(max.y);

  // We make the boundaries safe
  if(minX < 0) { minX = 0; }
  if(minY < 0) { minY = 0; }
  if(maxX > buffer->width) { maxX = buffer->width; }
  if(maxY > buffer->height) { maxY = buffer->height; }


  uint32 color = UTILS::FLOAT::RealRGBToUInt32(R, G, B);

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
                         real32 centerPositionX, real32 centerPositionY,
                         int32 tileOffsetX, int32 tileOffsetY,
                         real32 realOffsetX, real32 realOffsetY,
                         real32 tileSizeX, real32 tileSizeY,
                         int32 pixelPaddingX, int32 pixelPaddingY,
                         real32 tileInMeters, real32 metersToPixels,
                         real32 R, real32 G, real32 B)
{
  v2<real32> min = {
    centerPositionX + (tileOffsetX * tileInMeters - realOffsetX) * metersToPixels,
    centerPositionY - ((tileOffsetY * tileInMeters + tileSizeY - realOffsetY) * metersToPixels + pixelPaddingY),
  };
  v2<real32> max = {
    centerPositionX + (tileOffsetX * tileInMeters + tileSizeX - realOffsetX) * metersToPixels + pixelPaddingX,
    centerPositionY - (tileOffsetY * tileInMeters - realOffsetY) * metersToPixels,
  };
  DrawRectangle(buffer, min, max, R, G, B);
}

inline void
DrawRectangleRelativeToCenter(game_offscreen_buffer* buffer,
                              real32 centerPositionX, real32 centerPositionY,
                              real32 offsetX, real32 offsetY,
                              real32 sizeX, real32 sizeY,
                              real32 R, real32 G, real32 B)
{
  v2<real32> min = {
    centerPositionX + offsetX,
    centerPositionY + (offsetY - sizeY)
  };
  v2<real32> max = {
    centerPositionX + offsetX + sizeX,
    centerPositionY + offsetY // Inverted axis
  };
  DrawRectangle(buffer, min, max, R, G, B);
}

inline void
DrawHollowRectangleRelativeToCenter(game_offscreen_buffer* buffer,
                                    real32 centerPositionX, real32 centerPositionY,
                                    real32 offsetX, real32 offsetY,
                                    real32 sizeX, real32 sizeY,
                                    real32 R, real32 G, real32 B)
{
  v2<real32> min = {
    centerPositionX + offsetX,
    centerPositionY + (offsetY - sizeY)
  };
  v2<real32> max = {
    centerPositionX + offsetX + sizeX,
    centerPositionY + offsetY // Inverted axis
  };
  DrawHollowRectangle(buffer, min, max, R, G, B);
}



internal void
DrawBitmap(game_offscreen_buffer* buffer, bitmap_definition bitmap,
           real32 screenX, real32 screenY,
           int32 pixelOffsetX, int32 pixelOffsetY,
           bool32 inverted)
{
  int32 minX = UTILS::FLOAT::RoundReal32ToUInt32(screenX - pixelOffsetX);
  int32 maxX = UTILS::FLOAT::RoundReal32ToUInt32(screenX - pixelOffsetX + bitmap.header.width);
  int32 minY = UTILS::FLOAT::RoundReal32ToUInt32(screenY - pixelOffsetY);
  int32 maxY = UTILS::FLOAT::RoundReal32ToUInt32(screenY - pixelOffsetY + bitmap.header.height);

  // We make the boundaries safe
  int32 offsetX = 0;
  if(minX < 0)
  {
    offsetX = -minX;
    minX = 0;
  }
  int32 offsetY = 0;
  if(minY < 0)
  {
    offsetY = -minY;
    minY = 0;
  }
  if(maxX > buffer->width) { maxX = buffer->width; }
  if(maxY > buffer->height) { maxY = buffer->height; }

  // We draw the bitmap
  int32 bitmapWidth = bitmap.header.width;
  if(bitmapWidth > buffer->width)
  {
    bitmapWidth = buffer->width;
  }
  int32 bitmapHeight = bitmap.header.height;
  if(bitmapHeight > buffer->height)
  {
    bitmapHeight = buffer->height;
  }

  uint32* bufferPixel = (uint32*)buffer->memory;
  uint32* firstPixelOfBufferRow = bufferPixel + minY * buffer->width;
  uint32* bitmapPixel = bitmap.pixels;
  uint32* firstPixelOfBitmapRow = bitmapPixel + offsetY * bitmap.header.width;
  int32 bitmapDiff = bitmap.header.width;
  if(inverted)
  {
    firstPixelOfBitmapRow = bitmapPixel + (bitmap.header.height - offsetY - 1) * bitmap.header.width;
    bitmapDiff = -bitmapDiff;
  }
  for(int32 y = minY;
      y < maxY;
      y++)
  {
    bufferPixel = firstPixelOfBufferRow + minX;
    bitmapPixel = firstPixelOfBitmapRow + offsetX;
    for(int32 x = minX;
        x < maxX;
        x++)
    {
      // We calculate the pixel position
      real32 t = (*bitmapPixel >> 24) / 255.0f;
      real32 sourceRed = (real32)((*bufferPixel >> 16) & 0xFF);
      real32 sourceGreen = (real32)((*bufferPixel >> 8) & 0xFF);
      real32 sourceBlue = (real32)((*bufferPixel >> 0) & 0xFF);

      real32 destRed = (real32)((*bitmapPixel >> 16) & 0xFF);
      real32 destGreen = (real32)((*bitmapPixel >> 8) & 0xFF);
      real32 destBlue = (real32)((*bitmapPixel >> 0) & 0xFF);

      // We do the linear blending in floating space
      int32 calcRed = (UTILS::FLOAT::RoundReal32ToUInt32((1 - t) * sourceRed + t * destRed) & 0xFF);
      int32 calcGreen = (UTILS::FLOAT::RoundReal32ToUInt32((1 - t) * sourceGreen + t * destGreen) & 0xFF);
      int32 calcBlue = (UTILS::FLOAT::RoundReal32ToUInt32((1 - t) * sourceBlue + t * destBlue) & 0xFF);

      *bufferPixel = ((calcRed << 16) |
                      (calcGreen << 8) |
                      (calcBlue << 0));

      bufferPixel++;
      bitmapPixel++;
    }
    firstPixelOfBufferRow += buffer->width;
    firstPixelOfBitmapRow += bitmapDiff;
  }
}

internal void
DrawBitmapRelativeToCenter(game_offscreen_buffer* buffer, bitmap_definition bitmap,
                           real32 centerPositionX, real32 centerPositionY,
                           real32 offsetX, real32 offsetY,
                           int32 pixelOffsetX, int32 pixelOffsetY,
                           bool32 inverted)
{
  DrawBitmap(buffer, bitmap,
             centerPositionX + offsetX, centerPositionY + offsetY,
             pixelOffsetX, pixelOffsetY,
             inverted);
}

#define _GAME_RENDER_CPP
#endif
