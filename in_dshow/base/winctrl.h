//------------------------------------------------------------------------------
// File: WinCtrl.h
//
// Desc: DirectShow base classes - defines classes for video control 
//       interfaces.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#ifndef __WINCTRL__
#define __WINCTRL__

#define ABSOL(x) (x < 0 ? -x : x)
#define NEGAT(x) (x > 0 ? -x : x)

//  Helper
BOOL WINAPI PossiblyEatMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


#endif // __WINCTRL__

