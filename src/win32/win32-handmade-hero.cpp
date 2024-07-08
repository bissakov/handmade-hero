// Copyright 2024 Alikhan Bissakov
// Handmade Hero
// File: handmade-hero.cpp
// ----------------------
// Handmade Hero License by Casey Muratori
// See the end of file for license information

#include "../../src/win32/win32-handmade-hero.h"

#include <dsound.h>
#include <math.h>
#include <windows.h>
#include <xinput.h>

#include <cassert>
#include <cstdint>
#include <cstdio>

#include "../../src/handmade-hero/handmade-hero.h"

static XInputGetStateT *DyXInputGetState;
static XInputSetStateT *DyXInputSetState;
static IDirectSoundBuffer *SOUND_BUFFER;

static bool RUNNING = true;
static const int DEFAULT_WIDTH = 1920;
static const int DEFAULT_HEIGHT = 1080;

static Buffer BUFFER;

static bool InitXInput() {
  HMODULE xinput_lib = LoadLibraryW(L"xinput1_4.dll");
  if (!xinput_lib) {
    return false;
  }

  DyXInputGetState = reinterpret_cast<XInputGetStateT *>(
      GetProcAddress(xinput_lib, "XInputGetState"));
  DyXInputSetState = reinterpret_cast<XInputSetStateT *>(
      GetProcAddress(xinput_lib, "XInputSetState"));

  if (!DyXInputGetState || !DyXInputSetState) {
    return false;
  }

  FreeLibrary(xinput_lib);
  return true;
}

static bool InitDirectSound(HWND window, int samples_per_second,
                            int buffer_size) {
  HMODULE direct_sound_lib = LoadLibraryW(L"dsound.dll");
  if (!direct_sound_lib) {
    return false;
  }

  DirectSoundCreateT *DyDirectSoundCreate =
      reinterpret_cast<DirectSoundCreateT *>(
          GetProcAddress(direct_sound_lib, "DirectSoundCreate"));

  FreeLibrary(direct_sound_lib);

  IDirectSound *direct_sound;
  if (!DyDirectSoundCreate ||
      !SUCCEEDED(DyDirectSoundCreate(0, &direct_sound, 0))) {
    return false;
  }

  if (!SUCCEEDED(direct_sound->SetCooperativeLevel(window, DSSCL_PRIORITY))) {
    return false;
  }

  IDirectSoundBuffer *primary_buffer;
  {
    DSBUFFERDESC buffer_desc = {};
    buffer_desc.dwSize = sizeof(buffer_desc);
    buffer_desc.dwFlags = DSBCAPS_PRIMARYBUFFER;

    if (!SUCCEEDED(direct_sound->CreateSoundBuffer(&buffer_desc,
                                                   &primary_buffer, 0))) {
      return false;
    }
  }

  WAVEFORMATEX wave_format = {};
  wave_format.wFormatTag = WAVE_FORMAT_PCM;
  wave_format.nChannels = 2;
  wave_format.nSamplesPerSec = samples_per_second;
  wave_format.wBitsPerSample = 16;
  wave_format.nBlockAlign =
      (wave_format.nChannels * wave_format.wBitsPerSample) / 8;
  wave_format.nAvgBytesPerSec =
      wave_format.nSamplesPerSec * wave_format.nBlockAlign;
  wave_format.cbSize = 0;

  if (!SUCCEEDED(primary_buffer->SetFormat(&wave_format))) {
    return false;
  }

  {
    DSBUFFERDESC buffer_desc = {};
    buffer_desc.dwSize = sizeof(buffer_desc);
    buffer_desc.dwFlags = 0;
    buffer_desc.dwBufferBytes = buffer_size;
    buffer_desc.lpwfxFormat = &wave_format;
    if (!SUCCEEDED(
            direct_sound->CreateSoundBuffer(&buffer_desc, &SOUND_BUFFER, 0))) {
      return false;
    }
  }

  return true;
}

static Dimensions GetDimensions(HWND window) {
  RECT rect;
  GetClientRect(window, &rect);
  int width = rect.right - rect.left;
  int height = rect.bottom - rect.top;
  return {width, height};
}

