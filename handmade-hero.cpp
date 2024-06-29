// Copyright 2024 Alikhan Bissakov
// Handmade Hero
// File: handmade-hero.cpp
// ----------------------
// Handmade Hero License by Casey Muratori
// See the end of file for license information

#include <windows.h>

#include <cassert>
#include <cstdint>

static bool RUNNING = true;
static const int DEFAULT_WIDTH = 1920;
static const int DEFAULT_HEIGHT = 1080;

struct Buffer {
  BITMAPINFO info;
  void *memory;
  int width;
  int height;
  int pitch;
  int bytes_per_pixel;
};

static Buffer buffer;

struct Dimensions {
  int width;
  int height;
};

static Dimensions GetDimensions(HWND window) {
  RECT rect;
  GetClientRect(window, &rect);
  int width = rect.right - rect.left;
  int height = rect.bottom - rect.top;
  return {width, height};
}

static void render(Buffer *buffer, int x_offset, int y_offset) {
  uint8_t *row = reinterpret_cast<uint8_t *>(buffer->memory);
  for (int y = 0; y < buffer->height; ++y) {
    uint32_t *pixel = reinterpret_cast<uint32_t *>(row);
    for (int x = 0; x < buffer->width; ++x) {
      uint8_t red = x + x_offset;
      uint8_t green = 0;
      uint8_t blue = y + y_offset;
      *pixel++ = (red << 16) | (green << 8) | blue;
    }
    row += buffer->pitch;
  }
}

static void ResizeDIBSection(Buffer *buffer, int width, int height) {
  if (buffer->memory) {
    VirtualFree(buffer->memory, 0, MEM_RELEASE);
  }

  buffer->width = width;
  buffer->height = height;
  buffer->bytes_per_pixel = 4;

  assert(buffer->width && buffer->height && buffer->bytes_per_pixel);

  buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
  buffer->info.bmiHeader.biWidth = buffer->width;
  buffer->info.bmiHeader.biHeight = buffer->height * -1;
  buffer->info.bmiHeader.biPlanes = 1;
  buffer->info.bmiHeader.biBitCount = 32;
  buffer->info.bmiHeader.biCompression = BI_RGB;

  int bitmap_memory_size =
      buffer->width * buffer->height * buffer->bytes_per_pixel;
  buffer->memory =
      VirtualAlloc(0, bitmap_memory_size, MEM_COMMIT, PAGE_READWRITE);

  buffer->pitch = buffer->width * buffer->bytes_per_pixel;
}

static void DisplayBuffer(HDC device_context, int window_x, int window_y,
                          uint16_t window_width, uint16_t window_height,
                          Buffer *buffer) {
  // float aspect_ratio = static_cast<float>(DEFAULT_WIDTH) / DEFAULT_HEIGHT;
  // float buffer_width = static_cast<float>(buffer->width);
  // float buffer_height = static_cast<float>(buffer->height);
  // if (buffer_width / aspect_ratio <= buffer->height) {
  //   buffer_height = buffer_width / aspect_ratio;
  // } else {
  //   buffer_width = buffer_height * aspect_ratio;
  // }
  // float buffer_x = (buffer_width - buffer->width) / 2;
  // float buffer_y = (buffer_height - buffer->height) / 2;
  // StretchDIBits(device_context, window_x, window_y, window_width,
  // window_height,
  //               buffer_x, buffer_y, buffer_width, buffer_height,
  //               buffer->memory, &buffer->info, DIB_RGB_COLORS, SRCCOPY);

  StretchDIBits(device_context, window_x, window_y, window_width, window_height,
                0, 0, buffer->width, buffer->height, buffer->memory,
                &buffer->info, DIB_RGB_COLORS, SRCCOPY);
}

LRESULT MainWindowCallback(HWND window, UINT message, WPARAM w_param,
                           LPARAM l_param) {
  LRESULT result = 0;

  switch (message) {
    case WM_SIZE: {
      break;
    }

    case WM_ACTIVATEAPP: {
      break;
    }

    case WM_DESTROY: {
      RUNNING = false;
      break;
    }

    case WM_CLOSE: {
      RUNNING = false;
      break;
    }

    case WM_PAINT: {
      PAINTSTRUCT paint;
      HDC device_context = BeginPaint(window, &paint);

      Dimensions window_dimensions = GetDimensions(window);
      DisplayBuffer(device_context, 0, 0, window_dimensions.width,
                    window_dimensions.height, &buffer);

      EndPaint(window, &paint);

      break;
    }

    default: {
      result = DefWindowProcW(window, message, w_param, l_param);
      break;
    }
  }

  return result;
}

int CALLBACK WinMain(HINSTANCE instance, HINSTANCE prev_instance,
                     LPSTR cmd_line, int show_code) {
  ResizeDIBSection(&buffer, DEFAULT_WIDTH, DEFAULT_HEIGHT);

  WNDCLASSW window_class = {};

  window_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  window_class.lpfnWndProc = MainWindowCallback;
  window_class.hInstance = instance;
  window_class.lpszClassName = L"HandmadeHeroWindowClass";

  if (!RegisterClassW(&window_class)) {
    return 1;
  }

  wchar_t window_name[] = L"Handmade Hero";
  uint32_t window_style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;

  HWND window = CreateWindowExW(
      0, window_class.lpszClassName, window_name, window_style, CW_USEDEFAULT,
      CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, instance, 0);

  if (!window) {
    return 1;
  }

  HDC device_context = GetDC(window);

  int x_offset = 0;
  int y_offset = 0;

  while (RUNNING) {
    MSG message;
    while (PeekMessageW(&message, 0, 0, 0, PM_REMOVE)) {
      if (message.message == WM_QUIT) {
        RUNNING = false;
        break;
      }
      TranslateMessage(&message);
      DispatchMessageW(&message);
    }

    render(&buffer, x_offset, y_offset);

    Dimensions window_dimensions = GetDimensions(window);
    DisplayBuffer(device_context, 0, 0, window_dimensions.width,
                  window_dimensions.height, &buffer);

    ++y_offset;
    ++x_offset;
  }

  ReleaseDC(window, device_context);

  return 0;
}

// All of the source code, artwork, and sound effects for Handmade Hero
// are Copyright 2014 by Molly Rocket, Inc., and all rights are reserved.
// Anyone who has purchased a copy of Handmade Hero is granted a
// personal, non-assignable, non-transferable, non-commercial license to
// use the source code, artwork, and sound effects for their own personal
// educational purposes. Any other use, including any redistribution in
// whole or in part, requires explicit, written permission from Molly
// Rocket, Inc.
//
// All of the music in Handmade Hero is licensed from Audio Network, who
// retains the copyright thereto. It may only be used for the original
// purpose of playback during execution of the original game, and may not
// be repurposed, redistributed, modified, or used in any other way. If
// you would like to use the music from Handmade Hero for any other
// purpose, you must contact Audio Network and negotiate a separate
// license agreement.
//
// Handmade Hero and its source materials are provided "as is" without
// warranty of any kind, either express or implied, including without
// limitation any implied warranties of condition, uninterrupted use,
// merchantability, fitness for a particular purpose, or
// non-infringement.
