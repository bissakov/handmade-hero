
#include <windows.h>
#include <xinput.h>

#include "../../src/handmade-hero/handmade-hero.h"

#ifndef WIN32_INPUT_H_

typedef DWORD WINAPI XInputGetStateT(DWORD controller_idx,
                                     XINPUT_STATE *controller_state);
typedef DWORD WINAPI XInputSetStateT(DWORD controller_idx,
                                     XINPUT_VIBRATION *vibration);

bool InitXInput();
bool ProcessPendingMessages(ControllerInput *keyboard_controller);
static inline void ProcessXInputDigitalButton(ButtonState *old_state,
                                              ButtonState *new_state,
                                              DWORD xinput_button_state,
                                              DWORD button_bit);
static inline void ProcessKeyboardMessage(ButtonState *new_state,
                                          bool is_key_down,
                                          uint32_t button_code);
static inline void HandleKeyboard(ControllerInput *keyboard_controller,
                                  uint32_t vk_code, bool is_key_down);
static inline float ProcessXInputStickPosition(SHORT raw_stick_value,
                                               SHORT deadzone);
void HandleGamepad(GameInput *old_input, GameInput *new_input);
void SwapInputs(GameInput *old_input, GameInput *new_input);

#define WIN32_INPUT_H_
#endif  // WIN32_INPUT_H_
