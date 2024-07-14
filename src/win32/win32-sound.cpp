#include "../../src/win32/win32-sound.h"

#include <dsound.h>
#include <windows.h>

#include "../../src/handmade-hero/handmade-hero.h"

IDirectSoundBuffer *InitDirectSound(HWND window, int samples_per_second,
                                    int buffer_size) {
  IDirectSoundBuffer *sound_buffer = 0;

  HMODULE direct_sound_lib = LoadLibraryW(L"dsound.dll");
  if (!direct_sound_lib) {
    return 0;
  }

  DirectSoundCreateT *DyDirectSoundCreate =
      reinterpret_cast<DirectSoundCreateT *>(
          GetProcAddress(direct_sound_lib, "DirectSoundCreate"));

  FreeLibrary(direct_sound_lib);

  IDirectSound *direct_sound;
  if (!DyDirectSoundCreate ||
      !SUCCEEDED(DyDirectSoundCreate(0, &direct_sound, 0))) {
    return 0;
  }

  if (!SUCCEEDED(direct_sound->SetCooperativeLevel(window, DSSCL_PRIORITY))) {
    return 0;
  }

  IDirectSoundBuffer *primary_buffer;
  {
    DSBUFFERDESC buffer_desc = {};
    buffer_desc.dwSize = sizeof(buffer_desc);
    buffer_desc.dwFlags = DSBCAPS_PRIMARYBUFFER;

    if (!SUCCEEDED(direct_sound->CreateSoundBuffer(&buffer_desc,
                                                   &primary_buffer, 0))) {
      return 0;
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
    return 0;
  }

  {
    DSBUFFERDESC buffer_desc = {};
    buffer_desc.dwSize = sizeof(buffer_desc);
    buffer_desc.dwFlags = DSBCAPS_GETCURRENTPOSITION2;
    buffer_desc.dwBufferBytes = buffer_size;
    buffer_desc.lpwfxFormat = &wave_format;
    if (!SUCCEEDED(
            direct_sound->CreateSoundBuffer(&buffer_desc, &sound_buffer, 0))) {
      return 0;
    }
  }

  return sound_buffer;
}

bool ClearBuffer(IDirectSoundBuffer *sound_buffer, SoundOutput *sound_output) {
  void *region1;
  void *region2;
  DWORD region1_size;
  DWORD region2_size;

  HRESULT result =
      sound_buffer->Lock(0, sound_output->secondary_buffer_size, &region1,
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
          sound_buffer->Unlock(region1, region1_size, region2, region2_size))) {
    return false;
  }

  return true;
}

bool FillSoundBuffer(IDirectSoundBuffer *sound_buffer,
                     SoundOutput *sound_output, uint32_t byte_to_lock,
                     uint32_t bytes_to_write,
                     GameSoundBuffer *game_sound_buffer) {
  void *region1;
  void *region2;
  DWORD region1_size;
  DWORD region2_size;

  HRESULT result =
      sound_buffer->Lock(byte_to_lock, bytes_to_write, &region1, &region1_size,
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

  sound_buffer->Unlock(region1, region1_size, region2, region2_size);

  return true;
}
