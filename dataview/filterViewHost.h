#ifndef _NULLSOFT_WINAMP_DATAVIEW_FILTER_VIEW_WINDOW_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_FILTER_VIEW_WINDOW_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "./filterChain.h"
#include "./ifc_viewconfig.h"
#include "./ifc_viewcontroller.h"


#define FILTERVIEWHOST_WINDOW_CLASS		L"NullsoftDataViewFilterHost"

HWND 
FilterViewHost_CreateWindow(FilterChain *filterChain,
							ifc_viewconfig *config,
							ifc_viewcontroller *controller,
							HWND parentWindow, 
							int x, 
							int y, 
							int width, 
							int height, 
							int controlId);



#endif //_NULLSOFT_WINAMP_DATAVIEW_FILTER_VIEW_WINDOW_HEADER