/*  =====================================================================
    $File: graphics_wrapper.cpp
    $Creation Date: 2015-01-09
    $Last Modified: $
    $Revision: $
    $Creator: Cristián Donoso $
    $Notice: (c) Copyright 2015 Cristián Donoso $
    =====================================================================

    Has the graphics functionalities of the graphics rendering in windows.

    ===================================================================== */

#ifndef _WIN32_GRAPHICS_WRAPPER_INCLUDED

#include "common_types.h"
#include <windows.h>

struct win32_offscreen_buffer
{
  BITMAPINFO info;
  void *memory;
  int width;
  int height;
  int pitch;
  int bytesPerPixel;
};

struct win32_window_dimension {
  int width;
  int height;
};

/****** GLOBAL VARIABLES ******/

global_variable win32_offscreen_buffer gBackBuffer;

/**
 * Creates a win32_window_dimension struct from a HWND window handle
 */
internal win32_window_dimension
Win32GetWindowDimension(HWND windowHandle)
{
  win32_window_dimension result;

  RECT clientRect;
  GetClientRect(windowHandle, &clientRect);
  result.width = clientRect.right - clientRect.left;
  result.height = clientRect.bottom - clientRect.top;

  return(result);
}

/**
 * (Re)Creates a Device Independent Bitmap to match the current window size
 * @param *buffer A pointer to the win32_offscreen_buffer. It is important that it is
 *                a pointer because it will modify the buffer and the changes must endure.
 *                (This makes it difficult to the compiler to inline, but is les error-prone).
 */
internal void
Win32ResizeDIBSection(win32_offscreen_buffer *buffer, int width, int height)
{
  //TODO(Cristián): Bulletproof this
  // Maybe don't free first, free after, then free first if that fails

  // TODO(Cristián): Free our DIBSection
  // We check whether we have created a bitmap
  if(buffer->memory)
  {
    VirtualFree(buffer->memory, 0, MEM_RELEASE);
  }

  // We update the buffer width
  buffer->width = width;
  buffer->height = height;
  buffer->bytesPerPixel = 4;

  // We set the BITMAPINFO data
  buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
  buffer->info.bmiHeader.biWidth = width;
  buffer->info.bmiHeader.biHeight = -height; // Negative height means top down bitmap (BITMAPINFO MSDN)
  buffer->info.bmiHeader.biPlanes = 1;
  buffer->info.bmiHeader.biBitCount = 32;
  buffer->info.bmiHeader.biCompression = BI_RGB;

  // NOTE(Cristián): There was a confusion on whether to use StretchDIBits or
  // BitBlt for passing the bitmap from our program to windows. The second method
  // is actually a transfer from DC to DC. With the first we can pass a pointer to
  // the bit map in memory with no reference to a DC. This means we can allocate
  // and maintain the memory ourselves.

  // We allocate the memory necesary for the buffer
  int bitmapMemorySize =  buffer->bytesPerPixel *
                          buffer->width *
                          buffer->height;
  buffer->memory = VirtualAlloc(0, bitmapMemorySize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
  buffer->pitch = buffer->width * buffer->bytesPerPixel;

}

internal void
Win32ClearWindow(HDC deviceContext, int windowWidth, int windowHeight)
{
  PatBlt(deviceContext, 0, 0,
                        windowWidth, windowHeight,
                        BLACKNESS);
}


/**
 * Transfer our buffer into windows via StretchDIBits syscall.
 * @param *buffer A pointer to the buffer info struct. We use a pointer because
 *                it is a fairly big struct so that if the compiler doesn't inline
 *                the code, it would require a 'big' memory copy in the stack. So
 *                we send the pointer even though we do not modify the origin struct.
 */
internal void
Win32TransferBufferToWindows(HDC deviceContext,
                             win32_offscreen_buffer *buffer,
                             int windowWidth, int windowHeight)
{



  int offsetX = 10;
  int offsetY = 10;

  // NOTE(Cristián): We are avoiding dirty (partial) rectangles for now
  // NOTE(Cristián): For prototyping, we're are always going to blit 1-1 pixels
  // to make sure we don't introduce artifacts. This will change when the renderer
  // code is done.
  StretchDIBits(deviceContext,
                /*
                x, y, destWidth, destHeight,
                x, y, originWidth, originHeight,
                */
                // 0, 0, windowWidth, windowHeight,
                offsetX, offsetY, buffer->width, buffer->height,
                0, 0, buffer->width, buffer->height,
                buffer->memory,
                &buffer->info,
                DIB_RGB_COLORS,
                SRCCOPY);
}

#define _WIN32_GRAPHICS_WRAPPER_INCLUDED
#endif
