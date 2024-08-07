#ifndef SRC_WIN32_WIN32_HANDMADE_HERO_H_
#define SRC_WIN32_WIN32_HANDMADE_HERO_H_

#include <windows.h>
#include <xinput.h>

#include <cstdint>

#include "../../src/win32/win32-display.h"

#ifndef DEV
#define DEV 1
#endif

#ifndef DEBUG
#define DEBUG 1
#endif

struct DebugTimeMarker {
  DWORD play_cursor;
  DWORD write_cursor;
};

static bool RUNNING = true;
static const int DEFAULT_WIDTH = 1920;
static const int DEFAULT_HEIGHT = 1080;

static Buffer BUFFER;
static int64_t perf_count_frequency;

static inline LRESULT CALLBACK MainWindowCallback(HWND window, UINT message,
                                                  WPARAM w_param,
                                                  LPARAM l_param);

#endif  // SRC_WIN32_WIN32_HANDMADE_HERO_H_
