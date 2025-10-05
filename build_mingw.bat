@echo off
echo Building GPDesk with MinGW-w64...

REM Create build directories
if not exist "build" mkdir "build"
if not exist "build\obj" mkdir "build\obj"
if not exist "build\obj\core" mkdir "build\obj\core"
if not exist "build\obj\input" mkdir "build\obj\input"
if not exist "build\obj\system" mkdir "build\obj\system"
if not exist "build\obj\config" mkdir "build\obj\config"
if not exist "build\bin" mkdir "build\bin"

REM Compiler flags
set CFLAGS=-std=c99 -Wall -Wextra -Iinclude -DWIN32_LEAN_AND_MEAN -D_WIN32_WINNT=0x0601
set LDFLAGS=-lkernel32 -luser32 -lgdi32 -lcomctl32 -lole32 -loleaut32 -luuid -lshell32 -ladvapi32 -lwinmm -lxinput -lpowrprof -ldxva2

REM Compile source files
echo Compiling core files...
gcc %CFLAGS% -c "src\core\main.c" -o "build\obj\core\main.o"
if errorlevel 1 goto error

gcc %CFLAGS% -c "src\core\logger.c" -o "build\obj\core\logger.o"
if errorlevel 1 goto error

echo Compiling input files...
gcc %CFLAGS% -c "src\input\gamepad.c" -o "build\obj\input\gamepad.o"
if errorlevel 1 goto error

echo Compiling system files...
gcc %CFLAGS% -c "src\system\audio_control.c" -o "build\obj\system\audio_control.o"
if errorlevel 1 goto error

gcc %CFLAGS% -c "src\system\power_control.c" -o "build\obj\system\power_control.o"
if errorlevel 1 goto error

gcc %CFLAGS% -c "src\system\display_control.c" -o "build\obj\system\display_control.o"
if errorlevel 1 goto error

gcc %CFLAGS% -c "src\system\app_control.c" -o "build\obj\system\app_control.o"
if errorlevel 1 goto error

echo Compiling config files...
gcc %CFLAGS% -c "src\config\config.c" -o "build\obj\config\config.o"
if errorlevel 1 goto error

REM Link executable
echo Linking executable...
gcc -o "build\bin\gpdesk.exe" build\obj\core\*.o build\obj\input\*.o build\obj\system\*.o build\obj\config\*.o %LDFLAGS%
if errorlevel 1 goto error

echo Build completed successfully!
echo Executable: build\bin\gpdesk.exe
goto end

:error
echo Build failed!
exit /b 1

:end