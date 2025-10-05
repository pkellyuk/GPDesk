#include "config.h"
#include <stdio.h>
#include <string.h>

// Global configuration instance
Config g_config = {0};

bool Config_Initialize(void)
{
    LOG_ENTRY_SIMPLE();
    
    if (g_config.isLoaded)
    {
        LOG_WARNING("Configuration already initialized");
        LOG_EXIT("return=true");
        return true;
    }
    
    // Set default values
    Config_SetDefaults();
    
    // Load configuration
    if (!Config_Load())
    {
        LOG_WARNING("Failed to load configuration, using defaults");
    }
    
    // Validate and apply settings
    if (!Config_ValidateSettings())
    {
        LOG_WARNING("Invalid configuration detected, resetting to defaults");
        Config_SetDefaults();
    }
    
    Config_ApplySettings();
    
    g_config.isLoaded = true;
    
    LOG_INFO("Configuration system initialized");
    LOG_EXIT("return=true");
    return true;
}

void Config_Cleanup(void)
{
    LOG_ENTRY_SIMPLE();
    
    if (!g_config.isLoaded)
    {
        LOG_EXIT_SIMPLE();
        return;
    }
    
    // Save current configuration
    if (!Config_Save())
    {
        LOG_ERROR("Failed to save configuration");
    }
    
    g_config.isLoaded = false;
    
    LOG_INFO("Configuration system cleanup complete");
    LOG_EXIT_SIMPLE();
}

void Config_SetDefaults(void)
{
    LOG_ENTRY_SIMPLE();
    
    // Application settings
    g_config.startWithWindows = false;
    g_config.minimizeToTray = true;
    g_config.showOverlayOnStart = false;
    g_config.logLevel = 0; // Debug level
    
    // Input settings
    g_config.enableGamepadInput = true;
    g_config.gamepadDeadzone = INPUT_DEADZONE;
    g_config.gamepadVibrationLevel = 50; // 50%
    
    // Audio settings
    g_config.volumeIncrement = 0.1f; // 10%
    g_config.muteOnStartup = false;
    
    // Display settings
    g_config.brightnessIncrement = 10; // 10%
    g_config.turnOffDisplayOnIdle = false;
    g_config.idleTimeout = 300; // 5 minutes
    
    // Power settings
    g_config.confirmPowerActions = true;
    g_config.powerButtonHoldTime = 2000; // 2 seconds
    
    // Custom settings
    strcpy_s(g_config.customCommand1, sizeof(g_config.customCommand1), "");
    strcpy_s(g_config.customCommand2, sizeof(g_config.customCommand2), "");
    strcpy_s(g_config.customCommand3, sizeof(g_config.customCommand3), "");
    strcpy_s(g_config.favoriteApp1, sizeof(g_config.favoriteApp1), "notepad.exe");
    strcpy_s(g_config.favoriteApp2, sizeof(g_config.favoriteApp2), "calc.exe");
    strcpy_s(g_config.favoriteApp3, sizeof(g_config.favoriteApp3), "mspaint.exe");
    
    LOG_DEBUG("Default configuration values set");
    LOG_EXIT_SIMPLE();
}

bool Config_Load(void)
{
    LOG_ENTRY_SIMPLE();
    
    // Try to load from registry first
    if (Config_LoadFromRegistry())
    {
        LOG_INFO("Configuration loaded from registry");
        LOG_EXIT("return=true");
        return true;
    }
    
    // Fall back to file
    char configPath[MAX_PATH];
    GetCurrentDirectoryA(sizeof(configPath), configPath);
    strcat_s(configPath, sizeof(configPath), "\\");
    strcat_s(configPath, sizeof(configPath), CONFIG_FILE_NAME);
    
    if (Config_LoadFromFile(configPath))
    {
        LOG_INFO("Configuration loaded from file: %s", configPath);
        LOG_EXIT("return=true");
        return true;
    }
    
    LOG_WARNING("Configuration not found, using defaults");
    LOG_EXIT("return=false");
    return false;
}

