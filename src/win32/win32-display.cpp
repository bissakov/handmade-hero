#include "../../src/win32/win32-display.h"

#include <windows.h>

#include "../../src/handmade-hero/handmade-hero.h"

Dimensions GetDimensions(HWND window) {
  RECT rect;
  GetClientRect(window, &rect);
  int width = rect.right - rect.left;
  int height = rect.bottom - rect.top;
  return {width, height};
}

void ResizeDIBSection(Buffer *buffer, int width, int height) {
  if (buffer->memory) {
    VirtualFree(buffer->memory, 0, MEM_RELEASE);
  }

  buffer->width = width;
  buffer->height = height;
  buffer->bytes_per_pixel = 4;

  Assert(buffer->width && buffer->height && buffer->bytes_per_pixel);

  buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
  buffer->info.bmiHeader.biWidth = buffer->width;
  buffer->info.bmiHeader.biHeight = buffer->height * -1;
  buffer->info.bmiHeader.biPlanes = 1;
  buffer->info.bmiHeader.biBitCount = 32;
  buffer->info.bmiHeader.biCompression = BI_RGB;

  int bitmap_memory_size =
      buffer->width * buffer->height * buffer->bytes_per_pixel;
  buffer->memory = VirtualAlloc(0, bitmap_memory_size, MEM_RESERVE | MEM_COMMIT,
                                PAGE_READWRITE);

  buffer->pitch = buffer->width * buffer->bytes_per_pixel;
}

void DisplayBuffer(HDC device_context, int window_x, int window_y,
                   int window_width, int window_height, Buffer *buffer) {
  StretchDIBits(device_context, window_x, window_y, window_width, window_height,
                0, 0, buffer->width, buffer->height, buffer->memory,
                &buffer->info, DIB_RGB_COLORS, SRCCOPY);
}
