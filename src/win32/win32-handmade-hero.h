#ifndef WIN32_HANDMADE_HERO_H_

#include <dsound.h>
#include <windows.h>
#include <xinput.h>

#include <cstdint>

#define DEV 1
#define DEBUG 1

#if DEBUG
#define Assert(expression)              \
  if (!static_cast<bool>(expression)) { \
    __debugbreak();                     \
  }
#else
#define Assert(expression)
#endif

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

static inline Dimensions GetDimensions(HWND window);

static inline void ResizeDIBSection(Buffer *buffer, int width, int height);
static inline void DisplayBuffer(HDC device_context, int window_x, int window_y,
                                 uint16_t window_width, uint16_t window_height,
                                 Buffer *buffer);
static inline LRESULT CALLBACK MainWindowCallback(HWND window, UINT message,
                                                  WPARAM w_param,
                                                  LPARAM l_param);

#define WIN32_HANDMADE_HERO_H_
#endif  // WIN32_HANDMADE_HERO_H_
