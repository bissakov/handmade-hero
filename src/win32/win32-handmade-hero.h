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

typedef DWORD WINAPI XInputGetStateT(DWORD controller_idx,
                                     XINPUT_STATE *controller_state);
typedef DWORD WINAPI XInputSetStateT(DWORD controller_idx,
                                     XINPUT_VIBRATION *vibration);
typedef HRESULT WINAPI DirectSoundCreateT(LPGUID lpGuid, LPDIRECTSOUND *ppDS,
                                          LPUNKNOWN pUnkOuter);

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

struct SoundOutput {
  int samples_per_second = 48000;
  uint32_t running_sample_idx = 0;
  int bytes_per_sample = sizeof(int16_t) * 2;
  uint16_t tone_volume = 1000;
  int secondary_buffer_size = 0;
  float t_sin = 0.0f;
  int latency_sample_count = 0;
};

struct FileResult {
  uint32_t file_size;
  void *content;
};

static inline bool InitXInput();
static inline bool InitDirectSound(HWND window, int samples_per_second,
                                   int buffer_size);

static inline Dimensions GetDimensions(HWND window);

static inline void ResizeDIBSection(Buffer *buffer, int width, int height);
static inline void DisplayBuffer(HDC device_context, int window_x, int window_y,
                                 uint16_t window_width, uint16_t window_height,
                                 Buffer *buffer);
static inline LRESULT MainWindowCallback(HWND window, UINT message,
                                         WPARAM w_param, LPARAM l_param);

static inline void ProcessXInputDigitalButton(ButtonState *old_state,
                                              ButtonState *new_state,
                                              DWORD xinput_button_state,
                                              DWORD button_bit);
static inline void HandleGamepad(GameInput *old_input, GameInput *new_input);

static inline bool ClearBuffer(SoundOutput *sound_output);
static inline bool FillSoundBuffer(SoundOutput *sound_output,
                                   uint32_t byte_to_lock,
                                   uint32_t bytes_to_write,
                                   GameSoundBuffer *game_sound_buffer);

static inline void SwapInputs(GameInput *old_input, GameInput *new_input);

#if DEV
static inline FileResult ReadEntireFileDebug(wchar_t *file_path);
static inline void FreeFileMemoryDebug(void **memory);

static inline bool WriteEntireFileDebug(wchar_t *file_path,
                                        uint32_t memory_size, void *memory);
#endif

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
