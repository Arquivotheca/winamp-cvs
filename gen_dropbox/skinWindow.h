#ifndef NULLOSFT_DROPBOX_PLUGIN_SKINWINDOW_HEADER
#define NULLOSFT_DROPBOX_PLUGIN_SKINWINDOW_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "../gen_ml/ml_ipc_0313.h"

#ifdef _DEBUG
#pragma warning( push )
#pragma warning( disable : 4244 )
#endif

#include <api/wnd/api_window.h>

#ifdef _DEBUG
#pragma warning( pop )
#endif

#define OSWNDHOST_REQUEST_IDEAL_SIZE (WM_USER + 2048)

BOOL InitializeSkinSupport();

typedef struct embedWindowState embedWindowState;
typedef int (CALLBACK *FFCALLBACK)(embedWindowState* /*windowState*/, INT /*eventId*/, LPARAM /*param*/);
typedef LPVOID HMLIMGLST;

HRESULT SkinWindow(HWND hwnd, const GUID *windowGuid, UINT flagsEx, FFCALLBACK callbackFF);
HRESULT MlSkinWindow(HWND hwnd, UINT style);
HRESULT MlSkinWindowEx(HWND hwnd, INT type, UINT style);
HRESULT MlUnskinWindow(HWND hwnd);
HRESULT MlGetSkinColor(UINT uObject, UINT uPart, UINT uState, COLORREF *pColor);
HRESULT MlTrackSkinnedPopupMenuEx(HMENU hmenu, UINT fuFlags, INT x, INT y, HWND hwnd, LPTPMPARAMS lptpm, 
										HMLIMGLST hmlil, INT width, UINT skinStyle, MENUCUSTOMIZEPROC customProc, ULONG_PTR customParam, INT *resultOut);
HRESULT MlIsSkinnedPopupEnabled(void); // returns S_OK/S_FALSE on success or E_XXX if failed

#define EXTENDED_COLORS			100
#define COLOR_DIALOG			(EXTENDED_COLORS + 0)
#define COLOR_DIALOGTEXT		(EXTENDED_COLORS + 1)

COLORREF WINAPI GetSystemColor(INT index);
COLORREF WINAPI GetSkinColor(INT index);
HBRUSH WINAPI GetSkinBrush(INT index);
HBRUSH WINAPI GetSystemBrush(INT index);

HFONT GetSkinFont();
HFONT CreateSkinFont();

#define UPDATESKIN_COLOR	0x0001
#define UPDATESKIN_FONT		0x0002
void UpdateSkinCache(INT updateFlags);

BOOL TrackPopupEx(HMENU hMenu, UINT fuFlags, INT x, INT y, HWND hwnd, BOOL fForceUnskinned, LPTPMPARAMS lptpm, HMLIMGLST hmlil, INT width, UINT skinStyle, MENUCUSTOMIZEPROC customProc, ULONG_PTR customParam);
BOOL TrackPopup(HMENU hMenu, UINT fuFlags, INT x, INT y, HWND hwnd, BOOL fForceUnskinned);

#endif //NULLOSFT_DROPBOX_PLUGIN_SKINWINDOW_HEADER