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

#include "../handmade-hero/handmade-hero.h"

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
  int tone_hz = 256;
  uint32_t running_sample_idx = 0;
  int bytes_per_sample = sizeof(int16_t) * 2;
  uint16_t tone_volume = 1000;
  int wave_period = 0;
  int secondary_buffer_size = 0;
  float t_sin = 0.0f;
  int latency_sample_count = 0;
};

bool InitXInput();
bool InitDirectSound(HWND window, int samples_per_second, int buffer_size);
Dimensions GetDimensions(HWND window);
void ResizeDIBSection(Buffer *buffer, int width, int height);
LRESULT MainWindowCallback(HWND window, UINT message, WPARAM w_param,
                           LPARAM l_param);
void HandleGamepad();
bool ClearBuffer(SoundOutput *sound_output);
bool FillSoundBuffer(SoundOutput *sound_output, uint32_t byte_to_lock,
                     uint32_t bytes_to_write,
                     GameSoundBuffer *game_sound_buffer);
int CALLBACK WinMain(HINSTANCE instance, HINSTANCE prev_instance,
                     LPSTR cmd_line, int show_code);

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
