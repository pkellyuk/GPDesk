#ifndef INPUT_H
#define INPUT_H

#include <windows.h>
#include <xinput.h>
#include <stdbool.h>
#include "logger.h"

// XInput constants
#define MAX_CONTROLLERS 4
#define INPUT_DEADZONE 8000
#define TRIGGER_THRESHOLD 128

// Button mapping for easy reference
typedef enum
{
    BUTTON_DPAD_UP = XINPUT_GAMEPAD_DPAD_UP,
    BUTTON_DPAD_DOWN = XINPUT_GAMEPAD_DPAD_DOWN,
    BUTTON_DPAD_LEFT = XINPUT_GAMEPAD_DPAD_LEFT,
    BUTTON_DPAD_RIGHT = XINPUT_GAMEPAD_DPAD_RIGHT,
    BUTTON_START = XINPUT_GAMEPAD_START,
    BUTTON_BACK = XINPUT_GAMEPAD_BACK,
    BUTTON_LEFT_THUMB = XINPUT_GAMEPAD_LEFT_THUMB,
    BUTTON_RIGHT_THUMB = XINPUT_GAMEPAD_RIGHT_THUMB,
    BUTTON_LEFT_SHOULDER = XINPUT_GAMEPAD_LEFT_SHOULDER,
    BUTTON_RIGHT_SHOULDER = XINPUT_GAMEPAD_RIGHT_SHOULDER,
    BUTTON_A = XINPUT_GAMEPAD_A,
    BUTTON_B = XINPUT_GAMEPAD_B,
    BUTTON_X = XINPUT_GAMEPAD_X,
    BUTTON_Y = XINPUT_GAMEPAD_Y
} GamepadButton;

// Action types for button mapping
typedef enum
{
    ACTION_NONE = 0,
    ACTION_TOGGLE_OVERLAY,
    ACTION_VOLUME_UP,
    ACTION_VOLUME_DOWN,
    ACTION_VOLUME_MUTE,
    ACTION_BRIGHTNESS_UP,
    ACTION_BRIGHTNESS_DOWN,
    ACTION_POWER_SHUTDOWN,
    ACTION_POWER_SLEEP,
    ACTION_LAUNCH_APP,
    ACTION_NEXT_DISPLAY,
    ACTION_TV_POWER,
    ACTION_TV_INPUT,
    ACTION_BROWSER_BACK,
    ACTION_BROWSER_FORWARD,
    ACTION_TOGGLE_OSK,
    ACTION_CUSTOM_COMMAND
} GamepadAction;

// Controller state structure
typedef struct
{
    bool isConnected;
    bool wasConnected;
    XINPUT_STATE currentState;
    XINPUT_STATE previousState;
    DWORD lastPacketNumber;
    DWORD connectionCheckTime;
} ControllerState;

// Input system state
typedef struct
{
    ControllerState controllers[MAX_CONTROLLERS];
    bool isInitialized;
    DWORD lastUpdateTime;
    HANDLE inputThread;
    bool shouldStopThread;
    CRITICAL_SECTION inputLock;
} InputSystem;

// Button mapping structure
typedef struct
{
    GamepadButton button;
    GamepadAction action;
    char customCommand[MAX_PATH];
    bool requiresHold;
    DWORD holdTime;
} ButtonMapping;

// Function prototypes
bool Input_Initialize(void);
void Input_Cleanup(void);
void Input_Update(void);
bool Input_IsControllerConnected(int controllerIndex);
bool Input_IsButtonPressed(int controllerIndex, GamepadButton button);
bool Input_IsButtonJustPressed(int controllerIndex, GamepadButton button);
bool Input_IsButtonJustReleased(int controllerIndex, GamepadButton button);
float Input_GetLeftTrigger(int controllerIndex);
float Input_GetRightTrigger(int controllerIndex);
void Input_GetLeftStick(int controllerIndex, float* x, float* y);
void Input_GetRightStick(int controllerIndex, float* x, float* y);
void Input_SetVibration(int controllerIndex, float leftMotor, float rightMotor);
void Input_ProcessActions(void);
DWORD WINAPI Input_ThreadProc(LPVOID lpParameter);

// Button mapping functions
bool Input_LoadButtonMappings(const char* configFile);
bool Input_SaveButtonMappings(const char* configFile);
void Input_SetButtonMapping(GamepadButton button, GamepadAction action, const char* customCommand);
GamepadAction Input_GetButtonAction(GamepadButton button);
void Input_ExecuteAction(GamepadAction action, const char* customCommand);

// Mouse control functions
void Input_UpdateMouseControl(int controllerIndex);
void Input_MouseClick(bool leftButton, bool rightButton);

// Utility functions
float Input_ApplyDeadzone(float value, float deadzone);
bool Input_HasStateChanged(int controllerIndex);

// Global input system
extern InputSystem g_inputSystem;

#endif // INPUT_H