bool Config_Save(void)
{
    LOG_ENTRY_SIMPLE();
    
    // Save to registry
    if (Config_SaveToRegistry())
    {
        LOG_INFO("Configuration saved to registry");
    }
    else
    {
        LOG_WARNING("Failed to save configuration to registry");
    }
    
    // Also save to file as backup
    char configPath[MAX_PATH];
    GetCurrentDirectoryA(sizeof(configPath), configPath);
    strcat_s(configPath, sizeof(configPath), "\\");
    strcat_s(configPath, sizeof(configPath), CONFIG_FILE_NAME);
    
    if (Config_SaveToFile(configPath))
    {
        LOG_INFO("Configuration saved to file: %s", configPath);
        LOG_EXIT("return=true");
        return true;
    }
    else
    {
        LOG_ERROR("Failed to save configuration to file");
        LOG_EXIT("return=false");
        return false;
    }
}

bool Config_LoadFromRegistry(void)
{
    LOG_ENTRY_SIMPLE();
    
    HKEY hKey;
    LONG result = RegOpenKeyExA(HKEY_CURRENT_USER, CONFIG_REGISTRY_KEY, 0, KEY_READ, &hKey);
    
    if (result != ERROR_SUCCESS)
    {
        LOG_DEBUG("Registry key not found or cannot be opened");
        LOG_EXIT("return=false");
        return false;
    }
    
    DWORD dwValue;
    
    // Load application settings
    if (Registry_ReadDWORD(hKey, "StartWithWindows", &dwValue))
        g_config.startWithWindows = (dwValue != 0);
    
    if (Registry_ReadDWORD(hKey, "MinimizeToTray", &dwValue))
        g_config.minimizeToTray = (dwValue != 0);
    
    if (Registry_ReadDWORD(hKey, "ShowOverlayOnStart", &dwValue))
        g_config.showOverlayOnStart = (dwValue != 0);
    
    if (Registry_ReadDWORD(hKey, "LogLevel", &dwValue))
        g_config.logLevel = (int)dwValue;
    
    // Load input settings
    if (Registry_ReadDWORD(hKey, "EnableGamepadInput", &dwValue))
        g_config.enableGamepadInput = (dwValue != 0);
    
    if (Registry_ReadDWORD(hKey, "GamepadDeadzone", &dwValue))
        g_config.gamepadDeadzone = (int)dwValue;
    
    if (Registry_ReadDWORD(hKey, "GamepadVibrationLevel", &dwValue))
        g_config.gamepadVibrationLevel = (int)dwValue;
    
    // Load audio settings
    if (Registry_ReadDWORD(hKey, "VolumeIncrement", &dwValue))
    {
        // Store as integer percentage, convert to float
        g_config.volumeIncrement = (float)dwValue / 100.0f;
    }
    
    if (Registry_ReadDWORD(hKey, "MuteOnStartup", &dwValue))
        g_config.muteOnStartup = (dwValue != 0);
    
    // Load display settings
    if (Registry_ReadDWORD(hKey, "BrightnessIncrement", &dwValue))
        g_config.brightnessIncrement = (int)dwValue;
    
    if (Registry_ReadDWORD(hKey, "TurnOffDisplayOnIdle", &dwValue))
        g_config.turnOffDisplayOnIdle = (dwValue != 0);
    
    if (Registry_ReadDWORD(hKey, "IdleTimeout", &dwValue))
        g_config.idleTimeout = (int)dwValue;
    
    // Load power settings
    if (Registry_ReadDWORD(hKey, "ConfirmPowerActions", &dwValue))
        g_config.confirmPowerActions = (dwValue != 0);
    
    if (Registry_ReadDWORD(hKey, "PowerButtonHoldTime", &dwValue))
        g_config.powerButtonHoldTime = (int)dwValue;
    
    // Load custom settings
    Registry_ReadString(hKey, "CustomCommand1", g_config.customCommand1, sizeof(g_config.customCommand1));
    Registry_ReadString(hKey, "CustomCommand2", g_config.customCommand2, sizeof(g_config.customCommand2));
    Registry_ReadString(hKey, "CustomCommand3", g_config.customCommand3, sizeof(g_config.customCommand3));
    Registry_ReadString(hKey, "FavoriteApp1", g_config.favoriteApp1, sizeof(g_config.favoriteApp1));
    Registry_ReadString(hKey, "FavoriteApp2", g_config.favoriteApp2, sizeof(g_config.favoriteApp2));
    Registry_ReadString(hKey, "FavoriteApp3", g_config.favoriteApp3, sizeof(g_config.favoriteApp3));
    
    RegCloseKey(hKey);
    
    LOG_DEBUG("Configuration loaded from registry successfully");
    LOG_EXIT("return=true");
    return true;
}

