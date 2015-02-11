/*  =====================================================================
    $File: x_input_wrapper.cpp
    $Creation Date: 2015-01-08
    $Last Modified: $
    $Revision: $
    $Creator: Cristián Donoso $
    $Notice: (c) Copyright 2015 Cristián Donoso $
    =====================================================================

    Provides a wrapper to x input.

    ===================================================================== */

#ifndef _WIN32_X_INPUT_WRAPPER_INCLUDED

#include "common_types.h"

#include <windows.h>
#include <xinput.h>

/**
 * Struct that contains all the booleans and values for the gamepad
 * state. This is basibally a simple XINPUT_GAMEPAD wrapper so we
 * don't need to and with the XINPUT_GAMEPAD state flags.
 */
struct x_input_gamepad_state
{
  // BUTTONS
  bool32 up;
  bool32 down;
  bool32 left;
  bool32 right;
  int16 leftThumbX;
  bool32 start;
  bool32 back;
  bool32 leftThumb;
  bool32 rightThumb;
  bool32 leftShoulder;
  bool32 rightShoulder;
  bool32 aButton;
  bool32 bButton;
  bool32 yButton;
  bool32 xButton;

  // THUMBS
  int16 leftThumbY;
  uint8 a;
  int16 rightThumbX;
  int16 rightThumbY;

  // TRIGGERS
  int16 leftTrigger;
  int16 rightTrigger;
};

/**
 * We make a windows binding ourselves. We this we bypass an import librabry.
 * We do this in the case of XInput because the required library changes according to
 * the version of Windows the user is using, so we would like the game to function
 * independent of the dll that's really required.
 *
 * What we do is we define types that serve as pointers to the functions we are trying
 * to override. The we define global variables of such types, so we have global pointers
 * that point to the functions in the Windows API.
 * Then we define there global variables as the real meaning of XInput methods, so we
 * never use the ones defined in xinput.h
 */
// We define a macro that generates the methods definition
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
// We generate the types for the pointers of the methods
typedef X_INPUT_GET_STATE(x_input_get_state);
typedef X_INPUT_SET_STATE(x_input_set_state);
// We generate stub functions for those methods
X_INPUT_GET_STATE(XInputGetStateStub) { return(ERROR_DEVICE_NOT_CONNECTED); }
X_INPUT_SET_STATE(XInputSetStateStub) { return(ERROR_DEVICE_NOT_CONNECTED); }
// We generate global pointers to the functions and at first we assign them
// to the harmless stubs. This way the game won't crash if the dll is not loaded.
global_variable x_input_get_state *DynamicXInputGetState = XInputGetStateStub;
global_variable x_input_set_state *DynamicXInputSetState = XInputSetStateStub;
#define XInputGetState DynamicXInputGetState
#define XInputSetState DynamicXInputSetState

/**
 * Tries to load the XInput libraries
 */
internal void
Win32LoadXInput()
{
  // We try to load 1.4 first (Windows 8), and then we try 1.3
  HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");
  if(!XInputLibrary) { LoadLibraryA("xinput9_1_0.dll"); }
  if(!XInputLibrary) { LoadLibraryA("xinput1_3.dll"); }
  if(!XInputLibrary) { return; } // TODO(Cristián): Diagnostics

  // Here we make a 'late-binding', where we go look at the address of library
  // loaded dynamically
  XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
  if(!XInputGetState) { XInputGetState = XInputGetStateStub; }
  XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
  if(!XInputSetState) { XInputSetState = XInputSetStateStub; }

  // TODO(Cristián): Diagnostics
}

/**
 * Translates from the XINPUT_STATE into a x_input_gamepad_state
 */
internal x_input_gamepad_state
GetGamepadState(XINPUT_STATE *controllerState)
{
  XINPUT_GAMEPAD *pad = &controllerState->Gamepad;
  x_input_gamepad_state gamepadState = {};
  gamepadState.up = (pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
  gamepadState.down = (pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
  gamepadState.left = (pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
  gamepadState.right = (pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
  gamepadState.start = (pad->wButtons & XINPUT_GAMEPAD_START);
  gamepadState.back = (pad->wButtons & XINPUT_GAMEPAD_BACK);
  gamepadState.leftThumb = (pad->wButtons & XINPUT_GAMEPAD_LEFT_THUMB);
  gamepadState.rightThumb = (pad->wButtons & XINPUT_GAMEPAD_RIGHT_THUMB);
  gamepadState.leftShoulder = (pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
  gamepadState.rightShoulder = (pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
  gamepadState.aButton = (pad->wButtons & XINPUT_GAMEPAD_A);
  gamepadState.bButton = (pad->wButtons & XINPUT_GAMEPAD_B);
  gamepadState.yButton = (pad->wButtons & XINPUT_GAMEPAD_Y);
  gamepadState.xButton = (pad->wButtons & XINPUT_GAMEPAD_X);

  // We inport the sticks
  gamepadState.leftThumbX = pad->sThumbLX;
  gamepadState.leftThumbY = pad->sThumbLY;
  gamepadState.rightThumbX = pad->sThumbRX;
  gamepadState.rightThumbY = pad->sThumbRY;

  // We import the triggers
  gamepadState.leftTrigger = pad->bLeftTrigger;
  gamepadState.rightTrigger = pad->bRightTrigger;

  return gamepadState;
}

internal void
Win32ProcessButtonState(game_button_state *oldState,
                        game_button_state *newState,
                        bool32 currentState)
{
  newState->halfTransitionCount =
    oldState->endedDown != currentState ? 1 : 0;
  newState->endedDown = currentState;
}

internal void
Win32ProcessKeyboardMessage(game_button_state *oldState,
                            game_button_state *newState,
                            bool32 currentState)
{
  newState->halfTransitionCount++;
  newState->endedDown = currentState;
  oldState->endedDown = currentState;
}

#define _WIN32_X_INPUT_WRAPPER_INCLUDED
#endif
