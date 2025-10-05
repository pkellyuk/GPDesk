#include "gpdesk.h"
#include "input.h"
#include "system_control.h"
#include "config.h"
#include <commctrl.h>

// Global application state
AppState g_appState = {0};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    LOG_ENTRY("hInstance=0x%p, nCmdShow=%d", hInstance, nCmdShow);
    
    // Initialize application
    if (!App_Initialize(hInstance))
    {
        LOG_ERROR("Failed to initialize application");
        LOG_EXIT("return=-1");
        return -1;
    }
    
    LOG_INFO("GPDesk application started successfully");
    
    // Main message loop
    MSG msg;
    g_appState.isRunning = true;
    
    while (g_appState.isRunning && GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    // Cleanup
    App_Cleanup();
    
    LOG_INFO("GPDesk application shutdown complete");
    LOG_EXIT("return=%d", (int)msg.wParam);
    return (int)msg.wParam;
}

bool App_Initialize(HINSTANCE hInstance)
{
    LOG_ENTRY("hInstance=0x%p", hInstance);
    
    if (!hInstance)
    {
        LOG_ERROR("Invalid hInstance parameter");
        LOG_EXIT("return=false");
        return false;
    }
    
    // Initialize logger first
    Logger_Initialize();
    
    // Initialize configuration system
    if (!Config_Initialize())
    {
        LOG_ERROR("Failed to initialize configuration system");
        LOG_EXIT("return=false");
        return false;
    }
    
    // Initialize common controls (use simple version for compatibility)
    InitCommonControls();
    LOG_DEBUG("Common controls initialized");
    
    // Initialize system control
    if (!SystemControl_Initialize())
    {
        LOG_ERROR("Failed to initialize system control");
        LOG_EXIT("return=false");
        return false;
    }
    
    // Initialize input system
    if (!Input_Initialize())
    {
        LOG_ERROR("Failed to initialize input system");
        SystemControl_Cleanup();
        LOG_EXIT("return=false");
        return false;
    }
    
    // Store application instance
    g_appState.hInstance = hInstance;
    g_appState.currentMode = MODE_GAMEPAD; // Start in gamepad mode (no interaction)
    g_appState.isOverlayVisible = false;
    g_appState.isAnimating = false;
    
    // Create main window
    if (!App_CreateMainWindow())
    {
        LOG_ERROR("Failed to create main window");
        LOG_EXIT("return=false");
        return false;
    }
    
    // Create overlay window
    if (!App_CreateOverlayWindow())
    {
        LOG_ERROR("Failed to create overlay window");
        LOG_EXIT("return=false");
        return false;
    }
    
    // Create system tray icon
    if (!App_CreateSystemTray())
    {
        LOG_ERROR("Failed to create system tray icon");
        LOG_EXIT("return=false");
        return false;
    }
    
    LOG_INFO("Application initialization complete");
    LOG_EXIT("return=true");
    return true;
}

void App_Cleanup(void)
{
    LOG_ENTRY_SIMPLE();
    
    // Cleanup input system
    Input_Cleanup();
    
    // Cleanup system control
    SystemControl_Cleanup();
    
    // Cleanup configuration system
    Config_Cleanup();
    
    // Remove system tray icon
    if (g_appState.nid.cbSize > 0)
    {
        Shell_NotifyIcon(NIM_DELETE, &g_appState.nid);
        LOG_DEBUG("System tray icon removed");
    }
    
    // Destroy windows
    if (g_appState.hOverlayWindow)
    {
        DestroyWindow(g_appState.hOverlayWindow);
        g_appState.hOverlayWindow = NULL;
        LOG_DEBUG("Overlay window destroyed");
    }
    
    if (g_appState.hMainWindow)
    {
        DestroyWindow(g_appState.hMainWindow);
        g_appState.hMainWindow = NULL;
        LOG_DEBUG("Main window destroyed");
    }
    
    // Cleanup logger last
    Logger_Cleanup();
    
    LOG_EXIT_SIMPLE();
}

