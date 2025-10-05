# GPDesk - Gamepad PC Control System

GPDesk is a C/Win32 application that allows you to control your PC and TV using a gamepad, similar to ASUS Armory Crate SE. It provides system-wide gamepad control for volume, brightness, power management, application launching, and more.

## Features

### Core Functionality
- **XInput Gamepad Support**: Full support for Xbox controllers and compatible gamepads
- **System Control**: Volume, brightness, power management (shutdown, sleep, hibernate)
- **Application Control**: Launch applications, execute custom commands
- **Overlay Interface**: On-screen display for gamepad navigation
- **System Tray Integration**: Minimize to tray, context menu controls
- **Configuration Management**: Registry and file-based settings storage

### Default Button Mapping
- **Start**: Toggle overlay
- **D-Pad Up/Down**: Volume up/down
- **D-Pad Left/Right**: Brightness down/up  
- **Back**: Mute toggle
- **Y (Hold 2s)**: Sleep mode
- **X**: TV power control
- **A/B**: Configurable actions

## Building

```bash
# Debug build
make debug

# Release build  
make release

# Build and run
make run
```

## Requirements
- Windows 7 or later
- XInput-compatible gamepad
- MinGW-w64 or Visual Studio Build Tools
- Administrator privileges for system control

## Architecture
The project follows a modular design with separate systems for input handling, system control, configuration management, and logging. All code follows Allman brace style with comprehensive logging and null-checking as specified in your requirements.