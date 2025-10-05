# GPDesk Makefile for Windows
# Requires MinGW-w64 or Visual Studio Build Tools

# Compiler settings
CC = gcc
RC = windres
CFLAGS = -std=c99 -Wall -Wextra -Iinclude -DWIN32_LEAN_AND_MEAN -D_WIN32_WINNT=0x0601
LDFLAGS = -lkernel32 -luser32 -lgdi32 -lcomctl32 -lole32 -loleaut32 -luuid -lshell32 -ladvapi32 -lwinmm -lxinput -lpowrprof -ldxva2 -mwindows

# Debug and release settings
DEBUG_CFLAGS = $(CFLAGS) -g -DDEBUG -D_DEBUG
RELEASE_CFLAGS = $(CFLAGS) -O2 -DNDEBUG

# Directories
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj
BIN_DIR = $(BUILD_DIR)/bin

# Source files
CORE_SOURCES = $(SRC_DIR)/core/main.c $(SRC_DIR)/core/logger.c
INPUT_SOURCES = $(SRC_DIR)/input/gamepad.c
SYSTEM_SOURCES = $(SRC_DIR)/system/audio_control.c $(SRC_DIR)/system/power_control.c $(SRC_DIR)/system/display_control.c $(SRC_DIR)/system/app_control.c
CONFIG_SOURCES = $(SRC_DIR)/config/config.c

ALL_SOURCES = $(CORE_SOURCES) $(INPUT_SOURCES) $(SYSTEM_SOURCES) $(CONFIG_SOURCES)

# Resource files
RESOURCE_FILE = gpdesk.rc
RESOURCE_OBJECT = $(OBJ_DIR)/gpdesk.res

# Object files
CORE_OBJECTS = $(CORE_SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
INPUT_OBJECTS = $(INPUT_SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
SYSTEM_OBJECTS = $(SYSTEM_SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
CONFIG_OBJECTS = $(CONFIG_SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

ALL_OBJECTS = $(CORE_OBJECTS) $(INPUT_OBJECTS) $(SYSTEM_OBJECTS) $(CONFIG_OBJECTS) $(RESOURCE_OBJECT)

# Target executable
TARGET = $(BIN_DIR)/gpdesk.exe
TARGET_DEBUG = $(BIN_DIR)/gpdesk_debug.exe

# Default target
all: release

# Create directories
$(OBJ_DIR):
	@if not exist "$(OBJ_DIR)" mkdir "$(OBJ_DIR)"
	@if not exist "$(OBJ_DIR)\core" mkdir "$(OBJ_DIR)\core"
	@if not exist "$(OBJ_DIR)\input" mkdir "$(OBJ_DIR)\input"
	@if not exist "$(OBJ_DIR)\system" mkdir "$(OBJ_DIR)\system"
	@if not exist "$(OBJ_DIR)\config" mkdir "$(OBJ_DIR)\config"

$(BIN_DIR):
	@if not exist "$(BIN_DIR)" mkdir "$(BIN_DIR)"

# Debug build
debug: CFLAGS = $(DEBUG_CFLAGS)
debug: $(TARGET_DEBUG)

$(TARGET_DEBUG): $(OBJ_DIR) $(BIN_DIR) $(ALL_OBJECTS)
	@echo Linking debug executable...
	$(CC) $(ALL_OBJECTS) -o $@ $(LDFLAGS)
	@echo Debug build completed: $@

# Release build
release: CFLAGS = $(RELEASE_CFLAGS)
release: $(TARGET)

$(TARGET): $(OBJ_DIR) $(BIN_DIR) $(ALL_OBJECTS)
	@echo Linking release executable...
	$(CC) $(ALL_OBJECTS) -o $@ $(LDFLAGS)
	@echo Release build completed: $@

# Compile source files
$(OBJ_DIR)/core/%.o: $(SRC_DIR)/core/%.c
	@echo Compiling $<...
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/input/%.o: $(SRC_DIR)/input/%.c
	@echo Compiling $<...
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/system/%.o: $(SRC_DIR)/system/%.c
	@echo Compiling $<...
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/config/%.o: $(SRC_DIR)/config/%.c
	@echo Compiling $<...
	$(CC) $(CFLAGS) -c $< -o $@

# Compile resource file
$(RESOURCE_OBJECT): $(RESOURCE_FILE) gpdesk.manifest
	@echo Compiling resource file...
	$(RC) -i $(RESOURCE_FILE) -o $@

# Clean build artifacts
clean:
	@echo Cleaning build artifacts...
	@if exist "$(BUILD_DIR)" rmdir /s /q "$(BUILD_DIR)"
	@echo Clean completed.

# Run the application
run: release
	@echo Running GPDesk...
	$(TARGET)

run-debug: debug
	@echo Running GPDesk (Debug)...
	$(TARGET_DEBUG)

# Install (copy to system directory)
install: release
	@echo Installing GPDesk...
	@if not exist "C:\Program Files\GPDesk" mkdir "C:\Program Files\GPDesk"
	copy "$(TARGET)" "C:\Program Files\GPDesk\"
	@echo Installation completed.

# Uninstall
uninstall:
	@echo Uninstalling GPDesk...
	@if exist "C:\Program Files\GPDesk" rmdir /s /q "C:\Program Files\GPDesk"
	@echo Uninstallation completed.

# Create distribution package
dist: release
	@echo Creating distribution package...
	@if not exist "dist" mkdir "dist"
	copy "$(TARGET)" "dist\"
	copy "README.md" "dist\" 2>nul || echo README.md not found
	copy "LICENSE" "dist\" 2>nul || echo LICENSE not found
	@echo Distribution package created in dist folder.

# Show help
help:
	@echo GPDesk Build System
	@echo Available targets:
	@echo   all       - Build release version (default)
	@echo   debug     - Build debug version
	@echo   release   - Build release version
	@echo   clean     - Clean build artifacts
	@echo   run       - Build and run release version
	@echo   run-debug - Build and run debug version
	@echo   install   - Install to system directory
	@echo   uninstall - Remove from system directory
	@echo   dist      - Create distribution package
	@echo   help      - Show this help

# Phony targets
.PHONY: all debug release clean run run-debug install uninstall dist help

# Dependencies
$(OBJ_DIR)/core/main.o: $(INCLUDE_DIR)/gpdesk.h $(INCLUDE_DIR)/logger.h $(INCLUDE_DIR)/input.h $(INCLUDE_DIR)/system_control.h $(INCLUDE_DIR)/config.h
$(OBJ_DIR)/core/logger.o: $(INCLUDE_DIR)/logger.h
$(OBJ_DIR)/input/gamepad.o: $(INCLUDE_DIR)/input.h $(INCLUDE_DIR)/gpdesk.h $(INCLUDE_DIR)/system_control.h $(INCLUDE_DIR)/logger.h
$(OBJ_DIR)/system/audio_control.o: $(INCLUDE_DIR)/system_control.h $(INCLUDE_DIR)/logger.h
$(OBJ_DIR)/system/power_control.o: $(INCLUDE_DIR)/system_control.h $(INCLUDE_DIR)/logger.h
$(OBJ_DIR)/system/display_control.o: $(INCLUDE_DIR)/system_control.h $(INCLUDE_DIR)/logger.h
$(OBJ_DIR)/system/app_control.o: $(INCLUDE_DIR)/system_control.h $(INCLUDE_DIR)/logger.h
$(OBJ_DIR)/config/config.o: $(INCLUDE_DIR)/config.h $(INCLUDE_DIR)/logger.h $(INCLUDE_DIR)/input.h