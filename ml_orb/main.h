#ifndef NULLSOFT_ORB_PLUGIN_MAIN_HEADER
#define NULLSOFT_ORB_PLUGIN_MAIN_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "../gen_ml/ml.h"
#include "./common.h"

#define PLUGIN_VERSION_MAJOR		1
#define PLUGIN_VERSION_MINOR		31

#define SERVICE_MAIN			110
#define SERVICE_AUDIO		111
#define SERVICE_VIDEO		112

HINSTANCE Plugin_GetInstance(void);
HWND Plugin_GetWinamp(void);
HWND Plugin_GetLibrary(void);

HRESULT Plugin_GetSessionId(LPWSTR pszBuffer, INT cchBufferMax);
HRESULT Plugin_GetClientId(LPWSTR pszBuffer, INT cchBufferMax);

#endif //NULLSOFT_ORB_PLUGIN_MAIN_HEADER