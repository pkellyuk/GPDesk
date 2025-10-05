#include "system_control.h"
#include <shellapi.h>
#include <tlhelp32.h>

bool App_LaunchApplication(const char* applicationPath, const char* parameters)
{
    LOG_ENTRY("applicationPath=%s, parameters=%s", 
              applicationPath ? applicationPath : "NULL",
              parameters ? parameters : "NULL");
    
    if (!applicationPath)
    {
        LOG_ERROR("Invalid application path");
        LOG_EXIT("return=false");
        return false;
    }
    
    SHELLEXECUTEINFOA sei;
    ZeroMemory(&sei, sizeof(sei));
    sei.cbSize = sizeof(sei);
    sei.fMask = SEE_MASK_NOCLOSEPROCESS;
    sei.lpVerb = "open";
    sei.lpFile = applicationPath;
    sei.lpParameters = parameters;
    sei.nShow = SW_SHOWNORMAL;
    
    if (!ShellExecuteExA(&sei))
    {
        LOG_ERROR("Failed to launch application, error: %lu", GetLastError());
        LOG_EXIT("return=false");
        return false;
    }
    
    if (sei.hProcess)
    {
        CloseHandle(sei.hProcess);
    }
    
    LOG_INFO("Application launched: %s %s", applicationPath, parameters ? parameters : "");
    LOG_EXIT("return=true");
    return true;
}

bool App_KillApplication(const char* processName)
{
    LOG_ENTRY("processName=%s", processName ? processName : "NULL");
    
    if (!processName)
    {
        LOG_ERROR("Invalid process name");
        LOG_EXIT("return=false");
        return false;
    }
    
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        LOG_ERROR("Failed to create process snapshot, error: %lu", GetLastError());
        LOG_EXIT("return=false");
        return false;
    }
    
    PROCESSENTRY32 pe32;
    ZeroMemory(&pe32, sizeof(pe32));
    pe32.dwSize = sizeof(pe32);
    
    bool processFound = false;
    int processesKilled = 0;
    
    if (Process32First(hSnapshot, &pe32))
    {
        do
        {
            if (_stricmp(pe32.szExeFile, processName) == 0)
            {
                processFound = true;
                
                HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe32.th32ProcessID);
                if (hProcess)
                {
                    if (TerminateProcess(hProcess, 0))
                    {
                        processesKilled++;
                        LOG_DEBUG("Terminated process: %s (PID: %lu)", processName, pe32.th32ProcessID);
                    }
                    else
                    {
                        LOG_ERROR("Failed to terminate process %s (PID: %lu), error: %lu", 
                                 processName, pe32.th32ProcessID, GetLastError());
                    }
                    
                    CloseHandle(hProcess);
                }
                else
                {
                    LOG_ERROR("Failed to open process %s (PID: %lu), error: %lu", 
                             processName, pe32.th32ProcessID, GetLastError());
                }
            }
        } while (Process32Next(hSnapshot, &pe32));
    }
    
    CloseHandle(hSnapshot);
    
    if (!processFound)
    {
        LOG_WARNING("Process not found: %s", processName);
        LOG_EXIT("return=false");
        return false;
    }
    
    LOG_INFO("Killed %d instances of process: %s", processesKilled, processName);
    LOG_EXIT("return=true");
    return true;
}

bool App_ExecuteCommand(const char* command)
{
    LOG_ENTRY("command=%s", command ? command : "NULL");
    
    if (!command)
    {
        LOG_ERROR("Invalid command");
        LOG_EXIT("return=false");
        return false;
    }
    
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE; // Hide command window
    
    ZeroMemory(&pi, sizeof(pi));
    
    // Create a copy of the command string as CreateProcess may modify it
    char* commandCopy = (char*)malloc(strlen(command) + 1);
    if (!commandCopy)
    {
        LOG_ERROR("Failed to allocate memory for command copy");
        LOG_EXIT("return=false");
        return false;
    }
    
    strcpy_s(commandCopy, strlen(command) + 1, command);
    
    if (!CreateProcessA(NULL, commandCopy, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    {
        LOG_ERROR("Failed to execute command, error: %lu", GetLastError());
        free(commandCopy);
        LOG_EXIT("return=false");
        return false;
    }
    
    // Clean up
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    free(commandCopy);
    
    LOG_INFO("Command executed: %s", command);
    LOG_EXIT("return=true");
    return true;
}

bool App_OpenFile(const char* filePath)
{
    LOG_ENTRY("filePath=%s", filePath ? filePath : "NULL");
    
    if (!filePath)
    {
        LOG_ERROR("Invalid file path");
        LOG_EXIT("return=false");
        return false;
    }
    
    HINSTANCE result = ShellExecuteA(NULL, "open", filePath, NULL, NULL, SW_SHOWNORMAL);
    
    if ((INT_PTR)result <= 32)
    {
        LOG_ERROR("Failed to open file, error: %d", (int)(INT_PTR)result);
        LOG_EXIT("return=false");
        return false;
    }
    
    LOG_INFO("File opened: %s", filePath);
    LOG_EXIT("return=true");
    return true;
}

bool App_OpenURL(const char* url)
{
    LOG_ENTRY("url=%s", url ? url : "NULL");
    
    if (!url)
    {
        LOG_ERROR("Invalid URL");
        LOG_EXIT("return=false");
        return false;
    }
    
    HINSTANCE result = ShellExecuteA(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
    
    if ((INT_PTR)result <= 32)
    {
        LOG_ERROR("Failed to open URL, error: %d", (int)(INT_PTR)result);
        LOG_EXIT("return=false");
        return false;
    }
    
    LOG_INFO("URL opened: %s", url);
    LOG_EXIT("return=true");
    return true;
}