bool Config_SaveToRegistry(void)
{
    LOG_ENTRY_SIMPLE();
    
    HKEY hKey;
    LONG result = RegCreateKeyExA(HKEY_CURRENT_USER, CONFIG_REGISTRY_KEY, 0, NULL, 
                                 REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);
    
    if (result != ERROR_SUCCESS)
    {
        LOG_ERROR("Failed to create/open registry key, error: %ld", result);
        LOG_EXIT("return=false");
        return false;
    }
    
    // Save application settings
    Registry_WriteDWORD(hKey, "StartWithWindows", g_config.startWithWindows ? 1 : 0);
    Registry_WriteDWORD(hKey, "MinimizeToTray", g_config.minimizeToTray ? 1 : 0);
    Registry_WriteDWORD(hKey, "ShowOverlayOnStart", g_config.showOverlayOnStart ? 1 : 0);
    Registry_WriteDWORD(hKey, "LogLevel", (DWORD)g_config.logLevel);
    
    // Save input settings
    Registry_WriteDWORD(hKey, "EnableGamepadInput", g_config.enableGamepadInput ? 1 : 0);
    Registry_WriteDWORD(hKey, "GamepadDeadzone", (DWORD)g_config.gamepadDeadzone);
    Registry_WriteDWORD(hKey, "GamepadVibrationLevel", (DWORD)g_config.gamepadVibrationLevel);
    
    // Save audio settings
    Registry_WriteDWORD(hKey, "VolumeIncrement", (DWORD)(g_config.volumeIncrement * 100));
    Registry_WriteDWORD(hKey, "MuteOnStartup", g_config.muteOnStartup ? 1 : 0);
    
    // Save display settings
    Registry_WriteDWORD(hKey, "BrightnessIncrement", (DWORD)g_config.brightnessIncrement);
    Registry_WriteDWORD(hKey, "TurnOffDisplayOnIdle", g_config.turnOffDisplayOnIdle ? 1 : 0);
    Registry_WriteDWORD(hKey, "IdleTimeout", (DWORD)g_config.idleTimeout);
    
    // Save power settings
    Registry_WriteDWORD(hKey, "ConfirmPowerActions", g_config.confirmPowerActions ? 1 : 0);
    Registry_WriteDWORD(hKey, "PowerButtonHoldTime", (DWORD)g_config.powerButtonHoldTime);
    
    // Save custom settings
    Registry_WriteString(hKey, "CustomCommand1", g_config.customCommand1);
    Registry_WriteString(hKey, "CustomCommand2", g_config.customCommand2);
    Registry_WriteString(hKey, "CustomCommand3", g_config.customCommand3);
    Registry_WriteString(hKey, "FavoriteApp1", g_config.favoriteApp1);
    Registry_WriteString(hKey, "FavoriteApp2", g_config.favoriteApp2);
    Registry_WriteString(hKey, "FavoriteApp3", g_config.favoriteApp3);
    
    RegCloseKey(hKey);
    
    LOG_DEBUG("Configuration saved to registry successfully");
    LOG_EXIT("return=true");
    return true;
}

