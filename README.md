# GPDesk - Gamepad PC Control System

GPDesk is a C/Win32 application that allows you to control your PC using a gamepad. It provides desktop mouse control, browser navigation, system shortcuts, and more - all from your Xbox-compatible controller.

## Download

**[Download the latest release (v1.0.0)](https://github.com/pkellyuk/GPDesk/releases/latest)** - No compilation required!

Simply download `gpdesk.exe` and run it. The application will appear in your system tray.

### ⚠️ About Security Warnings (FALSE POSITIVE)

**Windows Defender may flag this as `Trojan:Win32/Wacatac.C!ml` - This is a FALSE POSITIVE.**

This happens because GPDesk:
- **Uses SendInput() API** - Required for mouse/keyboard control from gamepad
- **Requests admin privileges** - Needed to work with elevated applications
- **Is unsigned** - Code signing certificates cost $300-500/year
- **Is new** - Not yet recognized by Windows SmartScreen

**Why this is safe:**
- 🔓 **100% open source** - All code is visible in this repository
- 🛡️ **No network activity** - The app doesn't connect to the internet
- 📝 **Simple C code** - Easy to audit and verify
- 🎮 **Legitimate purpose** - Uses standard Windows APIs for gamepad input control

**To download safely:**
1. Click the download link
2. Your browser may warn you - click **"Keep"** or **"Download anyway"**
3. Windows SmartScreen may appear - click **"More info"** then **"Run anyway"**
4. Windows Defender may quarantine it - add an exclusion for the file

**Best option if concerned:** [Build from source](#building-from-source) yourself using the exact commands shown below. You'll see it's just standard C code with no malicious behavior.

**Future plans:** We are applying for free code signing through [SignPath Foundation](https://signpath.org/) which will eliminate these warnings. Code signing certificates typically cost $300-500/year, but SignPath provides them free for open source projects.

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

## Code Signing

This project is working towards getting free code signing through [SignPath Foundation](https://signpath.org/), which provides code signing certificates at no cost to open source projects.

### Why Code Signing Matters

Code signing eliminates false positive virus warnings from Windows Defender and browsers, making it easier for users to download and trust the software.

### Current Status

- ✅ GitHub Actions CI/CD workflow created
- ✅ Automated builds working
- ⏳ Applying for SignPath Foundation certificate
- ⏳ Waiting for approval

Once approved, all releases will be automatically signed during the build process.

### For Contributors

The signing process is fully automated through GitHub Actions. When code is merged to `main` or a version tag is pushed, the workflow:
1. Builds the executable
2. Submits to SignPath for signing (once configured)
3. Creates a GitHub release with the signed binary

No manual signing steps are required.

## License

MIT License - See [LICENSE](LICENSE) file for details.

---

🤖 Built with [Claude Code](https://claude.com/claude-code)
