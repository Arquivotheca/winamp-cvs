#ifndef _NULLSOFT_WINAMP_DATAVIEW_PLUGIN_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_PLUGIN_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>


#define PLUGIN_VERSION_MAJOR		1
#define PLUGIN_VERSION_MINOR		0

// {AB9B9F97-0D96-47c2-AF3B-74995065FEAC}
DEFINE_GUID(PLUGIN_LANGUAGE_ID, 
0xab9b9f97, 0xd96, 0x47c2, 0xaf, 0x3b, 0x74, 0x99, 0x50, 0x65, 0xfe, 0xac);

class api_dataview;
class ifc_viewcolumnmanager;
class ifc_groupmanager;
class Theme;
class StringArticle;
class PerformanceTimerDebugOutput;
typedef struct NumberFormatEx NumberFormatEx;

HINSTANCE
Plugin_GetInstance(void);

HWND 
Plugin_GetWinampWindow(void);

ATOM
Plugin_GetWindowAtom();

void *
Plugin_GetWindowData(HWND hwnd);

BOOL
Plugin_SetWindowData(HWND hwnd, void *data);

void *
Plugin_RemoveWindowData(HWND hwnd);

HRESULT
Plugin_GetDataView(api_dataview **instance);

HRESULT
Plugin_GetColumnManager(ifc_viewcolumnmanager **instance);

HRESULT
Plugin_GetGroupManager(ifc_groupmanager **instance);

HRESULT
Plugin_GetTheme(Theme **theme);

HRESULT
Plugin_SetControlTheme(HWND hwnd, unsigned int type, unsigned int style);

HRESULT
Plugin_EnsurePathExist(const wchar_t *path);

LCID
Plugin_GetUserLocaleId();

_locale_t
Plugin_GetCLocale();

HRESULT
Plugin_GetNumberFormat(LCID localeId, NumberFormatEx *instance);

StringArticle *
Plugin_GetStringArticle();

PerformanceTimerDebugOutput *
Plugin_GetPerformanceTimerDebugOutput();


#endif //_NULLSOFT_WINAMP_DATAVIEW_PLUGIN_HEADER