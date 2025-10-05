#ifndef LOGGER_H
#define LOGGER_H

#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

// Log levels
typedef enum
{
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO = 1,
    LOG_LEVEL_WARNING = 2,
    LOG_LEVEL_ERROR = 3
} LogLevel;

// Function prototypes
void Logger_Initialize(void);
void Logger_Cleanup(void);
void Logger_SetLevel(LogLevel level);
void Logger_Log(LogLevel level, const char* function, int line, const char* format, ...);
void Logger_LogFunctionEntry(const char* function, const char* format, ...);
void Logger_LogFunctionExit(const char* function, const char* format, ...);

// Convenience macros following your requirements
#define LOG_DEBUG(format, ...) Logger_Log(LOG_LEVEL_DEBUG, __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define LOG_INFO(format, ...) Logger_Log(LOG_LEVEL_INFO, __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define LOG_WARNING(format, ...) Logger_Log(LOG_LEVEL_WARNING, __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) Logger_Log(LOG_LEVEL_ERROR, __FUNCTION__, __LINE__, format, ##__VA_ARGS__)

#define LOG_ENTRY(format, ...) Logger_LogFunctionEntry(__FUNCTION__, format, ##__VA_ARGS__)
#define LOG_EXIT(format, ...) Logger_LogFunctionExit(__FUNCTION__, format, ##__VA_ARGS__)

// Simple entry/exit without parameters
#define LOG_ENTRY_SIMPLE() Logger_LogFunctionEntry(__FUNCTION__, "")
#define LOG_EXIT_SIMPLE() Logger_LogFunctionExit(__FUNCTION__, "")

#endif // LOGGER_H