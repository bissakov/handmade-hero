#ifndef SRC_HANDMADE_HERO_HANDMADE_HERO_H_
#define SRC_HANDMADE_HERO_HANDMADE_HERO_H_

#include <cstdint>

#include "../../src/win32/win32-handmade-hero.h"

#define ArraySize(arr) (sizeof(arr) / sizeof((arr)[0]))
#if DEBUG
#define Assert(expression)              \
  if (!static_cast<bool>(expression)) { \
    __debugbreak();                     \
  }
#else
#define Assert(expression)
#endif

#define Kilobytes(value) ((value) * 1024)
#define Megabytes(value) (Kilobytes(value) * 1024)
#define Gigabytes(value) (Megabytes(value) * 1024)
#define Terabytes(value) (Gigabytes(value) * 1024)

struct GameMemory {
  bool is_init;
  uint64_t permanent_storage_size;
  void *permanent_storage;

  uint64_t transient_storage_size;
  void *transient_storage;
};

struct GameState {
  float t_sin;
  int tone_hz;
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
  bool is_connected;
  bool is_analog;
  float stick_avg_x;
  float stick_avg_y;

  union {
    ButtonState buttons[12];
    struct {
      ButtonState move_up;
      ButtonState move_down;
      ButtonState move_left;
      ButtonState move_right;

      ButtonState action_up;
      ButtonState action_down;
      ButtonState action_left;
      ButtonState action_right;

      ButtonState left_shoulder;
      ButtonState right_shoulder;

      ButtonState start_button;
      ButtonState back_button;
    };
  };
};

struct GameInput {
  ControllerInput controllers[5];
};

ControllerInput *GetController(GameInput *input, int controller_idx);

static inline void Render(GameBuffer *buffer, GameState *state);

static inline void OutputGameSound(GameSoundBuffer *sound_buffer,
                                   GameState *state);

void UpdateAndRender(GameMemory *memory, GameBuffer *buffer,
                     GameSoundBuffer *sound_buffer, GameInput *input);

#endif  // SRC_HANDMADE_HERO_HANDMADE_HERO_H_
