#ifndef HANDMADE_HERO_H_

#include <cstdint>

#define ArraySize(arr) (sizeof(arr) / sizeof((arr)[0]))

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
    ButtonState buttons[14];
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

inline ControllerInput *GetController(GameInput *input, int controller_idx) {
  Assert(controller_idx < ArraySize(input->controllers));
}

static inline void Render(GameBuffer *buffer, GameState *state);

static inline void OutputGameSound(GameSoundBuffer *sound_buffer,
                                   GameState *state);

void UpdateAndRender(GameMemory *memory, GameBuffer *buffer,
                     GameSoundBuffer *sound_buffer, GameInput *input);

#define HANDMADE_HERO_H_
#endif  // HANDMADE_HERO_H_
