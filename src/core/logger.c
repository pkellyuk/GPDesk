#include "logger.h"

static LogLevel g_currentLogLevel = LOG_LEVEL_DEBUG;
static HANDLE g_logFile = INVALID_HANDLE_VALUE;
static CRITICAL_SECTION g_logLock;
static BOOL g_initialized = FALSE;

void Logger_Initialize(void)
{
    if (g_initialized)
    {
        return;
    }

    InitializeCriticalSection(&g_logLock);
    
    // Create log file with timestamp
    char logFileName[MAX_PATH];
    SYSTEMTIME st;
    GetLocalTime(&st);
    
    sprintf_s(logFileName, sizeof(logFileName), 
              "logs\\gpdesk_%04d%02d%02d_%02d%02d%02d.log",
              st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
    
    // Create logs directory if it doesn't exist
    CreateDirectoryA("logs", NULL);
    
    g_logFile = CreateFileA(logFileName,
                           GENERIC_WRITE,
                           FILE_SHARE_READ,
                           NULL,
                           CREATE_ALWAYS,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL);
    
    g_initialized = TRUE;
    
    LOG_INFO("Logger initialized, log file: %s", logFileName);
}

void Logger_Cleanup(void)
{
    if (!g_initialized)
    {
        return;
    }
    
    LOG_INFO("Logger shutting down");
    
    if (g_logFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(g_logFile);
        g_logFile = INVALID_HANDLE_VALUE;
    }
    
    DeleteCriticalSection(&g_logLock);
    g_initialized = FALSE;
}

void Logger_SetLevel(LogLevel level)
{
    if (level < LOG_LEVEL_DEBUG || level > LOG_LEVEL_ERROR)
    {
        return;
    }
    
    g_currentLogLevel = level;
    LOG_INFO("Log level set to %d", level);
}

void Logger_Log(LogLevel level, const char* function, int line, const char* format, ...)
{
    if (!g_initialized || !function || !format)
    {
        return;
    }
    
    if (level < g_currentLogLevel)
    {
        return;
    }
    
    EnterCriticalSection(&g_logLock);
    
    // Get timestamp
    SYSTEMTIME st;
    GetLocalTime(&st);
    
    // Format log level string
    const char* levelStr;
    switch (level)
    {
        case LOG_LEVEL_DEBUG:
            levelStr = "DEBUG";
            break;
        case LOG_LEVEL_INFO:
            levelStr = "INFO";
            break;
        case LOG_LEVEL_WARNING:
            levelStr = "WARN";
            break;
        case LOG_LEVEL_ERROR:
            levelStr = "ERROR";
            break;
        default:
            levelStr = "UNKNOWN";
            break;
    }
    
    // Format the message
    va_list args;
    va_start(args, format);
    
    char message[1024];
    vsnprintf_s(message, sizeof(message), _TRUNCATE, format, args);
    va_end(args);
    
    // Create full log entry
    char logEntry[2048];
    sprintf_s(logEntry, sizeof(logEntry),
              "[%04d-%02d-%02d %02d:%02d:%02d.%03d] [%s] [%s:%d] %s\r\n",
              st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
              levelStr, function, line, message);
    
    // Write to debug output (visible in debugger)
    OutputDebugStringA(logEntry);
    
    // Write to console
    printf("%s", logEntry);
    
    // Write to file
    if (g_logFile != INVALID_HANDLE_VALUE)
    {
        DWORD bytesWritten;
        WriteFile(g_logFile, logEntry, (DWORD)strlen(logEntry), &bytesWritten, NULL);
        FlushFileBuffers(g_logFile);
    }
    
    LeaveCriticalSection(&g_logLock);
}

void Logger_LogFunctionEntry(const char* function, const char* format, ...)
{
    if (!g_initialized || !function)
    {
        return;
    }
    
    char message[512] = "";
    
    if (format && strlen(format) > 0)
    {
        va_list args;
        va_start(args, format);
        vsnprintf_s(message, sizeof(message), _TRUNCATE, format, args);
        va_end(args);
        
        Logger_Log(LOG_LEVEL_DEBUG, function, 0, "ENTRY -> %s", message);
    }
    else
    {
        Logger_Log(LOG_LEVEL_DEBUG, function, 0, "ENTRY");
    }
}

void Logger_LogFunctionExit(const char* function, const char* format, ...)
{
    if (!g_initialized || !function)
    {
        return;
    }
    
    char message[512] = "";
    
    if (format && strlen(format) > 0)
    {
        va_list args;
        va_start(args, format);
        vsnprintf_s(message, sizeof(message), _TRUNCATE, format, args);
        va_end(args);
        
        Logger_Log(LOG_LEVEL_DEBUG, function, 0, "EXIT <- %s", message);
    }
    else
    {
        Logger_Log(LOG_LEVEL_DEBUG, function, 0, "EXIT");
    }
}