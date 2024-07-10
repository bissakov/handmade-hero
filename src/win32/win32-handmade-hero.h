// Copyright 2024 Alikhan Bissakov
// Handmade Hero
// File: handmade-hero.cpp
// ----------------------
// Handmade Hero License by Casey Muratori
// See the end of file for license information

#ifndef WIN32_HANDMADE_HERO_H_

#include <dsound.h>
#include <windows.h>
#include <xinput.h>

#include <cstdint>

#include "../../src/handmade-hero/handmade-hero.h"

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

static inline bool ProcessPendingMessages(ControllerInput *keyboard_controller);

#define WIN32_HANDMADE_HERO_H_
#endif  // WIN32_HANDMADE_HERO_H_

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
