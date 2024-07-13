#ifndef SRC_WIN32_WIN32_SOUND_H_
#define SRC_WIN32_WIN32_SOUND_H_

#include <dsound.h>
#include <windows.h>

#include <cstdint>

#include "../../src/handmade-hero/handmade-hero.h"

typedef HRESULT WINAPI DirectSoundCreateT(LPGUID lpGuid, LPDIRECTSOUND *ppDS,
                                          LPUNKNOWN pUnkOuter);

struct SoundOutput {
  int samples_per_second = 48000;
  uint32_t running_sample_idx = 0;
  int bytes_per_sample = sizeof(int16_t) * 2;
  uint16_t tone_volume = 1000;
  int secondary_buffer_size = 0;
  float t_sin = 0.0f;
  int latency_sample_count = 0;
};

IDirectSoundBuffer *InitDirectSound(HWND window, int samples_per_second,
                                    int buffer_size);
bool ClearBuffer(IDirectSoundBuffer *sound_buffer, SoundOutput *sound_output);
bool FillSoundBuffer(IDirectSoundBuffer *sound_buffer,
                     SoundOutput *sound_output, uint32_t byte_to_lock,
                     uint32_t bytes_to_write,
                     GameSoundBuffer *game_sound_buffer);

#endif  // SRC_WIN32_WIN32_SOUND_H_
