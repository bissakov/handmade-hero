#include "../../src/win32/win32-input.h"

#include <windows.h>
#include <xinput.h>

#include "../../src/win32/win32-handmade-hero.h"

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

void ProcessXInputDigitalButton(ButtonState *old_state, ButtonState *new_state,
                                DWORD xinput_button_state, DWORD button_bit) {
  if (!(xinput_button_state & button_bit)) {
    return;
  }
  new_state->ended_down = (xinput_button_state & button_bit) == button_bit;
  new_state->half_transition_count =
      (old_state->ended_down != new_state->ended_down) ? 1 : 0;
}

void ProcessKeyboardMessage(ButtonState *new_state, bool is_key_down,
                            uint32_t button_code) {
  new_state->ended_down = is_key_down;
  ++new_state->half_transition_count;
}

void HandleKeyboard(ControllerInput *keyboard_controller, uint32_t vk_code,
                    bool is_key_down) {
  switch (vk_code) {
    case VK_UP: {
      ProcessKeyboardMessage(&keyboard_controller->up_button, is_key_down,
                             vk_code);
      break;
    }
    case VK_DOWN: {
      ProcessKeyboardMessage(&keyboard_controller->down_button, is_key_down,
                             vk_code);
      break;
    }
    case VK_LEFT: {
      ProcessKeyboardMessage(&keyboard_controller->left_button, is_key_down,
                             vk_code);
      break;
    }
    case VK_RIGHT: {
      ProcessKeyboardMessage(&keyboard_controller->right_button, is_key_down,
                             vk_code);
      break;
    }
    case 'Z': {
      ProcessKeyboardMessage(&keyboard_controller->y_button, is_key_down,
                             vk_code);
      break;
    }
    case 'X': {
      ProcessKeyboardMessage(&keyboard_controller->a_button, is_key_down,
                             vk_code);
      break;
    }
    case 'C': {
      ProcessKeyboardMessage(&keyboard_controller->x_button, is_key_down,
                             vk_code);
      break;
    }
    case 'V': {
      ProcessKeyboardMessage(&keyboard_controller->b_button, is_key_down,
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

void HandleGamepad(GameInput *old_input, GameInput *new_input) {
  if (!DyXInputGetState || !DyXInputSetState) {
    return;
  }

  int max_supported_controller_count = ArraySize(old_input->controllers);
  int max_controller_count = (XUSER_MAX_COUNT > max_supported_controller_count)
                                 ? max_supported_controller_count
                                 : XUSER_MAX_COUNT;

  for (int controller_idx = 0; controller_idx < max_controller_count;
       ++controller_idx) {
    ControllerInput *old_controller = &old_input->controllers[controller_idx];
    ControllerInput *new_controller = &new_input->controllers[controller_idx];

    old_controller->is_analog = true;
    new_controller->is_analog = true;

    XINPUT_STATE controller_state;
    if (DyXInputGetState(controller_idx, &controller_state) == ERROR_SUCCESS) {
      XINPUT_GAMEPAD *gamepad = &controller_state.Gamepad;

      float stick_x = (gamepad->sThumbLX < 0)
                          ? static_cast<float>(gamepad->sThumbLX) / 32768.0f
                          : static_cast<float>(gamepad->sThumbLX) / 32767.0f;
      new_controller->start_x = old_controller->end_x;

      new_controller->min_x = stick_x;
      new_controller->max_x = stick_x;
      new_controller->end_x = stick_x;

      float stick_y = (gamepad->sThumbLY < 0)
                          ? static_cast<float>(gamepad->sThumbLY) / 32768.0f
                          : static_cast<float>(gamepad->sThumbLY) / 32767.0f;
      new_controller->start_y = old_controller->end_y;

      new_controller->min_y = stick_y;
      new_controller->max_y = stick_y;
      new_controller->end_y = stick_y;

      DWORD button_bits[] = {
          XINPUT_GAMEPAD_DPAD_UP,
          XINPUT_GAMEPAD_DPAD_DOWN,
          XINPUT_GAMEPAD_DPAD_LEFT,
          XINPUT_GAMEPAD_DPAD_RIGHT,
          XINPUT_GAMEPAD_Y,
          XINPUT_GAMEPAD_A,
          XINPUT_GAMEPAD_X,
          XINPUT_GAMEPAD_B,
          XINPUT_GAMEPAD_LEFT_SHOULDER,
          XINPUT_GAMEPAD_RIGHT_SHOULDER,
      };

      Assert(ArraySize(button_bits) == ArraySize(old_controller->buttons));

      for (int i = 0; i < ArraySize(old_controller->buttons); ++i) {
        ProcessXInputDigitalButton(&old_controller->buttons[i],
                                   &new_controller->buttons[i],
                                   gamepad->wButtons, button_bits[i]);
      }

    } else {
      // TODO(bissakov): Controller is not available
    }
  }
}

void SwapInputs(GameInput *old_input, GameInput *new_input) {
  GameInput temp_input = *new_input;
  *new_input = *old_input;
  *old_input = temp_input;
}
