#ifndef NULLOSFT_DROPBOX_PLUGIN_GROUPHEADER_HEADER
#define NULLOSFT_DROPBOX_PLUGIN_GROUPHEADER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

#define NWC_GROUPHEADER		TEXT("NullsoftGroupHeader")

// styles
#define GHS_DEFAULTCOLORS	0x0008

#define GroupHeader_CreateWindow(__windowStyleEx, __linkText, __windowStyle, __x, __y, __cx, __cy, __parentWindow, __controlId, __hInstance)\
	CreateWindowEx((__windowStyleEx), NWC_GROUPHEADER, (__linkText), (__windowStyle),\
		(__x), (__y), (__cx), (__cy), (__parentWindow), (HMENU)(INT_PTR)(__controlId), (__hInstance), 0)
						

#define GHM_FIRST			(WM_USER + 1)

#define GHM_GETIDEALHEIGHT	(GHM_FIRST + 0)
#define GroupHeader_GetIdealHeight(/*HNWD*/__hwnd)\
	((INT)SendMessage((__hwnd), GHM_GETIDEALHEIGHT, 0, 0L))

#define GHM_ADJUSTRECT		(GHM_FIRST + 1)
#define GroupHeader_AdjustRect(/*HNWD*/ __hwnd, /*LPRECT*/ __rectToAdjust)\
	((BOOL)SendMessage((__hwnd), GHM_ADJUSTRECT, 0, (LPARAM)(__rectToAdjust)))

BOOL GroupHeader_RegisterClass(HINSTANCE hInstance);

#endif // NULLOSFT_DROPBOX_PLUGIN_GROUPHEADER_HEADER