bool App_CreateMainWindow(void)
{
    LOG_ENTRY_SIMPLE();
    
    // Register window class
    WNDCLASSEX wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = MainWindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = g_appState.hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = APP_CLASS_NAME;
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    
    if (!RegisterClassEx(&wc))
    {
        LOG_ERROR("Failed to register main window class, error: %lu", GetLastError());
        LOG_EXIT("return=false");
        return false;
    }
    
    // Create the window (initially hidden)
    g_appState.hMainWindow = CreateWindowEx(
        0,
        APP_CLASS_NAME,
        APP_NAME,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        800, 600,
        NULL,
        NULL,
        g_appState.hInstance,
        NULL
    );
    
    if (!g_appState.hMainWindow)
    {
        LOG_ERROR("Failed to create main window, error: %lu", GetLastError());
        LOG_EXIT("return=false");
        return false;
    }
    
    LOG_INFO("Main window created successfully, handle: 0x%p", g_appState.hMainWindow);
    LOG_EXIT("return=true");
    return true;
}

bool App_CreateOverlayWindow(void)
{
    LOG_ENTRY_SIMPLE();
    
    // Register overlay window class
    WNDCLASSEX wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = OverlayWindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = g_appState.hInstance;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL; // Transparent background
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "GPDeskOverlay";
    wc.hIconSm = NULL;
    
    if (!RegisterClassEx(&wc))
    {
        LOG_ERROR("Failed to register overlay window class, error: %lu", GetLastError());
        LOG_EXIT("return=false");
        return false;
    }
    
    // Create overlay window (slide-in panel from right, similar to Armory Crate SE)
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int overlayWidth = 400;  // Fixed width panel

    // Initialize off-screen to the right
    g_appState.overlaySlidePosition = screenWidth;

    g_appState.hOverlayWindow = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
        "GPDeskOverlay",
        "GPDesk Overlay",
        WS_POPUP,
        screenWidth, 0,  // Start off-screen to the right
        overlayWidth,
        screenHeight,
        NULL,
        NULL,
        g_appState.hInstance,
        NULL
    );
    
    if (!g_appState.hOverlayWindow)
    {
        LOG_ERROR("Failed to create overlay window, error: %lu", GetLastError());
        LOG_EXIT("return=false");
        return false;
    }
    
    // Set initial transparency
    SetLayeredWindowAttributes(g_appState.hOverlayWindow, RGB(0, 0, 0), 200, LWA_ALPHA);
    
    LOG_INFO("Overlay window created successfully, handle: 0x%p", g_appState.hOverlayWindow);
    LOG_EXIT("return=true");
    return true;
}

bool App_CreateSystemTray(void)
{
    LOG_ENTRY_SIMPLE();
    
    ZeroMemory(&g_appState.nid, sizeof(NOTIFYICONDATA));
    g_appState.nid.cbSize = sizeof(NOTIFYICONDATA);
    g_appState.nid.hWnd = g_appState.hMainWindow;
    g_appState.nid.uID = ID_TRAY_ICON;
    g_appState.nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_appState.nid.uCallbackMessage = WM_TRAYICON;
    g_appState.nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    strcpy_s(g_appState.nid.szTip, sizeof(g_appState.nid.szTip), APP_NAME " - Gamepad PC Control");
    
    if (!Shell_NotifyIcon(NIM_ADD, &g_appState.nid))
    {
        LOG_ERROR("Failed to create system tray icon, error: %lu", GetLastError());
        LOG_EXIT("return=false");
        return false;
    }
    
    LOG_INFO("System tray icon created successfully");
    LOG_EXIT("return=true");
    return true;
}

LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CREATE:
            LOG_DEBUG("Main window WM_CREATE received");
            return 0;
            
        case WM_CLOSE:
            LOG_DEBUG("Main window WM_CLOSE received - minimizing to tray");
            App_MinimizeToTray();
            return 0;
            
        case WM_DESTROY:
            LOG_DEBUG("Main window WM_DESTROY received");
            g_appState.isRunning = false;
            PostQuitMessage(0);
            return 0;
            
        case WM_TRAYICON:
            if (lParam == WM_RBUTTONUP)
            {
                POINT pt;
                GetCursorPos(&pt);
                App_ShowTrayMenu(hwnd, pt);
            }
            else if (lParam == WM_LBUTTONDBLCLK)
            {
                App_RestoreFromTray();
            }
            return 0;
            
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case ID_TRAY_SHOW:
                    App_RestoreFromTray();
                    break;
                    
                case ID_TRAY_EXIT:
                    g_appState.isRunning = false;
                    DestroyWindow(hwnd);
                    break;
                    
                case ID_TRAY_CONFIG:
                    // TODO: Show configuration window
                    LOG_INFO("Configuration requested");
                    break;
            }
            return 0;
            
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

LRESULT CALLBACK OverlayWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            RECT rect;
            GetClientRect(hwnd, &rect);

            // Dark background similar to Armory Crate SE
            HBRUSH bgBrush = CreateSolidBrush(RGB(20, 20, 30));
            FillRect(hdc, &rect, bgBrush);
            DeleteObject(bgBrush);

            // Header section
            RECT headerRect = {0, 0, rect.right, 60};
            HBRUSH headerBrush = CreateSolidBrush(RGB(30, 30, 45));
            FillRect(hdc, &headerRect, headerBrush);
            DeleteObject(headerBrush);

            // Title
            HFONT hTitleFont = CreateFontA(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");
            HFONT hOldFont = SelectObject(hdc, hTitleFont);

            SetTextColor(hdc, RGB(255, 255, 255));
            SetBkMode(hdc, TRANSPARENT);
            RECT titleRect = {20, 15, rect.right - 20, 50};
            DrawTextA(hdc, "GPDesk", -1, &titleRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

            SelectObject(hdc, hOldFont);
            DeleteObject(hTitleFont);

            // Mode section
            HFONT hNormalFont = CreateFontA(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");
            SelectObject(hdc, hNormalFont);

            int yPos = 80;
            RECT labelRect = {20, yPos, rect.right - 20, yPos + 30};
            SetTextColor(hdc, RGB(180, 180, 180));
            DrawTextA(hdc, "Control Mode", -1, &labelRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

            yPos += 35;
            RECT modeRect = {20, yPos, rect.right - 20, yPos + 40};

            // Highlight current mode
            if (g_appState.currentMode == MODE_DESKTOP)
            {
                HBRUSH modeBrush = CreateSolidBrush(RGB(60, 120, 200));
                FillRect(hdc, &modeRect, modeBrush);
                DeleteObject(modeBrush);
            }

            SetTextColor(hdc, RGB(255, 255, 255));
            const char* modeText = (g_appState.currentMode == MODE_DESKTOP) ?
                "Desktop Control (Mouse/KB)" : "Gamepad Mode (No Interaction)";
            DrawTextA(hdc, modeText, -1, &modeRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            // Instructions
            yPos += 60;
            SetTextColor(hdc, RGB(180, 180, 180));
            RECT instrRect = {20, yPos, rect.right - 20, yPos + 100};
            const char* instructions =
                "A Button: Toggle Mode\n\n"
                "Desktop Mode:\n"
                "- Right Stick: Move mouse\n"
                "- RT: Left click (hold)\n"
                "- LT: Right click (hold)\n\n"
                "Xbox Button: Close overlay";
            DrawTextA(hdc, instructions, -1, &instrRect, DT_LEFT | DT_WORDBREAK);

            SelectObject(hdc, hOldFont);
            DeleteObject(hNormalFont);

            EndPaint(hwnd, &ps);
            return 0;
        }
        
        case WM_TIMER:
            if (wParam == 1 && g_appState.isAnimating)
            {
                App_AnimateOverlay();
            }
            return 0;

        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE)
            {
                App_ToggleOverlay();
            }
            return 0;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

void App_ShowTrayMenu(HWND hwnd, POINT pt)
{
    LOG_ENTRY("hwnd=0x%p, pt=(%ld,%ld)", hwnd, pt.x, pt.y);
    
    HMENU hMenu = CreatePopupMenu();
    if (!hMenu)
    {
        LOG_ERROR("Failed to create tray menu");
        LOG_EXIT_SIMPLE();
        return;
    }
    
    AppendMenu(hMenu, MF_STRING, ID_TRAY_SHOW, "&Show GPDesk");
    AppendMenu(hMenu, MF_STRING, ID_TRAY_CONFIG, "&Configuration");
    AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hMenu, MF_STRING, ID_TRAY_EXIT, "E&xit");
    
    SetForegroundWindow(hwnd);
    TrackPopupMenu(hMenu, TPM_RIGHTALIGN | TPM_BOTTOMALIGN, pt.x, pt.y, 0, hwnd, NULL);
    DestroyMenu(hMenu);
    
    LOG_EXIT_SIMPLE();
}

void App_ToggleOverlay(void)
{
    LOG_ENTRY_SIMPLE();

    g_appState.isOverlayVisible = !g_appState.isOverlayVisible;

    if (g_appState.isOverlayVisible)
    {
        ShowWindow(g_appState.hOverlayWindow, SW_SHOW);
        g_appState.isAnimating = true;

        // Start animation timer
        SetTimer(g_appState.hOverlayWindow, 1, 16, NULL); // ~60 FPS
        LOG_INFO("Overlay showing - animating in");
    }
    else
    {
        g_appState.isAnimating = true;
        SetTimer(g_appState.hOverlayWindow, 1, 16, NULL);
        LOG_INFO("Overlay hiding - animating out");
    }

    LOG_EXIT("isOverlayVisible=%d", g_appState.isOverlayVisible);
}

void App_AnimateOverlay(void)
{
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int overlayWidth = 400;
    int targetPos;

    if (g_appState.isOverlayVisible)
    {
        // Slide in from right
        targetPos = screenWidth - overlayWidth;
    }
    else
    {
        // Slide out to right
        targetPos = screenWidth;
    }

    // Smooth animation
    int diff = targetPos - g_appState.overlaySlidePosition;
    if (abs(diff) > 2)
    {
        g_appState.overlaySlidePosition += diff / 5; // Ease motion
    }
    else
    {
        g_appState.overlaySlidePosition = targetPos;
        g_appState.isAnimating = false;
        KillTimer(g_appState.hOverlayWindow, 1);

        if (!g_appState.isOverlayVisible)
        {
            ShowWindow(g_appState.hOverlayWindow, SW_HIDE);
        }
    }

    // Update window position
    SetWindowPos(g_appState.hOverlayWindow, HWND_TOPMOST,
                 g_appState.overlaySlidePosition, 0,
                 overlayWidth, GetSystemMetrics(SM_CYSCREEN),
                 SWP_NOACTIVATE);
}

void App_ToggleMode(void)
{
    LOG_ENTRY_SIMPLE();

    g_appState.currentMode = (g_appState.currentMode == MODE_DESKTOP) ?
        MODE_GAMEPAD : MODE_DESKTOP;

    LOG_INFO("Control mode changed to: %s", App_GetModeName());

    // Redraw overlay to show new mode
    InvalidateRect(g_appState.hOverlayWindow, NULL, TRUE);

    LOG_EXIT_SIMPLE();
}

const char* App_GetModeName(void)
{
    return (g_appState.currentMode == MODE_DESKTOP) ?
        "Desktop Control" : "Gamepad Mode";
}

void App_MinimizeToTray(void)
{
    LOG_ENTRY_SIMPLE();
    
    ShowWindow(g_appState.hMainWindow, SW_HIDE);
    g_appState.isMinimizedToTray = true;
    
    LOG_INFO("Application minimized to system tray");
    LOG_EXIT_SIMPLE();
}

void App_RestoreFromTray(void)
{
    LOG_ENTRY_SIMPLE();
    
    ShowWindow(g_appState.hMainWindow, SW_RESTORE);
    SetForegroundWindow(g_appState.hMainWindow);
    g_appState.isMinimizedToTray = false;
    
    LOG_INFO("Application restored from system tray");
    LOG_EXIT_SIMPLE();
}