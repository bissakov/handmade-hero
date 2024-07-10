// Copyright 2024 Alikhan Bissakov
// Handmade Hero
// File: handmade-hero.cpp
// ----------------------
// Handmade Hero License by Casey Muratori
// See the end of file for license information

#include "../../src/win32/win32-handmade-hero.h"

#include <dsound.h>
#include <fileapi.h>
#include <math.h>
#include <windows.h>
#include <xinput.h>

#include <cassert>
#include <cstdint>
#include <cstdio>

#include "../../src/handmade-hero/handmade-hero.h"
#include "../../src/win32/win32-file-io.h"
#include "../../src/win32/win32-input.h"
#include "../../src/win32/win32-sound.h"

static bool RUNNING = true;
static const int DEFAULT_WIDTH = 1920;
static const int DEFAULT_HEIGHT = 1080;

static Buffer BUFFER;

static inline Dimensions GetDimensions(HWND window) {
  RECT rect;
  GetClientRect(window, &rect);
  int width = rect.right - rect.left;
  int height = rect.bottom - rect.top;
  return {width, height};
}

static inline void ResizeDIBSection(Buffer *buffer, int width, int height) {
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

static inline void DisplayBuffer(HDC device_context, int window_x, int window_y,
                                 int window_width, int window_height,
                                 Buffer *buffer) {
  StretchDIBits(device_context, window_x, window_y, window_width, window_height,
                0, 0, buffer->width, buffer->height, buffer->memory,
                &buffer->info, DIB_RGB_COLORS, SRCCOPY);
}

static inline bool ProcessPendingMessages(
    ControllerInput *keyboard_controller) {
  bool result = true;

  MSG message;
  while (PeekMessageW(&message, 0, 0, 0, PM_REMOVE)) {
    switch (message.message) {
      case WM_SYSKEYDOWN: {
        uint32_t vk_code = (uint32_t)message.wParam;

        bool alt_key_down =
            (static_cast<uint32_t>(message.lParam) & (1U << 29)) != 0;
        if ((vk_code == VK_F4) && alt_key_down) {
          result = false;
          break;
        }

        break;
      }

      case WM_SYSKEYUP: {
        break;
      }

      case WM_KEYDOWN: {
        // works

        uint32_t vk_code = (uint32_t)message.wParam;

        if (vk_code == VK_ESCAPE) {
          result = false;
          break;
        }

        bool was_key_down =
            (static_cast<int32_t>(message.lParam) & (1U << 30)) != 0;
        bool is_key_down =
            (static_cast<int32_t>(message.lParam) & (1U << 31)) == 0;

        if (was_key_down != is_key_down) {
          HandleKeyboard(keyboard_controller, vk_code, is_key_down);
        }
        break;
      }

      case WM_KEYUP: {
        break;
      }

      default: {
        TranslateMessage(&message);
        DispatchMessageW(&message);
      }
    }
  }

  return result;
}

static inline LRESULT CALLBACK MainWindowCallback(HWND window, UINT message,
                                                  WPARAM w_param,
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
                    window_dimensions.height, &BUFFER);

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
                     LPSTR command_line, int show_code) {
#if 0
  {
    wchar_t file_path[] = L"D:\\Work\\Bear\\CMakeCache.txt";
    FileResult result = ReadEntireFileDebug(file_path);
    if (result.content) {
      wchar_t dup_file_path[] = L"D:\\Work\\Bear\\CMakeCache2.txt";
      if (!WriteEntireFileDebug(dup_file_path, result.file_size,
                                result.content)) {
        FreeFileMemoryDebug(&result.content);
        return 1;
      }
      FreeFileMemoryDebug(&result.content);
    }
  }
#endif

  if (!InitXInput()) {
    OutputDebugStringW(L"XInput initialization failed\n");
    return ERROR_DEVICE_NOT_CONNECTED;
  }
  ResizeDIBSection(&BUFFER, DEFAULT_WIDTH, DEFAULT_HEIGHT);

  WNDCLASSW window_class = {};

  window_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  window_class.lpfnWndProc = MainWindowCallback;
  window_class.hInstance = instance;
  window_class.lpszClassName = L"HandmadeHeroWindowClass";

  if (!RegisterClassW(&window_class)) {
    OutputDebugStringW(L"Window class registration failed\n");
    return 1;
  }

  wchar_t window_name[] = L"Handmade Hero";
  uint32_t window_style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;

  HWND window = CreateWindowExW(
      0, window_class.lpszClassName, window_name, window_style, CW_USEDEFAULT,
      CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, instance, 0);

  if (!window) {
    OutputDebugStringW(L"Window creation failed\n");
    return 1;
  }

  HDC device_context = GetDC(window);

  SoundOutput sound_output;
  sound_output.secondary_buffer_size =
      sound_output.samples_per_second * sound_output.bytes_per_sample;
  sound_output.latency_sample_count = sound_output.samples_per_second / 15;

  IDirectSoundBuffer *sound_buffer =
      InitDirectSound(window, sound_output.samples_per_second,
                      sound_output.secondary_buffer_size);
  if (!sound_buffer) {
    OutputDebugStringW(L"DirectSound initialization failed\n");
    return 1;
  }

  if (!ClearBuffer(sound_buffer, &sound_output)) {
    return 1;
  }

  if (!SUCCEEDED(sound_buffer->Play(0, 0, DSBPLAY_LOOPING))) {
    return 1;
  }

  LARGE_INTEGER perf_count_frequency_result;
  QueryPerformanceFrequency(&perf_count_frequency_result);
  int64_t perf_count_frequency = perf_count_frequency_result.QuadPart;

  LARGE_INTEGER last_counter;
  QueryPerformanceCounter(&last_counter);

  uint64_t last_cycle_count = __rdtsc();

  int16_t *samples = reinterpret_cast<int16_t *>(
      VirtualAlloc(0, sound_output.secondary_buffer_size,
                   MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE));

  if (!samples) {
    OutputDebugStringW(L"Samples allocation failed\n");
    return 1;
  }

#if DEV
  LPVOID base_address = (LPVOID)Terabytes((uint64_t)2);
#else
  LPVOID base_address = 0;
#endif

  GameMemory memory = {};
  memory.permanent_storage_size = Megabytes(64);
  memory.transient_storage_size = Gigabytes((uint64_t)1);
  uint64_t total_memory_size =
      memory.permanent_storage_size + memory.transient_storage_size;

  memory.permanent_storage =
      VirtualAlloc(base_address, static_cast<size_t>(total_memory_size),
                   MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
  memory.transient_storage =
      reinterpret_cast<uint8_t *>(memory.permanent_storage) +
      memory.permanent_storage_size;

  Assert(sizeof(GameState) <= memory.permanent_storage_size);

  if (!memory.permanent_storage) {
    OutputDebugStringW(L"Memory allocation failed\n");
    return 1;
  }

  GameInput old_input = {};
  GameInput new_input = {};

  while (RUNNING) {
    ControllerInput *keyboard_controller = &new_input.controllers[0];
    ControllerInput empty_controller = {};
    *keyboard_controller = empty_controller;

    if (!ProcessPendingMessages(keyboard_controller)) {
      break;
    }

    HandleGamepad(&old_input, &new_input);

    DWORD byte_to_lock = 0;
    DWORD target_cursor = 0;
    DWORD play_cursor = 0;
    DWORD bytes_to_write = 0;
    DWORD write_cursor;
    bool sound_is_valid = false;
    if (SUCCEEDED(
            sound_buffer->GetCurrentPosition(&play_cursor, &write_cursor))) {
      byte_to_lock =
          (sound_output.running_sample_idx * sound_output.bytes_per_sample) %
          sound_output.secondary_buffer_size;
      target_cursor = (play_cursor + (sound_output.latency_sample_count *
                                      sound_output.bytes_per_sample)) %
                      sound_output.secondary_buffer_size;
      bytes_to_write = 0;

      if (byte_to_lock > target_cursor) {
        bytes_to_write = sound_output.secondary_buffer_size - byte_to_lock;
        bytes_to_write += target_cursor;

      } else {
        bytes_to_write = target_cursor - byte_to_lock;
      }
      sound_is_valid = true;
    }

    GameBuffer game_buffer = {};
    game_buffer.memory = BUFFER.memory;
    game_buffer.width = BUFFER.width;
    game_buffer.height = BUFFER.height;
    game_buffer.pitch = BUFFER.pitch;
    game_buffer.bytes_per_pixel = BUFFER.bytes_per_pixel;

    GameSoundBuffer game_sound_buffer = {};
    game_sound_buffer.samples_per_second = sound_output.samples_per_second;
    game_sound_buffer.sample_count =
        bytes_to_write / sound_output.bytes_per_sample;
    game_sound_buffer.samples = samples;

    UpdateAndRender(&memory, &game_buffer, &game_sound_buffer, &new_input);

    if (sound_is_valid) {
      FillSoundBuffer(sound_buffer, &sound_output, byte_to_lock, bytes_to_write,
                      &game_sound_buffer);
    }

    Dimensions window_dimensions = GetDimensions(window);
    DisplayBuffer(device_context, 0, 0, window_dimensions.width,
                  window_dimensions.height, &BUFFER);

    uint64_t end_cycle_count = __rdtsc();

    LARGE_INTEGER end_counter;
    QueryPerformanceCounter(&end_counter);

    float megacycles_elapsed =
        static_cast<float>((end_cycle_count - last_cycle_count) / 1'000'000.0f);
    uint64_t counter_elapsed = end_counter.QuadPart - last_counter.QuadPart;
    float ms_per_frame =
        static_cast<float>(1000.0f * counter_elapsed) / perf_count_frequency;
    float fps = static_cast<float>(perf_count_frequency) / counter_elapsed;

    char buffer[256];
    snprintf(buffer, sizeof(buffer), "%.02f ms/f\t%.02f fps\t%.02fmc/f\n",
             ms_per_frame, fps, megacycles_elapsed);
    OutputDebugStringA(buffer);

    last_counter = end_counter;
    last_cycle_count = end_cycle_count;

    SwapInputs(&old_input, &new_input);
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
