#include "../../src/win32/win32-input.h"

#include <windows.h>
#include <xinput.h>

static XInputGetStateT *DyXInputGetState;
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

bool ProcessPendingMessages(ControllerInput *keyboard_controller) {
  bool result = true;

  MSG message;
  while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
    uint32_t vk_code = (uint32_t)message.wParam;
    int32_t l_param = (int32_t)message.lParam;

    uint32_t was_key_down_bit = (l_param & (1U << 30U)) >> 30U;
    uint32_t is_key_down_bit = (l_param & (1U << 31U)) >> 31U;
    bool was_key_down = was_key_down_bit != 0;
    bool is_key_down = is_key_down_bit == 0;

    switch (message.message) {
      case WM_SYSKEYDOWN: {
        bool alt_key_down =
            (static_cast<uint32_t>(message.lParam) & (1U << 29)) != 0;
        if ((vk_code == VK_F4) && alt_key_down) {
          result = false;
          break;
        }

        break;
      }
      case WM_SYSKEYUP:
      case WM_KEYDOWN:
      case WM_KEYUP: {
        if (vk_code == VK_ESCAPE) {
          result = false;
          break;
        }

        if (was_key_down != is_key_down) {
          HandleKeyboard(keyboard_controller, vk_code, is_key_down);
        }

        break;
      }

      default: {
        TranslateMessage(&message);
        DispatchMessageW(&message);
      }
    }
  }

  return result;
}

static inline void ProcessXInputDigitalButton(ButtonState *old_state,
                                              ButtonState *new_state,
                                              DWORD xinput_button_state,
                                              DWORD button_bit) {
  new_state->ended_down = (xinput_button_state & button_bit) == button_bit;
  new_state->half_transition_count =
      (old_state->ended_down != new_state->ended_down) ? 1 : 0;
}

static inline void ProcessKeyboardMessage(ButtonState *new_state,
                                          bool is_key_down,
                                          uint32_t button_code) {
  Assert(new_state->ended_down != is_key_down);
  new_state->ended_down = is_key_down;
  ++new_state->half_transition_count;
}

static inline void HandleKeyboard(ControllerInput *keyboard_controller,
                                  uint32_t vk_code, bool is_key_down) {
  switch (vk_code) {
    case 'W': {
      ProcessKeyboardMessage(&keyboard_controller->move_up, is_key_down,
                             vk_code);
      break;
    }
    case 'S': {
      ProcessKeyboardMessage(&keyboard_controller->move_down, is_key_down,
                             vk_code);
      break;
    }
    case 'A': {
      ProcessKeyboardMessage(&keyboard_controller->move_left, is_key_down,
                             vk_code);
      break;
    }
    case 'D': {
      ProcessKeyboardMessage(&keyboard_controller->move_right, is_key_down,
                             vk_code);
      break;
    }
    case VK_UP: {
      ProcessKeyboardMessage(&keyboard_controller->action_up, is_key_down,
                             vk_code);
      break;
    }
    case VK_DOWN: {
      ProcessKeyboardMessage(&keyboard_controller->action_down, is_key_down,
                             vk_code);
      break;
    }
    case VK_LEFT: {
      ProcessKeyboardMessage(&keyboard_controller->action_left, is_key_down,
                             vk_code);
      break;
    }
    case VK_RIGHT: {
      ProcessKeyboardMessage(&keyboard_controller->action_right, is_key_down,
                             vk_code);
      break;
    }
    case 'Q': {
      ProcessKeyboardMessage(&keyboard_controller->left_shoulder, is_key_down,
                             vk_code);
      break;
    }
    case 'E': {
      ProcessKeyboardMessage(&keyboard_controller->right_shoulder, is_key_down,
                             vk_code);
      break;
    }
    default: {
      break;
    }
  }
}

static inline float ProcessXInputStickPosition(SHORT raw_stick_value,
                                               SHORT deadzone) {
  float stick_value;
  if (raw_stick_value < -deadzone) {
    stick_value =
        static_cast<float>(raw_stick_value + deadzone) / (32768.0f - deadzone);
  } else if (raw_stick_value > deadzone) {
    stick_value =
        static_cast<float>(raw_stick_value + deadzone) / (32767.0f - deadzone);
  } else {
    stick_value = 0.0f;
  }
  return stick_value;
}

