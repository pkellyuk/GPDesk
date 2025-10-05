#include "input.h"
#include "gpdesk.h"
#include "system_control.h"

// Global input system
InputSystem g_inputSystem = {0};

// Default button mappings
// NOTE: Xbox/Guide button cannot be detected via XInput (system reserves it)
// Using START button for overlay toggle instead
static ButtonMapping g_buttonMappings[] =
{
    {BUTTON_START, ACTION_TOGGLE_OVERLAY, "", false, 0},  // Xbox button replacement
    // D-pad now used for line-by-line scrolling (handled in Input_UpdateMouseControl)
    {BUTTON_BACK, ACTION_VOLUME_MUTE, "", false, 0},
    {BUTTON_Y, ACTION_POWER_SLEEP, "", true, 2000}, // Disabled
    {BUTTON_X, ACTION_TOGGLE_OSK, "", false, 0},
    {BUTTON_LEFT_SHOULDER, ACTION_BROWSER_BACK, "", false, 0},
    {BUTTON_RIGHT_SHOULDER, ACTION_BROWSER_FORWARD, "", false, 0},
    // A button handled separately for mode toggle when overlay is visible
    {BUTTON_B, ACTION_NONE, "", false, 0}
};

static const int g_numButtonMappings = sizeof(g_buttonMappings) / sizeof(ButtonMapping);

bool Input_Initialize(void)
{
    LOG_ENTRY_SIMPLE();
    
    if (g_inputSystem.isInitialized)
    {
        LOG_WARNING("Input system already initialized");
        LOG_EXIT("return=true");
        return true;
    }
    
    // Initialize critical section
    InitializeCriticalSection(&g_inputSystem.inputLock);
    
    // Initialize controller states
    for (int i = 0; i < MAX_CONTROLLERS; i++)
    {
        ZeroMemory(&g_inputSystem.controllers[i], sizeof(ControllerState));
        g_inputSystem.controllers[i].isConnected = false;
        g_inputSystem.controllers[i].wasConnected = false;
        g_inputSystem.controllers[i].lastPacketNumber = 0;
        g_inputSystem.controllers[i].connectionCheckTime = 0;
    }
    
    g_inputSystem.lastUpdateTime = GetTickCount();
    g_inputSystem.shouldStopThread = false;
    
    // Create input thread for continuous monitoring
    g_inputSystem.inputThread = CreateThread(
        NULL,
        0,
        Input_ThreadProc,
        NULL,
        0,
        NULL
    );
    
    if (!g_inputSystem.inputThread)
    {
        LOG_ERROR("Failed to create input thread, error: %lu", GetLastError());
        DeleteCriticalSection(&g_inputSystem.inputLock);
        LOG_EXIT("return=false");
        return false;
    }
    
    g_inputSystem.isInitialized = true;
    
    LOG_INFO("Input system initialized successfully");
    LOG_EXIT("return=true");
    return true;
}

void Input_Cleanup(void)
{
    LOG_ENTRY_SIMPLE();
    
    if (!g_inputSystem.isInitialized)
    {
        LOG_EXIT_SIMPLE();
        return;
    }
    
    // Stop input thread
    g_inputSystem.shouldStopThread = true;
    
    if (g_inputSystem.inputThread)
    {
        WaitForSingleObject(g_inputSystem.inputThread, 5000);
        CloseHandle(g_inputSystem.inputThread);
        g_inputSystem.inputThread = NULL;
        LOG_DEBUG("Input thread stopped");
    }
    
    // Turn off all vibration
    for (int i = 0; i < MAX_CONTROLLERS; i++)
    {
        if (g_inputSystem.controllers[i].isConnected)
        {
            Input_SetVibration(i, 0.0f, 0.0f);
        }
    }
    
    // Cleanup critical section
    DeleteCriticalSection(&g_inputSystem.inputLock);
    
    g_inputSystem.isInitialized = false;
    
    LOG_INFO("Input system cleanup complete");
    LOG_EXIT_SIMPLE();
}

