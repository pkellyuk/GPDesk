#include "system_control.h"
#include <powrprof.h>

bool Power_Shutdown(bool force)
{
    LOG_ENTRY("force=%d", force);
    
    // Enable shutdown privilege
    HANDLE hToken;
    TOKEN_PRIVILEGES tkp;
    
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
    {
        LOG_ERROR("Failed to open process token, error: %lu", GetLastError());
        LOG_EXIT("return=false");
        return false;
    }
    
    LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    
    if (!AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0))
    {
        LOG_ERROR("Failed to adjust token privileges, error: %lu", GetLastError());
        CloseHandle(hToken);
        LOG_EXIT("return=false");
        return false;
    }
    
    CloseHandle(hToken);
    
    // Initiate shutdown
    UINT flags = EWX_SHUTDOWN | EWX_POWEROFF;
    if (force)
    {
        flags |= EWX_FORCE;
    }
    
    if (!ExitWindowsEx(flags, SHTDN_REASON_MAJOR_APPLICATION | SHTDN_REASON_MINOR_OTHER))
    {
        LOG_ERROR("Failed to initiate shutdown, error: %lu", GetLastError());
        LOG_EXIT("return=false");
        return false;
    }
    
    LOG_INFO("Shutdown initiated, force=%d", force);
    LOG_EXIT("return=true");
    return true;
}

bool Power_Restart(bool force)
{
    LOG_ENTRY("force=%d", force);
    
    // Enable shutdown privilege
    HANDLE hToken;
    TOKEN_PRIVILEGES tkp;
    
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
    {
        LOG_ERROR("Failed to open process token, error: %lu", GetLastError());
        LOG_EXIT("return=false");
        return false;
    }
    
    LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    
    if (!AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0))
    {
        LOG_ERROR("Failed to adjust token privileges, error: %lu", GetLastError());
        CloseHandle(hToken);
        LOG_EXIT("return=false");
        return false;
    }
    
    CloseHandle(hToken);
    
    // Initiate restart
    UINT flags = EWX_REBOOT;
    if (force)
    {
        flags |= EWX_FORCE;
    }
    
    if (!ExitWindowsEx(flags, SHTDN_REASON_MAJOR_APPLICATION | SHTDN_REASON_MINOR_OTHER))
    {
        LOG_ERROR("Failed to initiate restart, error: %lu", GetLastError());
        LOG_EXIT("return=false");
        return false;
    }
    
    LOG_INFO("Restart initiated, force=%d", force);
    LOG_EXIT("return=true");
    return true;
}

bool Power_Sleep(void)
{
    LOG_ENTRY_SIMPLE();
    
    if (!SetSuspendState(FALSE, FALSE, FALSE))
    {
        LOG_ERROR("Failed to initiate sleep, error: %lu", GetLastError());
        LOG_EXIT("return=false");
        return false;
    }
    
    LOG_INFO("Sleep initiated");
    LOG_EXIT("return=true");
    return true;
}

bool Power_Hibernate(void)
{
    LOG_ENTRY_SIMPLE();
    
    if (!SetSuspendState(TRUE, FALSE, FALSE))
    {
        LOG_ERROR("Failed to initiate hibernate, error: %lu", GetLastError());
        LOG_EXIT("return=false");
        return false;
    }
    
    LOG_INFO("Hibernate initiated");
    LOG_EXIT("return=true");
    return true;
}

bool Power_Lock(void)
{
    LOG_ENTRY_SIMPLE();
    
    if (!LockWorkStation())
    {
        LOG_ERROR("Failed to lock workstation, error: %lu", GetLastError());
        LOG_EXIT("return=false");
        return false;
    }
    
    LOG_INFO("Workstation locked");
    LOG_EXIT("return=true");
    return true;
}

bool Power_Logoff(bool force)
{
    LOG_ENTRY("force=%d", force);
    
    UINT flags = EWX_LOGOFF;
    if (force)
    {
        flags |= EWX_FORCE;
    }
    
    if (!ExitWindowsEx(flags, SHTDN_REASON_MAJOR_APPLICATION | SHTDN_REASON_MINOR_OTHER))
    {
        LOG_ERROR("Failed to initiate logoff, error: %lu", GetLastError());
        LOG_EXIT("return=false");
        return false;
    }
    
    LOG_INFO("Logoff initiated, force=%d", force);
    LOG_EXIT("return=true");
    return true;
}

bool Power_ExecuteAction(PowerAction action, bool force)
{
    LOG_ENTRY("action=%d, force=%d", action, force);
    
    bool result = false;
    
    switch (action)
    {
        case POWER_ACTION_SHUTDOWN:
            result = Power_Shutdown(force);
            break;
            
        case POWER_ACTION_RESTART:
            result = Power_Restart(force);
            break;
            
        case POWER_ACTION_SLEEP:
            result = Power_Sleep();
            break;
            
        case POWER_ACTION_HIBERNATE:
            result = Power_Hibernate();
            break;
            
        case POWER_ACTION_LOCK:
            result = Power_Lock();
            break;
            
        case POWER_ACTION_LOGOFF:
            result = Power_Logoff(force);
            break;
            
        default:
            LOG_ERROR("Unknown power action: %d", action);
            result = false;
            break;
    }
    
    LOG_EXIT("return=%d", result);
    return result;
}