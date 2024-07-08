// Copyright 2024 Alikhan Bissakov
// Handmade Hero
// File: handmade-hero.cpp
// ----------------------
// Handmade Hero License by Casey Muratori
// See the end of file for license information

#include <cstdint>

#ifndef HANDMADE_HERO_H_

#define ArraySize(arr) (sizeof(arr) / sizeof((arr)[0]))

#define Kilobytes(value) ((value) * 1024)
#define Megabytes(value) (Kilobytes(value) * 1024)
#define Gigabytes(value) (Megabytes(value) * 1024)
#define Terabytes(value) (Gigabytes(value) * 1024)

#define DEBUG 1

#if DEBUG
#define Assert(expression)           \
  if (!(expression)) {               \
    *reinterpret_cast<int *>(0) = 0; \
  }
#else
#define Assert(expression)
#endif

struct GameMemory {
  bool is_init;
  uint64_t permanent_storage_size;
  void *permanent_storage;

  uint64_t transient_storage_size;
  void *transient_storage;
};

struct GameState {
  float tone_hz;
  int x_offset = 0;
  int y_offset = 0;
};

struct GameBuffer {
  void *memory;
  int width;
  int height;
  int pitch;
  int bytes_per_pixel;
};

struct GameSoundBuffer {
  int samples_per_second;
  int sample_count;
  float wave_period;
  int16_t *samples;
};

struct ButtonState {
  int half_transition_count;
  bool ended_down;
};

struct ControllerInput {
  bool is_analog;

  float start_x;
  float start_y;

  float min_x;
  float min_y;

  float max_x;
  float max_y;

  float end_x;
  float end_y;

  union {
    ButtonState buttons[6];
    struct {
      ButtonState y_button;
      ButtonState a_button;
      ButtonState x_button;
      ButtonState b_button;
      ButtonState left_shoulder;
      ButtonState right_shoulder;
    };
  };
};

struct GameInput {
  ControllerInput controllers[4];
};

static inline void Render(GameBuffer *buffer, int x_offset, int y_offset);

static inline void OutputGameSound(GameSoundBuffer *sound_buffer);

void UpdateAndRender(GameMemory *memory, GameBuffer *buffer,
                     GameSoundBuffer *sound_buffer, GameInput *input);

#define HANDMADE_HERO_H_
#endif  // HANDMADE_HERO_H_

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
