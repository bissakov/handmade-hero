#ifndef WIN32_DISPLAY_H_

#include <windows.h>

struct Buffer {
  BITMAPINFO info;
  void *memory;
  int width;
  int height;
  int pitch;
  int bytes_per_pixel;
};

struct Dimensions {
  int width;
  int height;
};

Dimensions GetDimensions(HWND window);

void ResizeDIBSection(Buffer *buffer, int width, int height);

void DisplayBuffer(HDC device_context, int window_x, int window_y,
                   int window_width, int window_height, Buffer *buffer);

#define WIN32_DISPLAY_H_
#endif  // WIN32_DISPLAY_H_
