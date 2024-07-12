#ifndef WIN32_HANDMADE_HERO_H_

#include <dsound.h>
#include <windows.h>
#include <xinput.h>

#include <cstdint>

#ifndef DEV
#define DEV 1
#endif

#ifndef DEBUG
#define DEBUG 1
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
