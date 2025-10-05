#ifndef GPDESK_H
#define GPDESK_H

#include <windows.h>
#include <shellapi.h>
#include <stdbool.h>
#include "logger.h"

// Application constants
#define APP_NAME "GPDesk"
#define APP_VERSION "1.0.0"
#define APP_CLASS_NAME "GPDeskMainWindow"
#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_ICON 1
#define ID_TRAY_EXIT 2001
#define ID_TRAY_SHOW 2002
#define ID_TRAY_CONFIG 2003
#define ID_GITHUB_LINK 2004

// Control mode enumeration
typedef enum
{
    MODE_GAMEPAD = 0,    // Normal gamepad mode (no desktop interaction)
    MODE_DESKTOP = 1     // Desktop control mode (mouse/keyboard control)
} ControlMode;

// Application state structure
typedef struct
{
    HINSTANCE hInstance;
    HWND hMainWindow;
    HWND hOverlayWindow;
    NOTIFYICONDATA nid;
    bool isRunning;
    bool isOverlayVisible;
    bool isMinimizedToTray;
    ControlMode currentMode;
    int overlaySlidePosition; // Current X position for slide animation
    bool isAnimating;
} AppState;

// Function prototypes
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK OverlayWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

bool App_Initialize(HINSTANCE hInstance);
void App_Cleanup(void);
bool App_CreateMainWindow(void);
bool App_CreateOverlayWindow(void);
bool App_CreateSystemTray(void);
void App_ShowTrayMenu(HWND hwnd, POINT pt);
void App_ToggleOverlay(void);
void App_AnimateOverlay(void);
void App_ToggleMode(void);
const char* App_GetModeName(void);
void App_MinimizeToTray(void);
void App_RestoreFromTray(void);

// Global application state
extern AppState g_appState;

#endif // GPDESK_H