DWORD WINAPI Input_ThreadProc(LPVOID lpParameter)
{
    LOG_ENTRY_SIMPLE();
    
    while (!g_inputSystem.shouldStopThread)
    {
        Input_Update();
        Input_ProcessActions();

        // Update mouse control for first connected controller
        for (int i = 0; i < MAX_CONTROLLERS; i++)
        {
            if (g_inputSystem.controllers[i].isConnected)
            {
                Input_UpdateMouseControl(i);
                break;
            }
        }

        Sleep(16); // ~60 FPS update rate
    }
    
    LOG_EXIT_SIMPLE();
    return 0;
}

void Input_Update(void)
{
    if (!g_inputSystem.isInitialized)
    {
        return;
    }
    
    EnterCriticalSection(&g_inputSystem.inputLock);
    
    DWORD currentTime = GetTickCount();
    g_inputSystem.lastUpdateTime = currentTime;
    
    // Update each controller
    for (int i = 0; i < MAX_CONTROLLERS; i++)
    {
        ControllerState* controller = &g_inputSystem.controllers[i];
        
        // Store previous state
        controller->previousState = controller->currentState;
        controller->wasConnected = controller->isConnected;
        
        // Get current state
        DWORD result = XInputGetState(i, &controller->currentState);
        
        if (result == ERROR_SUCCESS)
        {
            if (!controller->isConnected)
            {
                LOG_INFO("Controller %d connected", i);
                controller->isConnected = true;
            }
            
            controller->lastPacketNumber = controller->currentState.dwPacketNumber;
            controller->connectionCheckTime = currentTime;
        }
        else
        {
            if (controller->isConnected)
            {
                LOG_INFO("Controller %d disconnected", i);
                controller->isConnected = false;
                
                // Turn off vibration for disconnected controller
                Input_SetVibration(i, 0.0f, 0.0f);
            }
        }
    }
    
    LeaveCriticalSection(&g_inputSystem.inputLock);
}

bool Input_IsControllerConnected(int controllerIndex)
{
    if (controllerIndex < 0 || controllerIndex >= MAX_CONTROLLERS)
    {
        LOG_ERROR("Invalid controller index: %d", controllerIndex);
        return false;
    }
    
    if (!g_inputSystem.isInitialized)
    {
        return false;
    }
    
    EnterCriticalSection(&g_inputSystem.inputLock);
    bool connected = g_inputSystem.controllers[controllerIndex].isConnected;
    LeaveCriticalSection(&g_inputSystem.inputLock);
    
    return connected;
}

bool Input_IsButtonPressed(int controllerIndex, GamepadButton button)
{
    if (controllerIndex < 0 || controllerIndex >= MAX_CONTROLLERS)
    {
        return false;
    }
    
    if (!g_inputSystem.isInitialized || !g_inputSystem.controllers[controllerIndex].isConnected)
    {
        return false;
    }
    
    EnterCriticalSection(&g_inputSystem.inputLock);
    bool pressed = (g_inputSystem.controllers[controllerIndex].currentState.Gamepad.wButtons & button) != 0;
    LeaveCriticalSection(&g_inputSystem.inputLock);
    
    return pressed;
}

bool Input_IsButtonJustPressed(int controllerIndex, GamepadButton button)
{
    if (controllerIndex < 0 || controllerIndex >= MAX_CONTROLLERS)
    {
        return false;
    }
    
    if (!g_inputSystem.isInitialized || !g_inputSystem.controllers[controllerIndex].isConnected)
    {
        return false;
    }
    
    EnterCriticalSection(&g_inputSystem.inputLock);
    
    bool currentPressed = (g_inputSystem.controllers[controllerIndex].currentState.Gamepad.wButtons & button) != 0;
    bool previousPressed = (g_inputSystem.controllers[controllerIndex].previousState.Gamepad.wButtons & button) != 0;
    bool justPressed = currentPressed && !previousPressed;
    
    LeaveCriticalSection(&g_inputSystem.inputLock);
    
    return justPressed;
}

