#ifndef _NULLSOFT_WINAMP_DATAVIEW_THEME_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_THEME_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "../gen_ml/ml_ipc_0313.h"
#include "../gen_ml/colors.h"
#include "../winamp/wa_ipc.h"
#include "../winamp/wa_dlg.h"

class Theme
{

protected:
	Theme();
	~Theme();

public:
	static HRESULT CreateInstance(Theme **instance);

public:
	size_t AddRef();
	size_t Release();

	COLORREF GetColor(unsigned int colorIndex);
	COLORREF GetColorEx(UINT uObject, UINT uPart, UINT uState);
	HFONT GetTextFont();

	HRESULT SkinControl(HWND hwnd, UINT type, UINT style);

	HRESULT Rating_Draw(HDC hdc, INT maxValue, INT value, INT trackingVal, RECT *prc, UINT fStyle);
	HRESULT Rating_HitTest(POINT pt, INT maxValue, RECT *prc, UINT fStyle, long *result);
	HRESULT Rating_CalcMinRect(INT maxValue, RECT *prc);

	BOOL PopupMenu_Track(HMENU hMenu, UINT fuFlags, INT x, INT y, HWND hwnd, LPTPMPARAMS lptpm);
	HRESULT PopupMenu_IsEnabled(void);
	HANDLE PopupMenu_InitHook(HWND hwnd);
	HRESULT PopupMenu_RemoveHook(HANDLE popupHook);

protected:
	HRESULT LoadLibraryModule(void);
	HRESULT InitializeColorData(void);
	HRESULT MlTrackSkinnedPopupMenuEx(HMENU hmenu, UINT fuFlags, INT x, INT y, HWND hwnd, LPTPMPARAMS lptpm, HMLIMGLST hmlil, INT width, UINT skinStyle, MENUCUSTOMIZEPROC customProc, ULONG_PTR customParam, INT *resultOut);
	HRESULT MlInitSkinnedPopupHook(HWND hwnd, HMLIMGLST hmlil, INT width, UINT skinStyle, MENUCUSTOMIZEPROC customProc, ULONG_PTR customParam, HANDLE *hookOut);
	HFONT CreateThemeFont();


private:
	typedef BOOL (__cdecl *MLSKINWINDOWEX)(HWND /*hwnd*/, INT /*type*/, UINT /*style*/);
	typedef BOOL (__cdecl *MLGETSKINCOLOR)(UINT /*uObject*/, UINT /*uPart*/, UINT /*uState*/, COLORREF* /*pColor*/);
	typedef void (__cdecl *MLRESETSKINCOLOR)(void);
	typedef BOOL (__cdecl *MLTRACKSKINNEDPOPUPMENUEX)(HMENU /*hmenu*/, UINT /*fuFlags*/, INT /*x*/, INT /*y*/, HWND /*hwnd*/,
												LPTPMPARAMS /*lptpm*/, HMLIMGLST /*hmlil*/, INT /*width*/, UINT /*skinStyle*/,
												MENUCUSTOMIZEPROC /*customProc*/, ULONG_PTR /*customParam*/);
	typedef BOOL (__cdecl *MLISSKINNEDPOPUPENABLED)(void);
	typedef HANDLE (__cdecl *MLINITSKINNEDPOPUPHOOK)(HWND /*hwnd*/, HMLIMGLST /*hmlil*/, INT /*width*/, UINT /*skinStyle*/,
												MENUCUSTOMIZEPROC /*customProc*/, ULONG_PTR /*customParam*/);
	typedef void (__cdecl *MLREMOVESKINNEDPOPUPHOOK)(HANDLE /*hPopupHook*/);

	typedef BOOL (__cdecl *MLRATINGDRAW)(HDC /*hdc*/, INT /*maxValue*/, INT /*value*/, INT /*trackingVal*/, HMLIMGLST /*hmlil*/, INT /*index*/, RECT* /*prc*/, UINT /*fStyle*/);
	typedef LONG (__cdecl *MLRATINGHITTEST)(POINT /*pt*/, INT /*maxValue*/, HMLIMGLST /*hmlil*/, RECT* /*prc*/, UINT /*fStyle*/);
	typedef BOOL (__cdecl *MLRATINGCALCMINRECT)(INT /*maxValue*/, HMLIMGLST /*hmlil*/, RECT* /*prc*/);

protected:
	size_t ref;
	HMODULE libraryModule;
	HRESULT libraryLoadResult;
	BOOL colorInitRequired;
	HFONT textFont;

private:
	MLSKINWINDOWEX mlSkinWindowEx;
	MLGETSKINCOLOR mlGetSkinColor;
	MLRESETSKINCOLOR mlResetSkinColor;
	MLTRACKSKINNEDPOPUPMENUEX mlTrackSkinnedPopupMenuEx;
	MLISSKINNEDPOPUPENABLED mlIsSkinnedPopupEnabled;
	MLINITSKINNEDPOPUPHOOK mlInitSkinnedPopupHook;
	MLREMOVESKINNEDPOPUPHOOK mlRemoveSkinnedPopupHook;
	MLRATINGDRAW mlRatingDraw;
	MLRATINGHITTEST mlRatingHitTest;
	MLRATINGCALCMINRECT mlRatingCalcMinRect;

};


#endif //_NULLSOFT_WINAMP_DATAVIEW_THEME_HEADER