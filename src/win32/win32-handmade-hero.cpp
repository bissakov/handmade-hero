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
#include "../../src/win32/win32-clock.h"
#include "../../src/win32/win32-display.h"
#include "../../src/win32/win32-file-io.h"
#include "../../src/win32/win32-input.h"
#include "../../src/win32/win32-sound.h"

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

  LARGE_INTEGER perf_count_frequency_result;
  QueryPerformanceFrequency(&perf_count_frequency_result);
  perf_count_frequency = perf_count_frequency_result.QuadPart;

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

  int refresh_rate = GetDeviceCaps(device_context, VREFRESH);
  if (refresh_rate == 0) {
    OutputDebugStringW(L"Failed to get the refresh rate\n");
    return 1;
  }

  int default_fps = 60;
  int target_fps = (refresh_rate >= default_fps) ? default_fps : refresh_rate;
  float target_sec_per_frame = 1.0f / target_fps;

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

  LARGE_INTEGER last_counter = GetWallClock();
  uint64_t last_cycle_count = __rdtsc();

  while (RUNNING) {
    ControllerInput *old_keyboard_controller = GetController(&old_input, 0);
    ControllerInput *new_keyboard_controller = GetController(&new_input, 0);
    *new_keyboard_controller = {};

    new_keyboard_controller->is_connected = true;

    for (int i = 0; i < ArraySize(new_keyboard_controller->buttons); ++i) {
      new_keyboard_controller->buttons[i].ended_down =
          old_keyboard_controller->buttons[i].ended_down;
    }

    if (!ProcessPendingMessages(new_keyboard_controller)) {
      break;
    }
    HandleGamepad(&old_input, &new_input);
    SwapInputs(&old_input, &new_input);

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

    LARGE_INTEGER work_counter = GetWallClock();

    float elapsed_sec_per_frame_work =
        GetSecondsElapsed(last_counter, work_counter, perf_count_frequency);
    float elapsed_sec_per_frame = elapsed_sec_per_frame_work;

    if (elapsed_sec_per_frame > target_sec_per_frame) {
      // missed frame
    } else {
      while (elapsed_sec_per_frame < target_sec_per_frame) {
        work_counter = GetWallClock();
        elapsed_sec_per_frame =
            GetSecondsElapsed(last_counter, work_counter, perf_count_frequency);
      }
    }

    float counters_elsaped = GetCountersElapsed(last_counter, work_counter);
    float ms_per_frame = static_cast<float>(1000.0f * counters_elsaped) /
                         static_cast<float>(perf_count_frequency);
    float fps = static_cast<float>(perf_count_frequency) / counters_elsaped;

    char debug_buffer[256];
    snprintf(debug_buffer, sizeof(debug_buffer), "%.02f ms/f\t%.02f fps\n",
             ms_per_frame, fps);
    OutputDebugStringA(debug_buffer);

    work_counter = GetWallClock();
    last_counter = work_counter;

    uint64_t end_cycle_count = __rdtsc();
    // float cycles_elapsed =
    //     static_cast<float>((end_cycle_count - last_cycle_count) /
    //     1'000'000.0f);
    last_cycle_count = end_cycle_count;
  }

  ReleaseDC(window, device_context);

  return 0;
}
