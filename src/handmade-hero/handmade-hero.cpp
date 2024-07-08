// Copyright 2024 Alikhan Bissakov
// Handmade Hero
// File: handmade-hero.cpp
// ----------------------
// Handmade Hero License by Casey Muratori
// See the end of file for license information

#include "../../src/handmade-hero/handmade-hero.h"

#include <cmath>
#include <cstdint>

#define PI 3.14159265359f

static void Render(GameBuffer *buffer, int x_offset, int y_offset) {
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

static void OutputGameSound(GameSoundBuffer *sound_buffer) {
  static float t_sin;
  int16_t *samples = sound_buffer->samples;
  uint16_t tone_volume = 3000;

  int wave_period = sound_buffer->samples_per_second / sound_buffer->tone_hz;

  for (int i = 0; i < sound_buffer->sample_count; ++i) {
    float sin_value = sinf(t_sin);
    int16_t sample_value = (int16_t)(sin_value * tone_volume);
    *samples++ = sample_value;
    *samples++ = sample_value;

    t_sin += 2.0f * PI * 1.0f / static_cast<float>(wave_period);
  }
}

void UpdateAndRender(GameBuffer *buffer, GameSoundBuffer *sound_buffer,
                     GameInput *game_input) {
  static int x_offset = 0;
  static int y_offset = 0;

  ControllerInput input0 = game_input->controllers[0];

  if (input0.is_analog) {
    sound_buffer->tone_hz = 256 + (128 * input0.end_y);
    x_offset = 4 * input0.end_x;
  } else {
    // asdklasj
  }

  if (input0.a_button.ended_down) {
    y_offset += 10;
  }

  OutputGameSound(sound_buffer);
  Render(buffer, x_offset, y_offset);
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
