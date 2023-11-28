#ifndef _NULLSOFT_WINAMP_DATAVIEW_DATA_VIEW_WINDOW_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_DATA_VIEW_WINDOW_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "./ifc_dataprovider.h"
#include "./ifc_viewconfig.h"
#include "./ifc_viewcontroller.h"


#define DATAVIEWHOST_WINDOW_CLASS		L"NullsoftDataViewHost"

HWND 
DataViewHost_CreateWindow(const char *name,
						  ifc_dataprovider *provider,
						  ifc_viewconfig *config,
						  ifc_viewcontroller *controller,
						  HWND parentWindow, 
						  int x, 
						  int y, 
						  int width, 
						  int height, 
						  int controlId);

#define DATAVIEWHOST_WM_FIRST		(WM_USER + 10)

#define DATAVIEWHOST_WM_GETPERFORMANCETIMER			(DATAVIEWHOST_WM_FIRST + 1)
#define DATAVIEWHOST_GET_PERFORMANCE_TIMER(/*HWND*/ _hwnd, /*ifc_performancetimer** */_timer)\
		((HWND)SendMessageW((_hwnd), DATAVIEWHOST_WM_GETPERFORMANCETIMER, 0, (LPARAM)(_timer)))



#endif //_NULLSOFT_WINAMP_DATAVIEW_DATA_VIEW_WINDOW_HEADER