bool Config_LoadFromFile(const char* filePath)
{
    LOG_ENTRY("filePath=%s", filePath ? filePath : "NULL");
    
    if (!filePath)
    {
        LOG_ERROR("Invalid file path");
        LOG_EXIT("return=false");
        return false;
    }
    
    FILE* file;
    errno_t err = fopen_s(&file, filePath, "r");
    
    if (err != 0 || !file)
    {
        LOG_DEBUG("Configuration file not found or cannot be opened: %s", filePath);
        LOG_EXIT("return=false");
        return false;
    }
    
    char line[512];
    char key[128];
    char value[384];
    
    while (fgets(line, sizeof(line), file))
    {
        // Skip comments and empty lines
        if (line[0] == '#' || line[0] == ';' || line[0] == '\n' || line[0] == '\r')
            continue;
        
        // Parse key=value pairs
        if (sscanf_s(line, "%127[^=]=%383[^\n\r]", key, (unsigned)sizeof(key), value, (unsigned)sizeof(value)) == 2)
        {
            // Trim whitespace
            char* trimmedKey = key;
            while (*trimmedKey == ' ' || *trimmedKey == '\t') trimmedKey++;
            
            char* trimmedValue = value;
            while (*trimmedValue == ' ' || *trimmedValue == '\t') trimmedValue++;
            
            // Remove trailing whitespace
            size_t len = strlen(trimmedValue);
            while (len > 0 && (trimmedValue[len-1] == ' ' || trimmedValue[len-1] == '\t'))
            {
                trimmedValue[--len] = '\0';
            }
            
            // Set configuration values based on key
            if (_stricmp(trimmedKey, "StartWithWindows") == 0)
                g_config.startWithWindows = (_stricmp(trimmedValue, "true") == 0 || _stricmp(trimmedValue, "1") == 0);
            else if (_stricmp(trimmedKey, "MinimizeToTray") == 0)
                g_config.minimizeToTray = (_stricmp(trimmedValue, "true") == 0 || _stricmp(trimmedValue, "1") == 0);
            else if (_stricmp(trimmedKey, "ShowOverlayOnStart") == 0)
                g_config.showOverlayOnStart = (_stricmp(trimmedValue, "true") == 0 || _stricmp(trimmedValue, "1") == 0);
            else if (_stricmp(trimmedKey, "LogLevel") == 0)
                g_config.logLevel = atoi(trimmedValue);
            else if (_stricmp(trimmedKey, "EnableGamepadInput") == 0)
                g_config.enableGamepadInput = (_stricmp(trimmedValue, "true") == 0 || _stricmp(trimmedValue, "1") == 0);
            else if (_stricmp(trimmedKey, "GamepadDeadzone") == 0)
                g_config.gamepadDeadzone = atoi(trimmedValue);
            else if (_stricmp(trimmedKey, "GamepadVibrationLevel") == 0)
                g_config.gamepadVibrationLevel = atoi(trimmedValue);
            else if (_stricmp(trimmedKey, "VolumeIncrement") == 0)
                g_config.volumeIncrement = (float)atof(trimmedValue);
            else if (_stricmp(trimmedKey, "MuteOnStartup") == 0)
                g_config.muteOnStartup = (_stricmp(trimmedValue, "true") == 0 || _stricmp(trimmedValue, "1") == 0);
            else if (_stricmp(trimmedKey, "BrightnessIncrement") == 0)
                g_config.brightnessIncrement = atoi(trimmedValue);
            else if (_stricmp(trimmedKey, "TurnOffDisplayOnIdle") == 0)
                g_config.turnOffDisplayOnIdle = (_stricmp(trimmedValue, "true") == 0 || _stricmp(trimmedValue, "1") == 0);
            else if (_stricmp(trimmedKey, "IdleTimeout") == 0)
                g_config.idleTimeout = atoi(trimmedValue);
            else if (_stricmp(trimmedKey, "ConfirmPowerActions") == 0)
                g_config.confirmPowerActions = (_stricmp(trimmedValue, "true") == 0 || _stricmp(trimmedValue, "1") == 0);
            else if (_stricmp(trimmedKey, "PowerButtonHoldTime") == 0)
                g_config.powerButtonHoldTime = atoi(trimmedValue);
            else if (_stricmp(trimmedKey, "CustomCommand1") == 0)
                strcpy_s(g_config.customCommand1, sizeof(g_config.customCommand1), trimmedValue);
            else if (_stricmp(trimmedKey, "CustomCommand2") == 0)
                strcpy_s(g_config.customCommand2, sizeof(g_config.customCommand2), trimmedValue);
            else if (_stricmp(trimmedKey, "CustomCommand3") == 0)
                strcpy_s(g_config.customCommand3, sizeof(g_config.customCommand3), trimmedValue);
            else if (_stricmp(trimmedKey, "FavoriteApp1") == 0)
                strcpy_s(g_config.favoriteApp1, sizeof(g_config.favoriteApp1), trimmedValue);
            else if (_stricmp(trimmedKey, "FavoriteApp2") == 0)
                strcpy_s(g_config.favoriteApp2, sizeof(g_config.favoriteApp2), trimmedValue);
            else if (_stricmp(trimmedKey, "FavoriteApp3") == 0)
                strcpy_s(g_config.favoriteApp3, sizeof(g_config.favoriteApp3), trimmedValue);
        }
    }
    
    fclose(file);
    
    LOG_DEBUG("Configuration loaded from file successfully");
    LOG_EXIT("return=true");
    return true;
}