bool Input_IsButtonJustReleased(int controllerIndex, GamepadButton button)
{
    if (controllerIndex < 0 || controllerIndex >= MAX_CONTROLLERS)
    {
        return false;
    }
    
    if (!g_inputSystem.isInitialized || !g_inputSystem.controllers[controllerIndex].isConnected)
    {
        return false;
    }
    
    EnterCriticalSection(&g_inputSystem.inputLock);
    
    bool currentPressed = (g_inputSystem.controllers[controllerIndex].currentState.Gamepad.wButtons & button) != 0;
    bool previousPressed = (g_inputSystem.controllers[controllerIndex].previousState.Gamepad.wButtons & button) != 0;
    bool justReleased = !currentPressed && previousPressed;
    
    LeaveCriticalSection(&g_inputSystem.inputLock);
    
    return justReleased;
}

float Input_GetLeftTrigger(int controllerIndex)
{
    if (controllerIndex < 0 || controllerIndex >= MAX_CONTROLLERS)
    {
        return 0.0f;
    }
    
    if (!g_inputSystem.isInitialized || !g_inputSystem.controllers[controllerIndex].isConnected)
    {
        return 0.0f;
    }
    
    EnterCriticalSection(&g_inputSystem.inputLock);
    BYTE triggerValue = g_inputSystem.controllers[controllerIndex].currentState.Gamepad.bLeftTrigger;
    LeaveCriticalSection(&g_inputSystem.inputLock);
    
    if (triggerValue < TRIGGER_THRESHOLD)
    {
        return 0.0f;
    }
    
    return (float)(triggerValue - TRIGGER_THRESHOLD) / (255.0f - TRIGGER_THRESHOLD);
}

float Input_GetRightTrigger(int controllerIndex)
{
    if (controllerIndex < 0 || controllerIndex >= MAX_CONTROLLERS)
    {
        return 0.0f;
    }
    
    if (!g_inputSystem.isInitialized || !g_inputSystem.controllers[controllerIndex].isConnected)
    {
        return 0.0f;
    }
    
    EnterCriticalSection(&g_inputSystem.inputLock);
    BYTE triggerValue = g_inputSystem.controllers[controllerIndex].currentState.Gamepad.bRightTrigger;
    LeaveCriticalSection(&g_inputSystem.inputLock);
    
    if (triggerValue < TRIGGER_THRESHOLD)
    {
        return 0.0f;
    }
    
    return (float)(triggerValue - TRIGGER_THRESHOLD) / (255.0f - TRIGGER_THRESHOLD);
}

void Input_GetLeftStick(int controllerIndex, float* x, float* y)
{
    if (!x || !y)
    {
        LOG_ERROR("Invalid parameters: x=0x%p, y=0x%p", x, y);
        return;
    }
    
    *x = 0.0f;
    *y = 0.0f;
    
    if (controllerIndex < 0 || controllerIndex >= MAX_CONTROLLERS)
    {
        return;
    }
    
    if (!g_inputSystem.isInitialized || !g_inputSystem.controllers[controllerIndex].isConnected)
    {
        return;
    }
    
    EnterCriticalSection(&g_inputSystem.inputLock);
    SHORT thumbX = g_inputSystem.controllers[controllerIndex].currentState.Gamepad.sThumbLX;
    SHORT thumbY = g_inputSystem.controllers[controllerIndex].currentState.Gamepad.sThumbLY;
    LeaveCriticalSection(&g_inputSystem.inputLock);
    
    // Apply deadzone
    *x = Input_ApplyDeadzone((float)thumbX / 32767.0f, (float)INPUT_DEADZONE / 32767.0f);
    *y = Input_ApplyDeadzone((float)thumbY / 32767.0f, (float)INPUT_DEADZONE / 32767.0f);
}

