#ifndef _NULLSOFT_WINAMP_DATAVIEW_TOOLBAR_HOST_WINDOW_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_TOOLBAR_HOST_WINDOW_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "./toolbar.h"


#define TOOLBARHOST_WINDOW_CLASS		L"NullsoftToolbarHost"

HWND 
ToolbarHost_CreateWindow(Toolbar *toolbar,
						 HWND parentWindow, 
						 int x, 
						 int y, 
						 int width, 
						 int height, 
						 int controlId);


// messages
#define TBM_FIRST			(WM_USER + 10)

#define TBM_GETIDEALHEIGHT	(TBM_FIRST + 1)
#define Toolbar_GetIdealHeight(/*HNWD*/_hwnd)\
	((INT)SendMessage((_hwnd), TBM_GETIDEALHEIGHT, 0, 0L))




#endif //_NULLSOFT_WINAMP_DATAVIEW_TOOLBAR_HOST_WINDOW_HEADER