static void ResizeDIBSection(Buffer *buffer, int width, int height) {
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

static void DisplayBuffer(HDC device_context, int window_x, int window_y,
                          int window_width, int window_height, Buffer *buffer) {
  StretchDIBits(device_context, window_x, window_y, window_width, window_height,
                0, 0, buffer->width, buffer->height, buffer->memory,
                &buffer->info, DIB_RGB_COLORS, SRCCOPY);
}

static LRESULT MainWindowCallback(HWND window, UINT message, WPARAM w_param,
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

    case WM_SYSKEYDOWN: {
      WPARAM vk_code = w_param;

      bool alt_key_down = (l_param & (1 << 29)) != 0;
      if ((vk_code == VK_F4) && alt_key_down) {
        RUNNING = false;
        break;
      }

      break;
    }

    case WM_SYSKEYUP: {
      break;
    }

    case WM_KEYDOWN: {
      break;
    }

    case WM_KEYUP: {
      WPARAM vk_code = w_param;

      bool was_key_down = (l_param & (1 << 30)) != 0;
      bool is_key_down = (l_param & (1 << 31)) == 0;

      if (was_key_down == is_key_down) {
        break;
      }

      if (vk_code == VK_UP) {
        OutputDebugStringW(L"VK_UP: ");
        if (is_key_down) {
          OutputDebugStringW(L"is_key_down ");
        }
        if (was_key_down) {
          OutputDebugStringW(L"was_key_down");
        }
        OutputDebugStringA("\n");
      }

      if (vk_code == VK_DOWN) {
        OutputDebugStringW(L"VK_UP: ");
        if (is_key_down) {
          OutputDebugStringW(L"is_key_down ");
        }
        if (was_key_down) {
          OutputDebugStringW(L"was_key_down");
        }
        OutputDebugStringA("\n");
      }

      if (vk_code == VK_ESCAPE) {
        RUNNING = false;
        break;
      }

      break;
    }

    default: {
      result = DefWindowProcW(window, message, w_param, l_param);
      break;
    }
  }

  return result;
}

static void ProcessXInputDigitalButton(ButtonState *old_state,
                                       ButtonState *new_state,
                                       DWORD xinput_button_state,
                                       DWORD button_bit) {
  new_state->ended_down = (xinput_button_state & button_bit) == button_bit;
  new_state->half_transition_count =
      (old_state->ended_down != new_state->ended_down) ? 1 : 0;
}

static void HandleGamepad(GameInput *old_input, GameInput *new_input) {
  if (!DyXInputGetState || !DyXInputSetState) {
    return;
  }

  int max_supported_controller_count = ArraySize(old_input->controllers);
  int max_controller_count = (XUSER_MAX_COUNT > max_supported_controller_count)
                                 ? max_supported_controller_count
                                 : XUSER_MAX_COUNT;

  for (int controller_idx = 0; controller_idx < max_controller_count;
       ++controller_idx) {
    ControllerInput *old_controller = &old_input->controllers[controller_idx];
    ControllerInput *new_controller = &new_input->controllers[controller_idx];

    old_controller->is_analog = true;
    new_controller->is_analog = true;

    XINPUT_STATE controller_state;
    if (DyXInputGetState(controller_idx, &controller_state) == ERROR_SUCCESS) {
      XINPUT_GAMEPAD *gamepad = &controller_state.Gamepad;

      // bool up = gamepad->wButtons & XINPUT_GAMEPAD_DPAD_UP;
      // bool down = gamepad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN;
      // bool left = gamepad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT;
      // bool right = gamepad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT;

      float stick_x = (gamepad->sThumbLX < 0)
                          ? static_cast<float>(gamepad->sThumbLX) / 32768.0f
                          : static_cast<float>(gamepad->sThumbLX) / 32767.0f;
      new_controller->start_x = old_controller->end_x;

      new_controller->min_x = stick_x;
      new_controller->max_x = stick_x;
      new_controller->end_x = stick_x;

      float stick_y = (gamepad->sThumbLY < 0)
                          ? static_cast<float>(gamepad->sThumbLY) / 32768.0f
                          : static_cast<float>(gamepad->sThumbLY) / 32767.0f;
      new_controller->start_y = old_controller->end_y;

      new_controller->min_y = stick_y;
      new_controller->max_y = stick_y;
      new_controller->end_y = stick_y;

      DWORD button_bits[] = {
          XINPUT_GAMEPAD_Y,
          XINPUT_GAMEPAD_A,
          XINPUT_GAMEPAD_X,
          XINPUT_GAMEPAD_B,
          XINPUT_GAMEPAD_LEFT_SHOULDER,
          XINPUT_GAMEPAD_RIGHT_SHOULDER,
      };

      Assert(ArraySize(button_bits) == ArraySize(old_controller->buttons));

      for (int i = 0; i < ArraySize(old_controller->buttons); ++i) {
        ProcessXInputDigitalButton(&old_controller->buttons[i],
                                   &new_controller->buttons[i],
                                   gamepad->wButtons, button_bits[i]);
      }

    } else {
      // TODO(bissakov): Controller is not available
    }
  }
}