void Input_GetRightStick(int controllerIndex, float* x, float* y)
{
    if (!x || !y)
    {
        LOG_ERROR("Invalid parameters: x=0x%p, y=0x%p", x, y);
        return;
    }
    
    *x = 0.0f;
    *y = 0.0f;
    
    if (controllerIndex < 0 || controllerIndex >= MAX_CONTROLLERS)
    {
        return;
    }
    
    if (!g_inputSystem.isInitialized || !g_inputSystem.controllers[controllerIndex].isConnected)
    {
        return;
    }
    
    EnterCriticalSection(&g_inputSystem.inputLock);
    SHORT thumbX = g_inputSystem.controllers[controllerIndex].currentState.Gamepad.sThumbRX;
    SHORT thumbY = g_inputSystem.controllers[controllerIndex].currentState.Gamepad.sThumbRY;
    LeaveCriticalSection(&g_inputSystem.inputLock);
    
    // Apply deadzone
    *x = Input_ApplyDeadzone((float)thumbX / 32767.0f, (float)INPUT_DEADZONE / 32767.0f);
    *y = Input_ApplyDeadzone((float)thumbY / 32767.0f, (float)INPUT_DEADZONE / 32767.0f);
}

void Input_SetVibration(int controllerIndex, float leftMotor, float rightMotor)
{
    if (controllerIndex < 0 || controllerIndex >= MAX_CONTROLLERS)
    {
        LOG_ERROR("Invalid controller index: %d", controllerIndex);
        return;
    }
    
    if (!g_inputSystem.isInitialized || !g_inputSystem.controllers[controllerIndex].isConnected)
    {
        return;
    }
    
    // Clamp values to valid range
    if (leftMotor < 0.0f) leftMotor = 0.0f;
    if (leftMotor > 1.0f) leftMotor = 1.0f;
    if (rightMotor < 0.0f) rightMotor = 0.0f;
    if (rightMotor > 1.0f) rightMotor = 1.0f;
    
    XINPUT_VIBRATION vibration;
    vibration.wLeftMotorSpeed = (WORD)(leftMotor * 65535.0f);
    vibration.wRightMotorSpeed = (WORD)(rightMotor * 65535.0f);
    
    XInputSetState(controllerIndex, &vibration);
}

float Input_ApplyDeadzone(float value, float deadzone)
{
    if (value > deadzone)
    {
        return (value - deadzone) / (1.0f - deadzone);
    }
    else if (value < -deadzone)
    {
        return (value + deadzone) / (1.0f - deadzone);
    }
    
    return 0.0f;
}

bool Input_HasStateChanged(int controllerIndex)
{
    if (controllerIndex < 0 || controllerIndex >= MAX_CONTROLLERS)
    {
        return false;
    }
    
    if (!g_inputSystem.isInitialized || !g_inputSystem.controllers[controllerIndex].isConnected)
    {
        return false;
    }
    
    EnterCriticalSection(&g_inputSystem.inputLock);
    bool changed = g_inputSystem.controllers[controllerIndex].currentState.dwPacketNumber != 
                   g_inputSystem.controllers[controllerIndex].previousState.dwPacketNumber;
    LeaveCriticalSection(&g_inputSystem.inputLock);
    
    return changed;
}