bool Config_SaveToFile(const char* filePath)
{
    LOG_ENTRY("filePath=%s", filePath ? filePath : "NULL");
    
    if (!filePath)
    {
        LOG_ERROR("Invalid file path");
        LOG_EXIT("return=false");
        return false;
    }
    
    FILE* file;
    errno_t err = fopen_s(&file, filePath, "w");
    
    if (err != 0 || !file)
    {
        LOG_ERROR("Failed to create configuration file: %s", filePath);
        LOG_EXIT("return=false");
        return false;
    }
    
    fprintf(file, "# GPDesk Configuration File\n");
    fprintf(file, "# This file is automatically generated\n\n");
    
    fprintf(file, "[Application Settings]\n");
    fprintf(file, "StartWithWindows=%s\n", g_config.startWithWindows ? "true" : "false");
    fprintf(file, "MinimizeToTray=%s\n", g_config.minimizeToTray ? "true" : "false");
    fprintf(file, "ShowOverlayOnStart=%s\n", g_config.showOverlayOnStart ? "true" : "false");
    fprintf(file, "LogLevel=%d\n", g_config.logLevel);
    
    fprintf(file, "\n[Input Settings]\n");
    fprintf(file, "EnableGamepadInput=%s\n", g_config.enableGamepadInput ? "true" : "false");
    fprintf(file, "GamepadDeadzone=%d\n", g_config.gamepadDeadzone);
    fprintf(file, "GamepadVibrationLevel=%d\n", g_config.gamepadVibrationLevel);
    
    fprintf(file, "\n[Audio Settings]\n");
    fprintf(file, "VolumeIncrement=%.2f\n", g_config.volumeIncrement);
    fprintf(file, "MuteOnStartup=%s\n", g_config.muteOnStartup ? "true" : "false");
    
    fprintf(file, "\n[Display Settings]\n");
    fprintf(file, "BrightnessIncrement=%d\n", g_config.brightnessIncrement);
    fprintf(file, "TurnOffDisplayOnIdle=%s\n", g_config.turnOffDisplayOnIdle ? "true" : "false");
    fprintf(file, "IdleTimeout=%d\n", g_config.idleTimeout);
    
    fprintf(file, "\n[Power Settings]\n");
    fprintf(file, "ConfirmPowerActions=%s\n", g_config.confirmPowerActions ? "true" : "false");
    fprintf(file, "PowerButtonHoldTime=%d\n", g_config.powerButtonHoldTime);
    
    fprintf(file, "\n[Custom Settings]\n");
    fprintf(file, "CustomCommand1=%s\n", g_config.customCommand1);
    fprintf(file, "CustomCommand2=%s\n", g_config.customCommand2);
    fprintf(file, "CustomCommand3=%s\n", g_config.customCommand3);
    fprintf(file, "FavoriteApp1=%s\n", g_config.favoriteApp1);
    fprintf(file, "FavoriteApp2=%s\n", g_config.favoriteApp2);
    fprintf(file, "FavoriteApp3=%s\n", g_config.favoriteApp3);
    
    fclose(file);
    
    LOG_DEBUG("Configuration saved to file successfully");
    LOG_EXIT("return=true");
    return true;
}

