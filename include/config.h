#ifndef CONFIG_H
#define CONFIG_H

#include <windows.h>
#include <stdbool.h>
#include "logger.h"
#include "input.h"

// Configuration constants
#define CONFIG_REGISTRY_KEY "SOFTWARE\\GPDesk"
#define CONFIG_FILE_NAME "gpdesk_config.ini"
#define MAX_CONFIG_VALUE_LENGTH 512

// Configuration structure
typedef struct
{
    // Application settings
    bool startWithWindows;
    bool minimizeToTray;
    bool showOverlayOnStart;
    int logLevel;
    
    // Input settings
    bool enableGamepadInput;
    int gamepadDeadzone;
    int gamepadVibrationLevel;
    
    // Audio settings
    float volumeIncrement;
    bool muteOnStartup;
    
    // Display settings
    int brightnessIncrement;
    bool turnOffDisplayOnIdle;
    int idleTimeout;
    
    // Power settings
    bool confirmPowerActions;
    int powerButtonHoldTime;
    
    // Custom settings
    char customCommand1[MAX_CONFIG_VALUE_LENGTH];
    char customCommand2[MAX_CONFIG_VALUE_LENGTH];
    char customCommand3[MAX_CONFIG_VALUE_LENGTH];
    char favoriteApp1[MAX_CONFIG_VALUE_LENGTH];
    char favoriteApp2[MAX_CONFIG_VALUE_LENGTH];
    char favoriteApp3[MAX_CONFIG_VALUE_LENGTH];
    
    bool isLoaded;
} Config;

// Function prototypes
bool Config_Initialize(void);
void Config_Cleanup(void);
bool Config_Load(void);
bool Config_Save(void);
bool Config_LoadFromRegistry(void);
bool Config_SaveToRegistry(void);
bool Config_LoadFromFile(const char* filePath);
bool Config_SaveToFile(const char* filePath);
void Config_SetDefaults(void);

// Getter functions
bool Config_GetBool(const char* key, bool defaultValue);
int Config_GetInt(const char* key, int defaultValue);
float Config_GetFloat(const char* key, float defaultValue);
const char* Config_GetString(const char* key, const char* defaultValue);

// Setter functions
bool Config_SetBool(const char* key, bool value);
bool Config_SetInt(const char* key, int value);
bool Config_SetFloat(const char* key, float value);
bool Config_SetString(const char* key, const char* value);

// Registry helper functions
bool Registry_ReadString(HKEY hKey, const char* valueName, char* buffer, DWORD bufferSize);
bool Registry_WriteString(HKEY hKey, const char* valueName, const char* value);
bool Registry_ReadDWORD(HKEY hKey, const char* valueName, DWORD* value);
bool Registry_WriteDWORD(HKEY hKey, const char* valueName, DWORD value);

// File helper functions
bool File_ReadString(FILE* file, const char* key, char* buffer, size_t bufferSize);
bool File_WriteString(FILE* file, const char* key, const char* value);

// Configuration validation
bool Config_ValidateSettings(void);
void Config_ApplySettings(void);

// Global configuration instance
extern Config g_config;

#endif // CONFIG_H