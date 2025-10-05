@echo off
echo Building GPDesk...

REM Create build directories
if not exist "build" mkdir "build"
if not exist "build\obj" mkdir "build\obj"
if not exist "build\obj\core" mkdir "build\obj\core"
if not exist "build\obj\input" mkdir "build\obj\input"
if not exist "build\obj\system" mkdir "build\obj\system"
if not exist "build\obj\config" mkdir "build\obj\config"
if not exist "build\bin" mkdir "build\bin"

REM Setup Visual Studio environment
call "%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

REM Compiler flags
set CFLAGS=/I"include" /DWIN32_LEAN_AND_MEAN /D_WIN32_WINNT=0x0601 /std:c17 /W4
set LDFLAGS=kernel32.lib user32.lib gdi32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib shell32.lib advapi32.lib winmm.lib xinput.lib powrprof.lib dxva2.lib

REM Compile source files
echo Compiling core files...
cl %CFLAGS% /c "src\core\main.c" /Fo"build\obj\core\main.obj"
if errorlevel 1 goto error

cl %CFLAGS% /c "src\core\logger.c" /Fo"build\obj\core\logger.obj"
if errorlevel 1 goto error

echo Compiling input files...
cl %CFLAGS% /c "src\input\gamepad.c" /Fo"build\obj\input\gamepad.obj"
if errorlevel 1 goto error

echo Compiling system files...
cl %CFLAGS% /c "src\system\audio_control.c" /Fo"build\obj\system\audio_control.obj"
if errorlevel 1 goto error

cl %CFLAGS% /c "src\system\power_control.c" /Fo"build\obj\system\power_control.obj"
if errorlevel 1 goto error

cl %CFLAGS% /c "src\system\display_control.c" /Fo"build\obj\system\display_control.obj"
if errorlevel 1 goto error

cl %CFLAGS% /c "src\system\app_control.c" /Fo"build\obj\system\app_control.obj"
if errorlevel 1 goto error

echo Compiling config files...
cl %CFLAGS% /c "src\config\config.c" /Fo"build\obj\config\config.obj"
if errorlevel 1 goto error

REM Link executable
echo Linking executable...
link /OUT:"build\bin\gpdesk.exe" build\obj\core\*.obj build\obj\input\*.obj build\obj\system\*.obj build\obj\config\*.obj %LDFLAGS%
if errorlevel 1 goto error

echo Build completed successfully!
echo Executable: build\bin\gpdesk.exe
goto end

:error
echo Build failed!
exit /b 1

:end