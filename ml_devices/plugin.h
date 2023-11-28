#ifndef _NULLSOFT_WINAMP_ML_DEVICES_PLUGIN_HEADER
#define _NULLSOFT_WINAMP_ML_DEVICES_PLUGIN_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "../gen_ml/ml.h"
#include "./imageCache.h"
#include "./deviceManagerHandler.h"
#include "./deviceHandler.h"

#define PLUGIN_VERSION_MAJOR		1
#define PLUGIN_VERSION_MINOR		32

// {CA4D071B-4E9B-44fd-862A-783FC763B63D}
DEFINE_GUID(PLUGIN_LANGUAGE_ID, 
0xca4d071b, 0x4e9b, 0x44fd, 0x86, 0x2a, 0x78, 0x3f, 0xc7, 0x63, 0xb6, 0x3d);

typedef void (CALLBACK *PluginUnloadCallback)(void);

HINSTANCE 
Plugin_GetInstance(void);

HWND 
Plugin_GetWinampWindow(void);

HWND 
Plugin_GetLibraryWindow(void);

BOOL 
Plugin_RegisterUnloadCallback(PluginUnloadCallback callback);

DeviceImageCache *
Plugin_GetImageCache();

HWND
Plugin_GetEventRelayWindow();

const wchar_t *
Plugin_GetDefaultDeviceImage(unsigned int width, 
							 unsigned int height);

HRESULT 
Plugin_EnsurePathExist(const wchar_t *path);

BOOL
Plugin_GetResourceString(const wchar_t *resourceName,
						 const wchar_t *resourceType, 
						 wchar_t *buffer, 
						 size_t bufferMax);

HMENU
Plugin_LoadMenu();

BOOL
Plugin_ShowHelp();

BOOL
Plugin_BeginDiscovery();

BOOL 
Plugin_OpenUrl(HWND ownerWindow, 
			   const wchar_t *url, 
			   BOOL forceExternal);

#endif //_NULLSOFT_WINAMP_ML_DEVICES_PLUGIN_HEADER

