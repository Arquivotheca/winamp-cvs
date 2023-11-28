#ifndef NULLSOFT_ADDONS_PLUGIN_MAIN_HEADER
#define NULLSOFT_ADDONS_PLUGIN_MAIN_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "../gen_ml/ml.h"
#include "./common.h"

#define PLUGIN_VERSION_MAJOR		1
#define PLUGIN_VERSION_MINOR		12

HINSTANCE Plugin_GetInstance(void);
HWND Plugin_GetWinamp(void);
HWND Plugin_GetLibrary(void);

#endif //NULLSOFT_ADDONS_PLUGIN_MAIN_HEADER