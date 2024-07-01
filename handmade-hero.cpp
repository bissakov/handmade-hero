// Copyright 2024 Alikhan Bissakov
// Handmade Hero
// File: handmade-hero.cpp
// ----------------------
// Handmade Hero License by Casey Muratori
// See the end of file for license information

#include <dsound.h>
#include <windows.h>
#include <winerror.h>
#include <xinput.h>

#include <cassert>
#include <cstdint>

typedef DWORD WINAPI XInputGetStateT(DWORD controller_idx,
                                     XINPUT_STATE *controller_state);
static XInputGetStateT *DyXInputGetState;

typedef DWORD WINAPI XInputSetStateT(DWORD controller_idx,
                                     XINPUT_VIBRATION *vibration);
static XInputSetStateT *DyXInputSetState;

bool InitXInput() {
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

typedef HRESULT WINAPI DirectSoundCreateT(LPGUID lpGuid, LPDIRECTSOUND *ppDS,
                                          LPUNKNOWN pUnkOuter);

static bool InitDirectSound(HWND window, int samples_per_second,
                            int buffer_size) {
  HMODULE direct_sound_lib = LoadLibraryW(L"dsound.dll");
  if (!direct_sound_lib) {
    return false;
  } else {
    OutputDebugStringW(L"dsound.dll loaded\n");
  }

  DirectSoundCreateT *DyDirectSoundCreate =
      reinterpret_cast<DirectSoundCreateT *>(
          GetProcAddress(direct_sound_lib, "DirectSoundCreate"));

  FreeLibrary(direct_sound_lib);

  IDirectSound *direct_sound;
  if (!DyDirectSoundCreate ||
      !SUCCEEDED(DyDirectSoundCreate(0, &direct_sound, 0))) {
    return false;
  } else {
    OutputDebugStringW(L"DirectSoundCreate success\n");
  }

  if (!SUCCEEDED(direct_sound->SetCooperativeLevel(window, DSSCL_PRIORITY))) {
    return false;
  } else {
    OutputDebugStringW(L"SetCooperativeLevel success\n");
  }

  LPDIRECTSOUNDBUFFER primary_buffer;
  {
    DSBUFFERDESC buffer_desc = {};
    buffer_desc.dwSize = sizeof(buffer_desc);
    buffer_desc.dwFlags = DSBCAPS_PRIMARYBUFFER;

    if (!SUCCEEDED(direct_sound->CreateSoundBuffer(&buffer_desc,
                                                   &primary_buffer, 0))) {
      return false;
    } else {
      OutputDebugStringW(L"CreateSoundBuffer primary_buffer success\n");
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
  } else {
    OutputDebugStringW(L"SetFormat success\n");
  }

  LPDIRECTSOUNDBUFFER secondary_buffer;
  {
    DSBUFFERDESC buffer_desc = {};
    buffer_desc.dwSize = sizeof(buffer_desc);
    buffer_desc.dwFlags = 0;
    buffer_desc.dwBufferBytes = buffer_size;
    buffer_desc.lpwfxFormat = &wave_format;
    if (!SUCCEEDED(direct_sound->CreateSoundBuffer(&buffer_desc,
                                                   &secondary_buffer, 0))) {
      return false;
    } else {
      OutputDebugStringW(L"CreateSoundBuffer secondary_buffer success\n");
    }
  }

  return true;
}

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

static void Render(Buffer *buffer, int x_offset, int y_offset) {
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
  buffer->memory = VirtualAlloc(0, bitmap_memory_size, MEM_RESERVE | MEM_COMMIT,
                                PAGE_READWRITE);

  buffer->pitch = buffer->width * buffer->bytes_per_pixel;
}

static void DisplayBuffer(HDC device_context, int window_x, int window_y,
                          uint16_t window_width, uint16_t window_height,
                          Buffer *buffer) {
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
                    window_dimensions.height, &buffer);

      EndPaint(window, &paint);

      break;
    }

    case WM_SYSKEYDOWN: {
      uint32_t vk_code = w_param;

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
      uint32_t vk_code = w_param;
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

static void HandleGamepad(int *x_offset, int *y_offset) {
  if (!DyXInputGetState || !DyXInputSetState) {
    return;
  }

  // XINPUT_VIBRATION vibration;
  for (int controller_idx = 0; controller_idx < XUSER_MAX_COUNT;
       ++controller_idx) {
    XINPUT_STATE controller_state;
    if (DyXInputGetState(controller_idx, &controller_state) == ERROR_SUCCESS) {
      // vibration.wLeftMotorSpeed = 0;
      // vibration.wRightMotorSpeed = 0;

      XINPUT_GAMEPAD *gamepad = &controller_state.Gamepad;

      bool up = gamepad->wButtons & XINPUT_GAMEPAD_DPAD_UP;
      bool down = gamepad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN;
      bool left = gamepad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT;
      bool right = gamepad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT;

      if (up) {
        *y_offset += 10;
      }
      if (down) {
        *y_offset -= 10;
      }
      if (left) {
        *x_offset += 10;
      }
      if (right) {
        *x_offset -= 10;
      }

      *x_offset -= gamepad->sThumbLX / 4096;
      *y_offset += gamepad->sThumbLY / 4096;

      // vibration.wLeftMotorSpeed = 65535;
      // DyXInputSetState(controller_idx, &vibration);
    } else {
      // TODO(bissakov): Controller is not available
    }
  }
}

int CALLBACK WinMain(HINSTANCE instance, HINSTANCE prev_instance,
                     LPSTR cmd_line, int show_code) {
  if (!InitXInput()) {
    return ERROR_DEVICE_NOT_CONNECTED;
  }

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

  if (!InitDirectSound(window, 48000, 48000 * sizeof(int16_t) * 2)) {
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

    HandleGamepad(&x_offset, &y_offset);

    Render(&buffer, x_offset, y_offset);

    Dimensions window_dimensions = GetDimensions(window);
    DisplayBuffer(device_context, 0, 0, window_dimensions.width,
                  window_dimensions.height, &buffer);
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