void Input_ProcessActions(void)
{
    if (!g_inputSystem.isInitialized)
    {
        return;
    }

    // Check for button presses on the first connected controller
    int activeController = -1;
    for (int i = 0; i < MAX_CONTROLLERS; i++)
    {
        if (g_inputSystem.controllers[i].isConnected)
        {
            activeController = i;
            break;
        }
    }

    if (activeController == -1)
    {
        return;
    }

    // Special handling for A button - toggle mode when overlay is visible
    extern AppState g_appState;
    if (g_appState.isOverlayVisible && Input_IsButtonJustPressed(activeController, BUTTON_A))
    {
        App_ToggleMode();
        return; // Don't process other actions when overlay is up
    }

    // Process button mappings
    for (int i = 0; i < g_numButtonMappings; i++)
    {
        ButtonMapping* mapping = &g_buttonMappings[i];

        if (mapping->action == ACTION_NONE)
        {
            continue;
        }

        bool shouldExecute = false;

        if (mapping->requiresHold)
        {
            // TODO: Implement hold timing logic
            if (Input_IsButtonPressed(activeController, mapping->button))
            {
                // For now, just execute on press
                shouldExecute = Input_IsButtonJustPressed(activeController, mapping->button);
            }
        }
        else
        {
            shouldExecute = Input_IsButtonJustPressed(activeController, mapping->button);
        }

        if (shouldExecute)
        {
            LOG_DEBUG("Executing action %d for button %d", mapping->action, mapping->button);
            Input_ExecuteAction(mapping->action, mapping->customCommand);
        }
    }
}

void Input_ExecuteAction(GamepadAction action, const char* customCommand)
{
    LOG_ENTRY("action=%d, customCommand=%s", action, customCommand ? customCommand : "NULL");
    
    switch (action)
    {
        case ACTION_TOGGLE_OVERLAY:
            App_ToggleOverlay();
            break;
            
        case ACTION_VOLUME_UP:
            Audio_VolumeUp(0.1f); // 10% increment
            break;
            
        case ACTION_VOLUME_DOWN:
            Audio_VolumeDown(0.1f); // 10% decrement
            break;
            
        case ACTION_VOLUME_MUTE:
            Audio_ToggleMute();
            break;
            
        case ACTION_BRIGHTNESS_UP:
            Display_BrightnessUp(10); // 10% increment
            break;
            
        case ACTION_BRIGHTNESS_DOWN:
            Display_BrightnessDown(10); // 10% decrement
            break;
            
        case ACTION_POWER_SHUTDOWN:
            // Power management disabled for safety
            // Power_ExecuteAction(POWER_ACTION_SHUTDOWN, false);
            LOG_INFO("Power shutdown action disabled");
            break;

        case ACTION_POWER_SLEEP:
            // Power management disabled for safety
            // Power_ExecuteAction(POWER_ACTION_SLEEP, false);
            LOG_INFO("Power sleep action disabled");
            break;
            
        case ACTION_LAUNCH_APP:
            if (customCommand && strlen(customCommand) > 0)
            {
                App_LaunchApplication(customCommand, NULL);
            }
            break;
            
        case ACTION_CUSTOM_COMMAND:
            if (customCommand && strlen(customCommand) > 0)
            {
                App_ExecuteCommand(customCommand);
            }
            break;

        case ACTION_BROWSER_BACK:
            {
                INPUT input = {0};
                input.type = INPUT_KEYBOARD;
                input.ki.wVk = VK_BROWSER_BACK;
                SendInput(1, &input, sizeof(INPUT));
                input.ki.dwFlags = KEYEVENTF_KEYUP;
                SendInput(1, &input, sizeof(INPUT));
                LOG_DEBUG("Browser back");
            }
            break;

        case ACTION_BROWSER_FORWARD:
            {
                INPUT input = {0};
                input.type = INPUT_KEYBOARD;
                input.ki.wVk = VK_BROWSER_FORWARD;
                SendInput(1, &input, sizeof(INPUT));
                input.ki.dwFlags = KEYEVENTF_KEYUP;
                SendInput(1, &input, sizeof(INPUT));
                LOG_DEBUG("Browser forward");
            }
            break;

        case ACTION_TOGGLE_OSK:
            {
                LOG_ENTRY_SIMPLE();

                // Try to find OSK window using multiple possible class names
                HWND hOSK = FindWindowA("OSKMainClass", NULL);
                if (!hOSK)
                {
                    hOSK = FindWindowA("OSKMainClass", "On-Screen Keyboard");
                }
                if (!hOSK)
                {
                    hOSK = FindWindowW(L"OSKMainClass", NULL);
                }

                if (hOSK && IsWindow(hOSK))
                {
                    // If OSK is already running, close it
                    LOG_DEBUG("Found OSK window handle: %p", hOSK);
                    SendMessageA(hOSK, WM_SYSCOMMAND, SC_CLOSE, 0);
                    LOG_DEBUG("On-screen keyboard close command sent");
                }
                else
                {
                    // Launch the on-screen keyboard with full path
                    char oskPath[MAX_PATH];
                    GetSystemDirectoryA(oskPath, MAX_PATH);
                    strcat(oskPath, "\\osk.exe");

                    HINSTANCE hInst = ShellExecuteA(NULL, "open", oskPath, NULL, NULL, SW_SHOW);
                    if ((INT_PTR)hInst > 32)
                    {
                        LOG_DEBUG("On-screen keyboard launched: %s", oskPath);
                    }
                    else
                    {
                        LOG_ERROR("Failed to launch OSK, error: %d", (int)(INT_PTR)hInst);
                    }
                }

                LOG_EXIT_SIMPLE();
            }
            break;

        default:
            LOG_WARNING("Unknown action: %d", action);
            break;
    }

    LOG_EXIT_SIMPLE();
}