void HandleGamepad(GameInput *old_input, GameInput *new_input) {
  if (!DyXInputGetState || !DyXInputSetState) {
    return;
  }

  int max_supported_controller_count = ArraySize(old_input->controllers) - 1;
  int max_controller_count = (XUSER_MAX_COUNT > max_supported_controller_count)
                                 ? max_supported_controller_count
                                 : XUSER_MAX_COUNT;

  for (int controller_idx = 0; controller_idx < max_controller_count;
       ++controller_idx) {
    ControllerInput *old_controller =
        GetController(old_input, controller_idx + 1);
    ControllerInput *new_controller =
        GetController(new_input, controller_idx + 1);

    old_controller->is_analog = true;
    new_controller->is_analog = true;

    XINPUT_STATE controller_state;
    if (DyXInputGetState(controller_idx, &controller_state) != ERROR_SUCCESS) {
      new_controller->is_connected = false;
      continue;
    }

    new_controller->is_connected = true;

    XINPUT_GAMEPAD *gamepad = &controller_state.Gamepad;

    new_controller->stick_avg_x = ProcessXInputStickPosition(
        gamepad->sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
    new_controller->stick_avg_y = ProcessXInputStickPosition(
        gamepad->sThumbLY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);

    if (new_controller->stick_avg_x != 0.0f ||
        new_controller->stick_avg_y != 0.0f) {
      new_controller->is_analog = true;
    }

    if (gamepad->wButtons & XINPUT_GAMEPAD_DPAD_UP) {
      new_controller->stick_avg_y = 1.0f;
      new_controller->is_analog = false;
    }
    if (gamepad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN) {
      new_controller->stick_avg_y = -1.0f;
      new_controller->is_analog = false;
    }
    if (gamepad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT) {
      new_controller->stick_avg_x = -1.0f;
      new_controller->is_analog = false;
    }
    if (gamepad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) {
      new_controller->stick_avg_x = 1.0f;
      new_controller->is_analog = false;
    }

    float threshold = 0.5f;
    ProcessXInputDigitalButton(
        &old_controller->move_up, &new_controller->move_up,
        ((new_controller->stick_avg_x > threshold) ? 1 : 0), 1);
    ProcessXInputDigitalButton(
        &old_controller->move_down, &new_controller->move_down,
        ((new_controller->stick_avg_x < -threshold) ? 1 : 0), 1);
    ProcessXInputDigitalButton(
        &old_controller->move_left, &new_controller->move_left,
        ((new_controller->stick_avg_x < -threshold) ? 1 : 0), 1);
    ProcessXInputDigitalButton(
        &old_controller->move_right, &new_controller->move_right,
        ((new_controller->stick_avg_x > threshold) ? 1 : 0), 1);

    ProcessXInputDigitalButton(&old_controller->action_up,
                               &new_controller->action_up, gamepad->wButtons,
                               XINPUT_GAMEPAD_Y);
    ProcessXInputDigitalButton(&old_controller->action_down,
                               &new_controller->action_down, gamepad->wButtons,
                               XINPUT_GAMEPAD_A);
    ProcessXInputDigitalButton(&old_controller->action_left,
                               &new_controller->action_left, gamepad->wButtons,
                               XINPUT_GAMEPAD_X);
    ProcessXInputDigitalButton(&old_controller->action_right,
                               &new_controller->action_right, gamepad->wButtons,
                               XINPUT_GAMEPAD_B);
    ProcessXInputDigitalButton(&old_controller->left_shoulder,
                               &new_controller->left_shoulder,
                               gamepad->wButtons, XINPUT_GAMEPAD_LEFT_SHOULDER);
    ProcessXInputDigitalButton(&old_controller->right_shoulder,
                               &new_controller->right_shoulder,
                               gamepad->wButtons, XINPUT_GAMEPAD_LEFT_SHOULDER);
    ProcessXInputDigitalButton(&old_controller->start_button,
                               &new_controller->start_button, gamepad->wButtons,
                               XINPUT_GAMEPAD_START);
    ProcessXInputDigitalButton(&old_controller->back_button,
                               &new_controller->back_button, gamepad->wButtons,
                               XINPUT_GAMEPAD_BACK);
  }
}

void SwapInputs(GameInput *old_input, GameInput *new_input) {
  GameInput temp_input = *new_input;
  *new_input = *old_input;
  *old_input = temp_input;
}
