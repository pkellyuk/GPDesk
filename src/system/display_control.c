#include "system_control.h"
#include <winuser.h>
#include <physicalmonitorenumerationapi.h>
#include <highlevelmonitorconfigurationapi.h>

bool Display_SetBrightness(int brightness)
{
    LOG_ENTRY("brightness=%d", brightness);
    
    if (brightness < 0)
    {
        brightness = 0;
    }
    else if (brightness > 100)
    {
        brightness = 100;
    }
    
    // Get monitor handle
    HWND hwnd = GetDesktopWindow();
    HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);
    
    DWORD numPhysicalMonitors;
    if (!GetNumberOfPhysicalMonitorsFromHMONITOR(hMonitor, &numPhysicalMonitors))
    {
        LOG_ERROR("Failed to get number of physical monitors, error: %lu", GetLastError());
        LOG_EXIT("return=false");
        return false;
    }
    
    LPPHYSICAL_MONITOR physicalMonitors = (LPPHYSICAL_MONITOR)malloc(
        numPhysicalMonitors * sizeof(PHYSICAL_MONITOR));
    
    if (!physicalMonitors)
    {
        LOG_ERROR("Failed to allocate memory for physical monitors");
        LOG_EXIT("return=false");
        return false;
    }
    
    if (!GetPhysicalMonitorsFromHMONITOR(hMonitor, numPhysicalMonitors, physicalMonitors))
    {
        LOG_ERROR("Failed to get physical monitors, error: %lu", GetLastError());
        free(physicalMonitors);
        LOG_EXIT("return=false");
        return false;
    }
    
    bool success = false;
    
    // Set brightness for the first monitor
    if (numPhysicalMonitors > 0)
    {
        DWORD minBrightness, currentBrightness, maxBrightness;
        
        if (GetMonitorBrightness(physicalMonitors[0].hPhysicalMonitor, 
                                &minBrightness, &currentBrightness, &maxBrightness))
        {
            DWORD newBrightness = minBrightness + 
                ((maxBrightness - minBrightness) * brightness) / 100;
                
            if (SetMonitorBrightness(physicalMonitors[0].hPhysicalMonitor, newBrightness))
            {
                success = true;
                LOG_INFO("Brightness set to %d%% (value: %lu)", brightness, newBrightness);
            }
            else
            {
                LOG_ERROR("Failed to set monitor brightness, error: %lu", GetLastError());
            }
        }
        else
        {
            LOG_ERROR("Failed to get monitor brightness, error: %lu", GetLastError());
        }
    }
    
    // Cleanup
    DestroyPhysicalMonitors(numPhysicalMonitors, physicalMonitors);
    free(physicalMonitors);
    
    LOG_EXIT("return=%d", success);
    return success;
}

int Display_GetBrightness(void)
{
    LOG_ENTRY_SIMPLE();
    
    // Get monitor handle
    HWND hwnd = GetDesktopWindow();
    HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);
    
    DWORD numPhysicalMonitors;
    if (!GetNumberOfPhysicalMonitorsFromHMONITOR(hMonitor, &numPhysicalMonitors))
    {
        LOG_ERROR("Failed to get number of physical monitors, error: %lu", GetLastError());
        LOG_EXIT("return=0");
        return 0;
    }
    
    LPPHYSICAL_MONITOR physicalMonitors = (LPPHYSICAL_MONITOR)malloc(
        numPhysicalMonitors * sizeof(PHYSICAL_MONITOR));
    
    if (!physicalMonitors)
    {
        LOG_ERROR("Failed to allocate memory for physical monitors");
        LOG_EXIT("return=0");
        return 0;
    }
    
    if (!GetPhysicalMonitorsFromHMONITOR(hMonitor, numPhysicalMonitors, physicalMonitors))
    {
        LOG_ERROR("Failed to get physical monitors, error: %lu", GetLastError());
        free(physicalMonitors);
        LOG_EXIT("return=0");
        return 0;
    }
    
    int brightnessPercent = 0;
    
    // Get brightness from the first monitor
    if (numPhysicalMonitors > 0)
    {
        DWORD minBrightness, currentBrightness, maxBrightness;
        
        if (GetMonitorBrightness(physicalMonitors[0].hPhysicalMonitor, 
                                &minBrightness, &currentBrightness, &maxBrightness))
        {
            if (maxBrightness > minBrightness)
            {
                brightnessPercent = ((currentBrightness - minBrightness) * 100) / 
                                   (maxBrightness - minBrightness);
            }
            
            LOG_DEBUG("Current brightness: %d%% (value: %lu, range: %lu-%lu)", 
                     brightnessPercent, currentBrightness, minBrightness, maxBrightness);
        }
        else
        {
            LOG_ERROR("Failed to get monitor brightness, error: %lu", GetLastError());
        }
    }
    
    // Cleanup
    DestroyPhysicalMonitors(numPhysicalMonitors, physicalMonitors);
    free(physicalMonitors);
    
    LOG_EXIT("return=%d", brightnessPercent);
    return brightnessPercent;
}

bool Display_BrightnessUp(int increment)
{
    LOG_ENTRY("increment=%d", increment);
    
    if (increment <= 0)
    {
        LOG_ERROR("Invalid increment value: %d", increment);
        LOG_EXIT("return=false");
        return false;
    }
    
    int currentBrightness = Display_GetBrightness();
    int newBrightness = currentBrightness + increment;
    
    bool result = Display_SetBrightness(newBrightness);
    
    LOG_INFO("Brightness up: %d%% -> %d%%", currentBrightness, newBrightness);
    LOG_EXIT("return=%d", result);
    return result;
}

bool Display_BrightnessDown(int increment)
{
    LOG_ENTRY("increment=%d", increment);
    
    if (increment <= 0)
    {
        LOG_ERROR("Invalid increment value: %d", increment);
        LOG_EXIT("return=false");
        return false;
    }
    
    int currentBrightness = Display_GetBrightness();
    int newBrightness = currentBrightness - increment;
    
    bool result = Display_SetBrightness(newBrightness);
    
    LOG_INFO("Brightness down: %d%% -> %d%%", currentBrightness, newBrightness);
    LOG_EXIT("return=%d", result);
    return result;
}

bool Display_TurnOffMonitors(void)
{
    LOG_ENTRY_SIMPLE();
    
    // Send monitor off message
    SendMessage(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, 2);
    
    LOG_INFO("Monitors turned off");
    LOG_EXIT("return=true");
    return true;
}

bool Display_TurnOnMonitors(void)
{
    LOG_ENTRY_SIMPLE();
    
    // Send monitor on message
    SendMessage(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, -1);
    
    LOG_INFO("Monitors turned on");
    LOG_EXIT("return=true");
    return true;
}

bool Display_CycleThroughDisplays(void)
{
    LOG_ENTRY_SIMPLE();
    
    // Simulate Win+P key combination to cycle through display modes
    keybd_event(VK_LWIN, 0, 0, 0);
    keybd_event('P', 0, 0, 0);
    keybd_event('P', 0, KEYEVENTF_KEYUP, 0);
    keybd_event(VK_LWIN, 0, KEYEVENTF_KEYUP, 0);
    
    LOG_INFO("Display mode cycling initiated");
    LOG_EXIT("return=true");
    return true;
}