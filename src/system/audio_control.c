#include "system_control.h"
#include <objbase.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <initguid.h>

// Define COM GUIDs
DEFINE_GUID(CLSID_MMDeviceEnumerator, 0xBCDE0395, 0xE52F, 0x467C, 0x8E, 0x3D, 0xC4, 0x57, 0x92, 0x91, 0x69, 0x2E);
DEFINE_GUID(IID_IMMDeviceEnumerator, 0xA95664D2, 0x9614, 0x4F35, 0xA7, 0x46, 0xDE, 0x8D, 0xB6, 0x36, 0x17, 0xE6);
DEFINE_GUID(IID_IAudioEndpointVolume, 0x5CDF2C82, 0x841E, 0x4546, 0x97, 0x22, 0x0C, 0xF7, 0x40, 0x78, 0x22, 0x9A);

// Global system control instance
SystemControl g_systemControl = {0};

bool SystemControl_Initialize(void)
{
    LOG_ENTRY_SIMPLE();
    
    if (g_systemControl.isInitialized)
    {
        LOG_WARNING("System control already initialized");
        LOG_EXIT("return=true");
        return true;
    }
    
    // Initialize COM
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr))
    {
        LOG_ERROR("Failed to initialize COM, hr=0x%08X", hr);
        LOG_EXIT("return=false");
        return false;
    }
    
    // Initialize audio system
    if (!Audio_Initialize())
    {
        LOG_ERROR("Failed to initialize audio system");
        CoUninitialize();
        LOG_EXIT("return=false");
        return false;
    }
    
    // Load required libraries
    g_systemControl.powerProfileLib = LoadLibraryA("powrprof.dll");
    g_systemControl.user32Lib = LoadLibraryA("user32.dll");
    
    g_systemControl.isInitialized = true;
    
    LOG_INFO("System control initialized successfully");
    LOG_EXIT("return=true");
    return true;
}

void SystemControl_Cleanup(void)
{
    LOG_ENTRY_SIMPLE();
    
    if (!g_systemControl.isInitialized)
    {
        LOG_EXIT_SIMPLE();
        return;
    }
    
    // Cleanup audio system
    Audio_Cleanup();
    
    // Unload libraries
    if (g_systemControl.powerProfileLib)
    {
        FreeLibrary(g_systemControl.powerProfileLib);
        g_systemControl.powerProfileLib = NULL;
    }
    
    if (g_systemControl.user32Lib)
    {
        FreeLibrary(g_systemControl.user32Lib);
        g_systemControl.user32Lib = NULL;
    }
    
    // Uninitialize COM
    CoUninitialize();
    
    g_systemControl.isInitialized = false;
    
    LOG_INFO("System control cleanup complete");
    LOG_EXIT_SIMPLE();
}

bool Audio_Initialize(void)
{
    LOG_ENTRY_SIMPLE();
    
    if (g_systemControl.audio.isInitialized)
    {
        LOG_WARNING("Audio system already initialized");
        LOG_EXIT("return=true");
        return true;
    }
    
    HRESULT hr;
    
    // Create device enumerator
    hr = CoCreateInstance(&CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL,
                         &IID_IMMDeviceEnumerator, (void**)&g_systemControl.audio.deviceEnumerator);
    if (FAILED(hr))
    {
        LOG_ERROR("Failed to create device enumerator, hr=0x%08X", hr);
        LOG_EXIT("return=false");
        return false;
    }
    
    // Get default audio endpoint
    hr = g_systemControl.audio.deviceEnumerator->lpVtbl->GetDefaultAudioEndpoint(
        g_systemControl.audio.deviceEnumerator,
        eRender,
        eConsole,
        &g_systemControl.audio.defaultDevice);
    if (FAILED(hr))
    {
        LOG_ERROR("Failed to get default audio endpoint, hr=0x%08X", hr);
        g_systemControl.audio.deviceEnumerator->lpVtbl->Release(g_systemControl.audio.deviceEnumerator);
        g_systemControl.audio.deviceEnumerator = NULL;
        LOG_EXIT("return=false");
        return false;
    }
    
    // Get endpoint volume interface
    hr = g_systemControl.audio.defaultDevice->lpVtbl->Activate(
        g_systemControl.audio.defaultDevice,
        &IID_IAudioEndpointVolume,
        CLSCTX_ALL,
        NULL,
        (void**)&g_systemControl.audio.endpointVolume);
    if (FAILED(hr))
    {
        LOG_ERROR("Failed to activate endpoint volume, hr=0x%08X", hr);
        g_systemControl.audio.defaultDevice->lpVtbl->Release(g_systemControl.audio.defaultDevice);
        g_systemControl.audio.defaultDevice = NULL;
        g_systemControl.audio.deviceEnumerator->lpVtbl->Release(g_systemControl.audio.deviceEnumerator);
        g_systemControl.audio.deviceEnumerator = NULL;
        LOG_EXIT("return=false");
        return false;
    }
    
    // Get current volume and mute state
    g_systemControl.audio.currentVolume = Audio_GetVolume();
    g_systemControl.audio.isMuted = Audio_IsMuted();
    
    g_systemControl.audio.isInitialized = true;
    
    LOG_INFO("Audio system initialized, current volume: %.2f, muted: %d", 
             g_systemControl.audio.currentVolume, g_systemControl.audio.isMuted);
    LOG_EXIT("return=true");
    return true;
}