bool Config_ValidateSettings(void)
{
    LOG_ENTRY_SIMPLE();
    
    bool isValid = true;
    
    // Validate ranges
    if (g_config.logLevel < 0 || g_config.logLevel > 3)
    {
        LOG_WARNING("Invalid log level: %d, resetting to 0", g_config.logLevel);
        g_config.logLevel = 0;
        isValid = false;
    }
    
    if (g_config.gamepadDeadzone < 0 || g_config.gamepadDeadzone > 32767)
    {
        LOG_WARNING("Invalid gamepad deadzone: %d, resetting to %d", g_config.gamepadDeadzone, INPUT_DEADZONE);
        g_config.gamepadDeadzone = INPUT_DEADZONE;
        isValid = false;
    }
    
    if (g_config.gamepadVibrationLevel < 0 || g_config.gamepadVibrationLevel > 100)
    {
        LOG_WARNING("Invalid vibration level: %d, resetting to 50", g_config.gamepadVibrationLevel);
        g_config.gamepadVibrationLevel = 50;
        isValid = false;
    }
    
    if (g_config.volumeIncrement < 0.01f || g_config.volumeIncrement > 1.0f)
    {
        LOG_WARNING("Invalid volume increment: %.2f, resetting to 0.1", g_config.volumeIncrement);
        g_config.volumeIncrement = 0.1f;
        isValid = false;
    }
    
    if (g_config.brightnessIncrement < 1 || g_config.brightnessIncrement > 100)
    {
        LOG_WARNING("Invalid brightness increment: %d, resetting to 10", g_config.brightnessIncrement);
        g_config.brightnessIncrement = 10;
        isValid = false;
    }
    
    if (g_config.idleTimeout < 10 || g_config.idleTimeout > 3600)
    {
        LOG_WARNING("Invalid idle timeout: %d, resetting to 300", g_config.idleTimeout);
        g_config.idleTimeout = 300;
        isValid = false;
    }
    
    if (g_config.powerButtonHoldTime < 100 || g_config.powerButtonHoldTime > 10000)
    {
        LOG_WARNING("Invalid power button hold time: %d, resetting to 2000", g_config.powerButtonHoldTime);
        g_config.powerButtonHoldTime = 2000;
        isValid = false;
    }
    
    LOG_EXIT("return=%d", isValid);
    return isValid;
}

void Config_ApplySettings(void)
{
    LOG_ENTRY_SIMPLE();
    
    // Apply log level
    Logger_SetLevel((LogLevel)g_config.logLevel);
    
    LOG_INFO("Configuration settings applied");
    LOG_DEBUG("Settings: startWithWindows=%d, minimizeToTray=%d, showOverlayOnStart=%d", 
             g_config.startWithWindows, g_config.minimizeToTray, g_config.showOverlayOnStart);
    LOG_DEBUG("Volume increment: %.2f, Brightness increment: %d", 
             g_config.volumeIncrement, g_config.brightnessIncrement);
    
    LOG_EXIT_SIMPLE();
}

// Registry helper functions implementation
bool Registry_ReadString(HKEY hKey, const char* valueName, char* buffer, DWORD bufferSize)
{
    if (!valueName || !buffer || bufferSize == 0)
    {
        return false;
    }
    
    DWORD type = REG_SZ;
    LONG result = RegQueryValueExA(hKey, valueName, NULL, &type, (LPBYTE)buffer, &bufferSize);
    
    return (result == ERROR_SUCCESS && type == REG_SZ);
}

bool Registry_WriteString(HKEY hKey, const char* valueName, const char* value)
{
    if (!valueName || !value)
    {
        return false;
    }
    
    LONG result = RegSetValueExA(hKey, valueName, 0, REG_SZ, (const BYTE*)value, (DWORD)(strlen(value) + 1));
    
    return (result == ERROR_SUCCESS);
}

bool Registry_ReadDWORD(HKEY hKey, const char* valueName, DWORD* value)
{
    if (!valueName || !value)
    {
        return false;
    }
    
    DWORD type = REG_DWORD;
    DWORD size = sizeof(DWORD);
    LONG result = RegQueryValueExA(hKey, valueName, NULL, &type, (LPBYTE)value, &size);
    
    return (result == ERROR_SUCCESS && type == REG_DWORD);
}

bool Registry_WriteDWORD(HKEY hKey, const char* valueName, DWORD value)
{
    if (!valueName)
    {
        return false;
    }
    
    LONG result = RegSetValueExA(hKey, valueName, 0, REG_DWORD, (const BYTE*)&value, sizeof(DWORD));
    
    return (result == ERROR_SUCCESS);
}