void Input_UpdateMouseControl(int controllerIndex)
{
    if (controllerIndex < 0 || controllerIndex >= MAX_CONTROLLERS)
    {
        return;
    }

    if (!g_inputSystem.isInitialized || !g_inputSystem.controllers[controllerIndex].isConnected)
    {
        return;
    }

    // Only allow mouse control in Desktop mode
    extern AppState g_appState;
    if (g_appState.currentMode != MODE_DESKTOP)
    {
        return;
    }

    // Get right stick position for mouse movement
    float rightStickX, rightStickY;
    Input_GetRightStick(controllerIndex, &rightStickX, &rightStickY);

    // Only move mouse if stick is moved significantly
    if (rightStickX != 0.0f || rightStickY != 0.0f)
    {
        // Get current cursor position
        POINT cursorPos;
        if (!GetCursorPos(&cursorPos))
        {
            return;
        }

        // Calculate mouse movement (scale for sensitivity)
        // Higher sensitivity for faster movement
        float sensitivity = 20.0f;
        int deltaX = (int)(rightStickX * sensitivity);
        int deltaY = (int)(-rightStickY * sensitivity); // Invert Y axis for natural movement

        // Calculate new position
        int newX = cursorPos.x + deltaX;
        int newY = cursorPos.y + deltaY;

        // Clamp to screen bounds
        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);

        if (newX < 0) newX = 0;
        if (newX >= screenWidth) newX = screenWidth - 1;
        if (newY < 0) newY = 0;
        if (newY >= screenHeight) newY = screenHeight - 1;

        // Move the cursor
        SetCursorPos(newX, newY);
    }

    // Get left stick position for smooth scrolling
    float leftStickX, leftStickY;
    Input_GetLeftStick(controllerIndex, &leftStickX, &leftStickY);

    // Smooth scrolling with left stick (vertical only)
    if (leftStickY != 0.0f)
    {
        // Scale the scroll amount based on stick position
        // WHEEL_DELTA is 120 units per "notch"
        // Use float for accumulation to allow smooth scrolling
        static float scrollAccumulator = 0.0f;

        // Sensitivity for smooth scrolling - higher value = faster scroll
        float scrollSensitivity = 15.0f;
        scrollAccumulator += leftStickY * scrollSensitivity;

        // Only send scroll events when we've accumulated enough
        if (scrollAccumulator >= 1.0f || scrollAccumulator <= -1.0f)
        {
            int scrollAmount = (int)scrollAccumulator;

            INPUT input = {0};
            input.type = INPUT_MOUSE;
            input.mi.dwFlags = MOUSEEVENTF_WHEEL;
            input.mi.mouseData = scrollAmount * (WHEEL_DELTA / 10); // Divide for finer control
            SendInput(1, &input, sizeof(INPUT));

            scrollAccumulator -= (float)scrollAmount;
        }
    }

    // D-pad for line-by-line scrolling
    static bool wasDpadUpPressed = false;
    static bool wasDpadDownPressed = false;

    bool dpadUp = Input_IsButtonPressed(controllerIndex, BUTTON_DPAD_UP);
    bool dpadDown = Input_IsButtonPressed(controllerIndex, BUTTON_DPAD_DOWN);

    // Scroll up (one line at a time)
    if (dpadUp && !wasDpadUpPressed)
    {
        INPUT input = {0};
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = MOUSEEVENTF_WHEEL;
        input.mi.mouseData = WHEEL_DELTA / 3; // One line
        SendInput(1, &input, sizeof(INPUT));
        LOG_DEBUG("D-pad scroll up (line-by-line)");
    }

    // Scroll down (one line at a time)
    if (dpadDown && !wasDpadDownPressed)
    {
        INPUT input = {0};
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = MOUSEEVENTF_WHEEL;
        input.mi.mouseData = -(WHEEL_DELTA / 3); // One line
        SendInput(1, &input, sizeof(INPUT));
        LOG_DEBUG("D-pad scroll down (line-by-line)");
    }

    wasDpadUpPressed = dpadUp;
    wasDpadDownPressed = dpadDown;

    // Handle mouse clicks with triggers or buttons
    // SWAPPED: Right trigger = left click, Left trigger = right click
    bool rightTrigger = Input_GetRightTrigger(controllerIndex) > 0.5f;
    bool leftTrigger = Input_GetLeftTrigger(controllerIndex) > 0.5f;

    static bool wasRightPressed = false;
    static bool wasLeftPressed = false;
    static bool isRightButtonDown = false;
    static bool isLeftButtonDown = false;

    // Left click with RIGHT trigger or A button (swapped)
    bool leftClick = rightTrigger || Input_IsButtonPressed(controllerIndex, BUTTON_A);
    if (leftClick && !wasRightPressed)
    {
        // Button down
        INPUT input = {0};
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
        SendInput(1, &input, sizeof(INPUT));
        isRightButtonDown = true;
        LOG_DEBUG("Mouse left button down");
    }
    else if (!leftClick && wasRightPressed && isRightButtonDown)
    {
        // Button up
        INPUT input = {0};
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
        SendInput(1, &input, sizeof(INPUT));
        isRightButtonDown = false;
        LOG_DEBUG("Mouse left button up");
    }
    wasRightPressed = leftClick;

    // Right click with LEFT trigger or B button (swapped)
    bool rightClick = leftTrigger || Input_IsButtonPressed(controllerIndex, BUTTON_B);
    if (rightClick && !wasLeftPressed)
    {
        // Button down
        INPUT input = {0};
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
        SendInput(1, &input, sizeof(INPUT));
        isLeftButtonDown = true;
        LOG_DEBUG("Mouse right button down");
    }
    else if (!rightClick && wasLeftPressed && isLeftButtonDown)
    {
        // Button up
        INPUT input = {0};
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
        SendInput(1, &input, sizeof(INPUT));
        isLeftButtonDown = false;
        LOG_DEBUG("Mouse right button up");
    }
    wasLeftPressed = rightClick;
}

void Input_MouseClick(bool leftButton, bool rightButton)
{
    INPUT input = {0};
    input.type = INPUT_MOUSE;

    if (leftButton)
    {
        // Left button down
        input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
        SendInput(1, &input, sizeof(INPUT));

        // Left button up
        input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
        SendInput(1, &input, sizeof(INPUT));

        LOG_DEBUG("Mouse left click");
    }

    if (rightButton)
    {
        // Right button down
        input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
        SendInput(1, &input, sizeof(INPUT));

        // Right button up
        input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
        SendInput(1, &input, sizeof(INPUT));

        LOG_DEBUG("Mouse right click");
    }
}