#ifndef NULLSOFT_ORB_PLUGIN_NAVIGATION_HEADER
#define NULLSOFT_ORB_PLUGIN_NAVIGATION_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

typedef LPVOID HNAVITEM;
class OmService;

BOOL Navigation_Initialize(void);
BOOL Navigation_ProcessMessage(INT msg, INT_PTR param1, INT_PTR param2, INT_PTR param3, INT_PTR *result);
HNAVITEM Navigation_GetActive(OmService **serviceOut);
HNAVITEM Navigation_FindService(UINT serviceId, HNAVITEM hStart, OmService **serviceOut);
BOOL Navigation_RemoveService(UINT serviceId);
#endif //NULLSOFT_ORB_PLUGIN_NAVIGATION_HEADER