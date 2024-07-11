#include "../../src/handmade-hero/handmade-hero.h"

#include <cmath>
#include <cstdint>

#define PI 3.14159265359f

static void Render(GameBuffer *buffer, GameState *state) {
  uint8_t *row = reinterpret_cast<uint8_t *>(buffer->memory);
  for (int y = 0; y < buffer->height; ++y) {
    uint32_t *pixel = reinterpret_cast<uint32_t *>(row);
    for (int x = 0; x < buffer->width; ++x) {
      uint8_t red = static_cast<uint8_t>(x + state->x_offset);
      uint8_t green = 0;
      uint8_t blue = static_cast<uint8_t>(y + state->y_offset);
      *pixel++ = (red << 16) | (green << 8) | blue;
    }
    row += buffer->pitch;
  }
}

static void OutputGameSound(GameSoundBuffer *sound_buffer, GameState *state) {
  int16_t *samples = sound_buffer->samples;
  uint16_t tone_volume = 3000;

  sound_buffer->wave_period =
      static_cast<float>(sound_buffer->samples_per_second) / state->tone_hz;

  for (int i = 0; i < sound_buffer->sample_count; ++i) {
    float sin_value = sinf(state->t_sin);
    int16_t sample_value = (int16_t)(sin_value * tone_volume);
    *samples++ = sample_value;
    *samples++ = sample_value;

    state->t_sin += 2.0f * PI * 1.0f / sound_buffer->wave_period;
  }
}

void UpdateAndRender(GameMemory *memory, GameBuffer *buffer,
                     GameSoundBuffer *sound_buffer, GameInput *input) {
  GameState *state = static_cast<GameState *>(memory->permanent_storage);
  if (!memory->is_init) {
    state->t_sin = 0.0f;
    state->tone_hz = 256;
    memory->is_init = true;
  }

  for (int i = 0; i < ArraySize(input->controllers); ++i) {
    ControllerInput *controller = &input->controllers[i];

    if (controller->is_analog && controller->is_connected) {
      state->tone_hz = 256 + static_cast<int>(128 * controller->stick_avg_x);
      state->x_offset -= 10 * static_cast<int>(controller->stick_avg_x);
      state->y_offset += 10 * static_cast<int>(controller->stick_avg_y);
    } else {
      if (controller->move_left.ended_down) {
        state->tone_hz = 256 - 128;
        state->x_offset -= 10;
      }
      if (controller->move_right.ended_down) {
        state->tone_hz = 256 + 128;
        state->x_offset += 10;
      }
    }

    if (controller->action_down.ended_down) {
      state->y_offset += 10;
    }
  }

  OutputGameSound(sound_buffer, state);
  Render(buffer, state);
}
