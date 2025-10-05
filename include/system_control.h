#ifndef SYSTEM_CONTROL_H
#define SYSTEM_CONTROL_H

#include <windows.h>
#include <stdbool.h>
#include <endpointvolume.h>
#include <mmdeviceapi.h>
#include <powrprof.h>
#include <winuser.h>
#include "logger.h"

// Power management constants
typedef enum
{
    POWER_ACTION_SHUTDOWN = 0,
    POWER_ACTION_RESTART,
    POWER_ACTION_SLEEP,
    POWER_ACTION_HIBERNATE,
    POWER_ACTION_LOCK,
    POWER_ACTION_LOGOFF
} PowerAction;

// Display control constants
typedef enum
{
    DISPLAY_BRIGHTNESS_UP = 0,
    DISPLAY_BRIGHTNESS_DOWN,
    DISPLAY_TOGGLE_MONITOR,
    DISPLAY_NEXT_RESOLUTION,
    DISPLAY_NEXT_REFRESH_RATE
} DisplayAction;

// Audio control structure
typedef struct
{
    IMMDeviceEnumerator* deviceEnumerator;
    IMMDevice* defaultDevice;
    IAudioEndpointVolume* endpointVolume;
    bool isInitialized;
    bool isMuted;
    float currentVolume;
} AudioSystem;

// System control structure
typedef struct
{
    AudioSystem audio;
    bool isInitialized;
    HMODULE powerProfileLib;
    HMODULE user32Lib;
} SystemControl;

// Function prototypes - Audio Control
bool SystemControl_Initialize(void);
void SystemControl_Cleanup(void);
bool Audio_Initialize(void);
void Audio_Cleanup(void);
bool Audio_SetVolume(float volume);
float Audio_GetVolume(void);
bool Audio_SetMute(bool mute);
bool Audio_IsMuted(void);
bool Audio_VolumeUp(float increment);
bool Audio_VolumeDown(float increment);
bool Audio_ToggleMute(void);

// Function prototypes - Power Management
bool Power_Shutdown(bool force);
bool Power_Restart(bool force);
bool Power_Sleep(void);
bool Power_Hibernate(void);
bool Power_Lock(void);
bool Power_Logoff(bool force);
bool Power_ExecuteAction(PowerAction action, bool force);

// Function prototypes - Display Control
bool Display_SetBrightness(int brightness);
int Display_GetBrightness(void);
bool Display_BrightnessUp(int increment);
bool Display_BrightnessDown(int increment);
bool Display_TurnOffMonitors(void);
bool Display_TurnOnMonitors(void);
bool Display_CycleThroughDisplays(void);

// Function prototypes - Application Control
bool App_LaunchApplication(const char* applicationPath, const char* parameters);
bool App_KillApplication(const char* processName);
bool App_ExecuteCommand(const char* command);
bool App_OpenFile(const char* filePath);
bool App_OpenURL(const char* url);

// Function prototypes - Window Management
bool Window_MinimizeAll(void);
bool Window_RestoreAll(void);
bool Window_ShowDesktop(void);
HWND Window_GetForegroundWindow(void);
bool Window_BringToFront(HWND hwnd);
bool Window_Minimize(HWND hwnd);
bool Window_Maximize(HWND hwnd);
bool Window_Close(HWND hwnd);

// Function prototypes - Hardware Control
bool Hardware_EjectCD(char driveLetter);
bool Hardware_SetScreenSaver(bool enable);
bool Hardware_TriggerScreenSaver(void);

// Global system control instance
extern SystemControl g_systemControl;

#endif // SYSTEM_CONTROL_H