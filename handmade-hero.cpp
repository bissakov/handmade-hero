// Handmade Hero
// File: handmade-hero.cpp
// ----------------------
// Handmade Hero License
//
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
//
// Modified by:
// Alikhan Bissakov 2024

#include <windows.h>
#include <cstdint>

#define internal static
#define local_persist static
#define global_variable static

global_variable bool running;
global_variable BITMAPINFO bitmap_info;
global_variable void *bitmap_memory;

global_variable int bitmap_width;
global_variable int bitmap_height;

struct Dimensions {
  int width;
  int height;
};

Dimensions CalculateDimensions(const RECT *rect) {
  int width = rect->right - rect->left;
  int height = rect->bottom - rect->top;
  return {width, height};
}

internal void ResizeDIBSection(RECT *rect) {
  if (bitmap_memory) {
    VirtualFree(bitmap_memory, 0, MEM_RELEASE);
  }

  Dimensions dimensions = CalculateDimensions(rect);
  int width = dimensions.width;
  int height = dimensions.height;

  bitmap_width = width;
  bitmap_height = height;

  bitmap_info.bmiHeader.biSize = sizeof(bitmap_info.bmiHeader);
  bitmap_info.bmiHeader.biWidth = bitmap_width;
  bitmap_info.bmiHeader.biHeight = bitmap_height;
  bitmap_info.bmiHeader.biPlanes = 1;
  bitmap_info.bmiHeader.biBitCount = 32;
  bitmap_info.bmiHeader.biCompression = BI_RGB;

  int bytes_per_pixel = 4;
  int bitmap_memory_size = bitmap_width * bitmap_height * bytes_per_pixel;
  // VirtualAlloc(LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType,
  //              DWORD flProtect)
  bitmap_memory =
      VirtualAlloc(0, bitmap_memory_size, MEM_COMMIT, PAGE_READWRITE);
}

internal void UpdateClientWindow(HDC device_context, RECT *rect) {
  Dimensions dimensions = CalculateDimensions(rect);
  int window_width = dimensions.width;
  int window_height = dimensions.height;
  StretchDIBits(device_context, 0, 0, bitmap_width, bitmap_height, 0, 0,
                window_width, window_height, bitmap_memory, &bitmap_info,
                DIB_RGB_COLORS, SRCCOPY);
}

LRESULT MainWindowCallback(HWND window, UINT message, WPARAM w_param,
                           LPARAM l_param) {
  LRESULT result = 0;

  switch (message) {
    case WM_ACTIVATEAPP: {
      break;
    }

    case WM_SIZE: {
      RECT client_rect;
      GetClientRect(window, &client_rect);

      ResizeDIBSection(&client_rect);

      break;
    }

    case WM_DESTROY: {
      running = false;
      break;
    }

    case WM_CLOSE: {
      running = false;
      break;
    }

    case WM_PAINT: {
      PAINTSTRUCT paint;
      HDC device_context = BeginPaint(window, &paint);

      UpdateClientWindow(device_context, &paint.rcPaint);

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
  WNDCLASSW window_class = {};

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

  MSG message;
  running = true;
  while (running) {
    BOOL message_result = GetMessageW(&message, 0, 0, 0);

    if (message_result == -1) {
      return -1;
    }

    TranslateMessage(&message);
    DispatchMessageW(&message);
  }

  return 0;
}