void Audio_Cleanup(void)
{
    LOG_ENTRY_SIMPLE();
    
    if (!g_systemControl.audio.isInitialized)
    {
        LOG_EXIT_SIMPLE();
        return;
    }
    
    if (g_systemControl.audio.endpointVolume)
    {
        g_systemControl.audio.endpointVolume->lpVtbl->Release(g_systemControl.audio.endpointVolume);
        g_systemControl.audio.endpointVolume = NULL;
    }
    
    if (g_systemControl.audio.defaultDevice)
    {
        g_systemControl.audio.defaultDevice->lpVtbl->Release(g_systemControl.audio.defaultDevice);
        g_systemControl.audio.defaultDevice = NULL;
    }
    
    if (g_systemControl.audio.deviceEnumerator)
    {
        g_systemControl.audio.deviceEnumerator->lpVtbl->Release(g_systemControl.audio.deviceEnumerator);
        g_systemControl.audio.deviceEnumerator = NULL;
    }
    
    g_systemControl.audio.isInitialized = false;
    
    LOG_INFO("Audio system cleanup complete");
    LOG_EXIT_SIMPLE();
}

bool Audio_SetVolume(float volume)
{
    LOG_ENTRY("volume=%.2f", volume);
    
    if (!g_systemControl.audio.isInitialized)
    {
        LOG_ERROR("Audio system not initialized");
        LOG_EXIT("return=false");
        return false;
    }
    
    if (volume < 0.0f)
    {
        volume = 0.0f;
    }
    else if (volume > 1.0f)
    {
        volume = 1.0f;
    }
    
    HRESULT hr = g_systemControl.audio.endpointVolume->lpVtbl->SetMasterVolumeLevelScalar(
        g_systemControl.audio.endpointVolume, volume, NULL);
    
    if (FAILED(hr))
    {
        LOG_ERROR("Failed to set volume, hr=0x%08X", hr);
        LOG_EXIT("return=false");
        return false;
    }
    
    g_systemControl.audio.currentVolume = volume;
    
    LOG_DEBUG("Volume set to %.2f", volume);
    LOG_EXIT("return=true");
    return true;
}

float Audio_GetVolume(void)
{
    if (!g_systemControl.audio.isInitialized)
    {
        LOG_ERROR("Audio system not initialized");
        return 0.0f;
    }
    
    float volume = 0.0f;
    HRESULT hr = g_systemControl.audio.endpointVolume->lpVtbl->GetMasterVolumeLevelScalar(
        g_systemControl.audio.endpointVolume, &volume);
    
    if (FAILED(hr))
    {
        LOG_ERROR("Failed to get volume, hr=0x%08X", hr);
        return 0.0f;
    }
    
    return volume;
}

bool Audio_SetMute(bool mute)
{
    LOG_ENTRY("mute=%d", mute);
    
    if (!g_systemControl.audio.isInitialized)
    {
        LOG_ERROR("Audio system not initialized");
        LOG_EXIT("return=false");
        return false;
    }
    
    HRESULT hr = g_systemControl.audio.endpointVolume->lpVtbl->SetMute(
        g_systemControl.audio.endpointVolume, mute ? TRUE : FALSE, NULL);
    
    if (FAILED(hr))
    {
        LOG_ERROR("Failed to set mute, hr=0x%08X", hr);
        LOG_EXIT("return=false");
        return false;
    }
    
    g_systemControl.audio.isMuted = mute;
    
    LOG_DEBUG("Mute set to %d", mute);
    LOG_EXIT("return=true");
    return true;
}

bool Audio_IsMuted(void)
{
    if (!g_systemControl.audio.isInitialized)
    {
        LOG_ERROR("Audio system not initialized");
        return false;
    }
    
    BOOL muted = FALSE;
    HRESULT hr = g_systemControl.audio.endpointVolume->lpVtbl->GetMute(
        g_systemControl.audio.endpointVolume, &muted);
    
    if (FAILED(hr))
    {
        LOG_ERROR("Failed to get mute state, hr=0x%08X", hr);
        return false;
    }
    
    return muted == TRUE;
}

bool Audio_VolumeUp(float increment)
{
    LOG_ENTRY("increment=%.2f", increment);
    
    if (increment <= 0.0f)
    {
        LOG_ERROR("Invalid increment value: %.2f", increment);
        LOG_EXIT("return=false");
        return false;
    }
    
    float currentVolume = Audio_GetVolume();
    float newVolume = currentVolume + increment;
    
    bool result = Audio_SetVolume(newVolume);
    
    LOG_INFO("Volume up: %.2f -> %.2f", currentVolume, newVolume);
    LOG_EXIT("return=%d", result);
    return result;
}

bool Audio_VolumeDown(float increment)
{
    LOG_ENTRY("increment=%.2f", increment);
    
    if (increment <= 0.0f)
    {
        LOG_ERROR("Invalid increment value: %.2f", increment);
        LOG_EXIT("return=false");
        return false;
    }
    
    float currentVolume = Audio_GetVolume();
    float newVolume = currentVolume - increment;
    
    bool result = Audio_SetVolume(newVolume);
    
    LOG_INFO("Volume down: %.2f -> %.2f", currentVolume, newVolume);
    LOG_EXIT("return=%d", result);
    return result;
}

bool Audio_ToggleMute(void)
{
    LOG_ENTRY_SIMPLE();
    
    bool currentMuted = Audio_IsMuted();
    bool newMuted = !currentMuted;
    
    bool result = Audio_SetMute(newMuted);
    
    LOG_INFO("Mute toggled: %d -> %d", currentMuted, newMuted);
    LOG_EXIT("return=%d", result);
    return result;
}