static void SwapInputs(GameInput *old_input, GameInput *new_input) {
  GameInput *temp_input = new_input;
  new_input = old_input;
  old_input = temp_input;
}

static bool ClearBuffer(SoundOutput *sound_output) {
  void *region1;
  void *region2;
  DWORD region1_size;
  DWORD region2_size;

  HRESULT result =
      SOUND_BUFFER->Lock(0, sound_output->secondary_buffer_size, &region1,
                         &region1_size, &region2, &region2_size, 0);
  if (!SUCCEEDED(result)) {
    return false;
  }

  int8_t *dest_sample = reinterpret_cast<int8_t *>(region1);
  for (DWORD byte_idx = 0; byte_idx < region1_size; ++byte_idx) {
    *dest_sample++ = 0;
  }

  dest_sample = reinterpret_cast<int8_t *>(region2);
  for (DWORD byte_idx = 0; byte_idx < region2_size; ++byte_idx) {
    *dest_sample++ = 0;
  }

  if (!SUCCEEDED(
          SOUND_BUFFER->Unlock(region1, region1_size, region2, region2_size))) {
    return false;
  }

  return true;
}

static bool FillSoundBuffer(SoundOutput *sound_output, uint32_t byte_to_lock,
                            uint32_t bytes_to_write,
                            GameSoundBuffer *game_sound_buffer) {
  void *region1;
  void *region2;
  DWORD region1_size;
  DWORD region2_size;

  HRESULT result =
      SOUND_BUFFER->Lock(byte_to_lock, bytes_to_write, &region1, &region1_size,
                         &region2, &region2_size, 0);
  if (!SUCCEEDED(result)) {
    return false;
  }

  DWORD region1_sample_count = region1_size / sound_output->bytes_per_sample;
  int16_t *dest_sample = reinterpret_cast<int16_t *>(region1);
  int16_t *source_sample = game_sound_buffer->samples;
  for (DWORD i = 0; i < region1_sample_count; ++i) {
    *dest_sample++ = *source_sample++;
    *dest_sample++ = *source_sample++;
    ++sound_output->running_sample_idx;
  }

  DWORD region2_sample_count = region2_size / sound_output->bytes_per_sample;
  dest_sample = reinterpret_cast<int16_t *>(region2);
  for (DWORD i = 0; i < region2_sample_count; ++i) {
    *dest_sample++ = *source_sample++;
    *dest_sample++ = *source_sample++;
    ++sound_output->running_sample_idx;
  }

  SOUND_BUFFER->Unlock(region1, region1_size, region2, region2_size);

  return true;
}

int CALLBACK WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int) {
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

  if (!InitDirectSound(window, sound_output.samples_per_second,
                       sound_output.secondary_buffer_size)) {
    OutputDebugStringW(L"DirectSound initialization failed\n");
    return 1;
  }

  if (!ClearBuffer(&sound_output)) {
    return 1;
  }

  if (!SUCCEEDED(SOUND_BUFFER->Play(0, 0, DSBPLAY_LOOPING))) {
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
  LPVOID base_address = 0;
#else
  LPVOID base_address = (LPVOID)Terabytes((uint64_t)2);
#endif

  GameMemory memory = {};
  memory.permanent_storage_size = Megabytes(64);
  memory.transient_storage_size = Gigabytes((uint64_t)4);
  uint64_t total_memory_size =
      memory.permanent_storage_size + memory.transient_storage_size;

  memory.permanent_storage =
      VirtualAlloc(base_address, total_memory_size, MEM_RESERVE | MEM_COMMIT,
                   PAGE_READWRITE);
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
    MSG message;
    while (PeekMessageW(&message, 0, 0, 0, PM_REMOVE)) {
      if (message.message == WM_QUIT) {
        RUNNING = false;
        break;
      }
      TranslateMessage(&message);
      DispatchMessageW(&message);
    }

    HandleGamepad(&old_input, &new_input);

    DWORD byte_to_lock = 0;
    DWORD target_cursor = 0;
    DWORD play_cursor = 0;
    DWORD bytes_to_write = 0;
    DWORD write_cursor;
    bool sound_is_valid = false;
    if (SUCCEEDED(
            SOUND_BUFFER->GetCurrentPosition(&play_cursor, &write_cursor))) {
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
      FillSoundBuffer(&sound_output, byte_to_lock, bytes_to_write,
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
