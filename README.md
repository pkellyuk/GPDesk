# GPDesk - Gamepad PC Control System

GPDesk is a C/Win32 application that allows you to control your PC using a gamepad. It provides desktop mouse control, browser navigation, system shortcuts, and more - all from your Xbox-compatible controller.

## Download

**[Download the latest release (v1.0.0)](https://github.com/pkellyuk/GPDesk/releases/latest)** - No compilation required!

Simply download `gpdesk.exe` and run it. The application will appear in your system tray.

### ⚠️ About Security Warnings

Browsers and Windows may show security warnings because this executable is:
- **Not code-signed** (code signing certificates cost $300-500/year)
- **New to Windows SmartScreen** (hasn't been downloaded by enough users yet)

**This is a false positive.** The source code is completely open and available for review in this repository.

**To download safely:**
1. Click the download link
2. Your browser may warn you - click **"Keep"** or **"Download anyway"**
3. Windows SmartScreen may appear - click **"More info"** then **"Run anyway"**

**Alternative:** [Build from source](#building-from-source) yourself to avoid any warnings.

## Features

### Desktop Control Mode
- **Right Stick**: Mouse movement
- **Left Stick**: Smooth scrolling (vertical)
- **D-Pad**: Line-by-line scrolling (up/down)
- **Right Trigger**: Left mouse click
- **Left Trigger**: Right mouse click

### Button Controls
- **START + BACK**: Toggle overlay (press both simultaneously)
- **A Button**: Toggle between Desktop and TV modes (when overlay is visible)
- **X Button**: Show/hide Windows On-Screen Keyboard
- **Y Button (Hold 2s)**: Put PC to sleep
- **Left Shoulder**: Browser back
- **Right Shoulder**: Browser forward

### System Features
- **XInput Gamepad Support**: Full support for Xbox controllers and compatible gamepads
- **System Tray Integration**: Runs in background, accessible from system tray
- **Automatic Gamepad Detection**: Instantly detects when controller is connected
- **Low Resource Usage**: Minimal CPU and memory footprint

## Administrator Privileges

GPDesk requests administrator privileges for the following reasons:
- **System-wide input injection**: Required to send mouse and keyboard events to any application, including elevated programs
- **Task Manager compatibility**: Allows gamepad control to work even when Task Manager or other elevated applications are in focus
- **Power management**: Sleep and hibernation features require elevated permissions

The application does not modify system files or settings without user interaction.

## Requirements

- Windows 10/11
- XInput-compatible gamepad (Xbox 360, Xbox One, Xbox Series X/S controllers)
- No additional dependencies required

## Building from Source

```bash
# Using GCC (MinGW)
gcc -o build/bin/gpdesk.exe -Iinclude src/core/*.c src/input/*.c src/system/*.c src/config/*.c -lkernel32 -luser32 -lgdi32 -lcomctl32 -lole32 -loleaut32 -luuid -lshell32 -ladvapi32 -lwinmm -lxinput -lpowrprof -ldxva2 -DWIN32_LEAN_AND_MEAN -D_WIN32_WINNT=0x0601 -std=c17 -Wall

# Or use the build script
build.bat
```

## Usage

1. Run `gpdesk.exe`
2. The application icon will appear in your system tray
3. Connect your Xbox-compatible gamepad
4. Press **START + BACK** together to show the overlay
5. Use your gamepad to control the desktop!

## Architecture

The project follows a modular design with separate systems for:
- **Input Handling** (`src/input/`): XInput gamepad state management and button mapping
- **System Control** (`src/system/`): Audio, power, display, and application control
- **Core** (`src/core/`): Main application loop and logging
- **Configuration** (`src/config/`): Settings management

All code follows Allman brace style with comprehensive logging and null-checking.

## License

MIT License - See [LICENSE](LICENSE) file for details.

---

🤖 Built with [Claude Code](https://claude.com/claude-code)
