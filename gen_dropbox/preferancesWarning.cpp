#include "./main.h"
#include "./preferences.h"
#include "./plugin.h"
#include "./resource.h"

#include "./imageLoader.h"
#include "./fontHelper.h"
#include "./wasabiAPI.h"

typedef struct __PREFWARNING
{
	WNDPROC originalProc;
	BOOL	fUnicode;
	BOOL	fEnabled;
	INT		ref;
	HBITMAP bitmap;
	HFONT	font;
} PREFWARNING;

static ATOM PREFWARNING_PROP = 0;
static LRESULT CALLBACK PrererencesWarning_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static void CALLBACK PreferencesWarning_Uninitialize()
{
	if (0 != PREFWARNING_PROP)
	{
		GlobalDeleteAtom(PREFWARNING_PROP);
		PREFWARNING_PROP = 0;
	}
}

static BOOL PreferencesWarning_GetRect(HWND hwnd, RECT *prcOut)
{
	RECT clientRect;
	if (!GetClientRect(hwnd, &clientRect))
		return FALSE;
	
	HWND hBox = GetDlgItem(hwnd, 1186 /*IDC_RECT*/);
	if (NULL == hBox || !GetWindowRect(hBox, prcOut))
		return FALSE;

	MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)prcOut, 2);

	prcOut->top = prcOut->bottom;
	prcOut->bottom = clientRect.bottom;

	return TRUE;
}

static PREFWARNING *PreferencesWarning_GetData(HWND hwnd, BOOL fCreate)
{
	if (!IsWindow(hwnd)) 
		return  NULL;

	if (0 == PREFWARNING_PROP)
	{
		 PREFWARNING_PROP = GlobalAddAtom(TEXT("waDropboxPreferencesWarning"));
		 if (0 == PREFWARNING_PROP) return  NULL;
		 Plugin_RegisterUnloadCallback(PreferencesWarning_Uninitialize);
	}
	
	PREFWARNING *warning = (PREFWARNING*)GetProp(hwnd, MAKEINTATOM(PREFWARNING_PROP));
	if (NULL == warning && FALSE != fCreate)
	{
		warning = (PREFWARNING*)malloc(sizeof(PREFWARNING));
		if (NULL == warning) return  NULL;
		
		ZeroMemory(warning, sizeof(PREFWARNING));
		
		warning->originalProc = (WNDPROC)(LONG_PTR)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)PrererencesWarning_WindowProc);
				
		if (NULL == warning->originalProc || !SetProp(hwnd, MAKEINTATOM(PREFWARNING_PROP), warning))
		{
			if (NULL != warning->originalProc)
				SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)warning->originalProc);
			return NULL;
		}
		warning->fUnicode = IsWindowUnicode(hwnd);
	}
	
	return warning;
}
static void PreferencesWarning_Invalidate(HWND hwnd)
{
	RECT invalidateRect;
	if (PreferencesWarning_GetRect(hwnd, &invalidateRect))
		RedrawWindow(hwnd, &invalidateRect, NULL,  RDW_INVALIDATE | RDW_ERASE | RDW_ERASENOW | RDW_NOCHILDREN);
}

INT Preferences_ShowWarning(HWND hDialog, BOOL fShow)
{
	PREFWARNING *warning = PreferencesWarning_GetData(hDialog, TRUE);
	if (NULL == warning) return -1;

	if (FALSE == fShow)
	{
		if (warning->ref > 0)
		{
			warning->ref--;
			if (0 == warning->ref)
				PreferencesWarning_Invalidate(hDialog);
		}
	}
	else
	{
		warning->ref++;
		if (1 == warning->ref)
			PreferencesWarning_Invalidate(hDialog);
	}

	return TRUE;
}

BOOL Preferences_EnableWarning(HWND hDialog, BOOL fEnable)
{
	PREFWARNING *warning = PreferencesWarning_GetData(hDialog, TRUE);
	if (NULL == warning) return FALSE;
	if (warning->fEnabled != (FALSE != fEnable))
	{
		warning->fEnabled = (FALSE != fEnable);
		PreferencesWarning_Invalidate(hDialog);
	}
	return TRUE;
}


static HBITMAP PreferencesWarning_LoadBitmap()
{
	INT bitmapCX, bitmapCY;
	HBITMAP bitmap = ImageLoader_LoadPng(plugin.hDllInstance, 
						MAKEINTRESOURCE(IDR_WARNING_SMALL_IMAGE), 
						&bitmapCX, &bitmapCY);

	if (NULL != bitmap)
	{
		RECT bitmapRect;
		SetRect(&bitmapRect, 0, 0, bitmapCX, bitmapCY);
		BlendOnColor(bitmap, &bitmapRect, FALSE, GetSysColor(COLOR_3DFACE));
	}

	return bitmap;
}

static HFONT PreferencesWarning_LoadFont()
{
	LOGFONT lf;
	ZeroMemory(&lf, sizeof(LOGFONT));
	lf.lfHeight = FontHelper_GetSysFontHeight();
	lf.lfWeight = FW_DONTCARE;
	lf.lfItalic = FALSE;
	lf.lfUnderline = FALSE;
	lf.lfStrikeOut = FALSE;
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf.lfQuality = ANTIALIASED_QUALITY;
	lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	
	lf.lfHeight += (lf.lfHeight < 0) ? 1 : -1;
	LPCTSTR pszFonts[] = { TEXT("Arial"), TEXT("MS Shell Dlg 2"), TEXT("MS Shell Dlg"), };
	FontHelper_PickFontName(NULL, &lf, pszFonts, ARRAYSIZE(pszFonts));

	return CreateFontIndirect(&lf);
}

static LRESULT PreferencesWarning_CallOrigWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PREFWARNING *warning = (PREFWARNING*)GetProp(hwnd, MAKEINTATOM(PREFWARNING_PROP));

	if (NULL == warning || NULL == warning->originalProc)
	{
		return (IsWindowUnicode(hwnd)) ? 
				DefWindowProcW(hwnd, uMsg, wParam, lParam) : 
				DefWindowProcA(hwnd, uMsg, wParam, lParam);
	}

	return (FALSE != warning->fUnicode) ? 
			CallWindowProcW(warning->originalProc, hwnd, uMsg, wParam, lParam) : 
			CallWindowProcA(warning->originalProc, hwnd, uMsg, wParam, lParam);
}

static void PreferencesWarning_Draw(HWND hwnd, HDC hdc)
{
	PREFWARNING *warning = (PREFWARNING*)GetProp(hwnd, MAKEINTATOM(PREFWARNING_PROP));
	if (NULL == warning || 0 == warning->ref || FALSE == warning->fEnabled)
		return;

	RECT updateRect, paintRect;
	
	if (!PreferencesWarning_GetRect(hwnd, &paintRect) ||
		paintRect.top >= paintRect.bottom || paintRect.left >= paintRect.right)
	{
		return;
	}
	
	if (!GetUpdateRect(hwnd, &updateRect, FALSE) || !IntersectRect(&updateRect, &updateRect, &paintRect))
		return;

	if (NULL == warning->bitmap)
	{
		warning->bitmap = PreferencesWarning_LoadBitmap();
		if (NULL == warning->bitmap) return;
	}

	if (NULL == warning->font)
	{
		warning->font = PreferencesWarning_LoadFont();
		if (NULL == warning->font) return;
	}
	
	DIBSECTION ds;
	INT textOffset = 0;
	
	if (NULL != warning->bitmap && 
		sizeof(DIBSECTION) == GetObject(warning->bitmap, sizeof(DIBSECTION), &ds))
	{
		BOOL success = StretchDIBits(hdc, paintRect.left + 1, paintRect.bottom - 3, 
						ds.dsBm.bmWidth, -abs(ds.dsBm.bmHeight), 
						0, 0, ds.dsBm.bmWidth, abs(ds.dsBm.bmHeight),
						ds.dsBm.bmBits, (BITMAPINFO*)&ds.dsBmih, DIB_RGB_COLORS, SRCCOPY);

		if (success)
			textOffset += (ds.dsBm.bmWidth + 4);
	}
	
	TCHAR szMessage[256];
	WASABI_API_LNGSTRINGW_BUF(IDS_RESTART_WINDOW, szMessage, ARRAYSIZE(szMessage));
	INT cchMessage = lstrlen(szMessage);

	if (0 != cchMessage)
	{
		COLORREF rgbBk, rgbFg, originalFg, originalBk;

		rgbFg = GetSysColor(COLOR_WINDOWTEXT);
		rgbBk = GetSysColor(COLOR_3DFACE);
		originalFg = SetTextColor(hdc, rgbFg);
		originalBk = SetBkColor(hdc, rgbBk);
		UINT originalAlign = SetTextAlign(hdc, TA_LEFT | TA_BOTTOM);
		
        HFONT originalFont = (HFONT)SelectObject(hdc, warning->font);

		ExtTextOut(hdc, paintRect.left + textOffset, paintRect.bottom, ETO_CLIPPED, &paintRect, szMessage, cchMessage, NULL);
		
		SelectObject(hdc, originalFont);
		if ((TA_LEFT | TA_BOTTOM) != originalAlign) SetTextAlign(hdc, originalAlign);
		if (rgbFg != originalFg) SetTextColor(hdc, originalFg);
		if (rgbBk != originalBk) SetBkColor(hdc, originalBk);
	}
}

static void PrererencesWarning_OnDestroy(HWND hwnd)
{
	PREFWARNING *warning = (PREFWARNING*)GetProp(hwnd, MAKEINTATOM(PREFWARNING_PROP));
	RemoveProp(hwnd, MAKEINTATOM(PREFWARNING_PROP));
	if (NULL == warning) return;
	
	if (NULL != warning->originalProc)
	{
		SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)warning->originalProc);
		if (FALSE != warning->fUnicode)
			CallWindowProcW(warning->originalProc, hwnd, WM_DESTROY, 0, 0L);
		else
			CallWindowProcA(warning->originalProc, hwnd, WM_DESTROY, 0, 0L);
	}
	
	if (NULL != warning->bitmap)
		DeleteObject(warning->bitmap);

	if (NULL != warning->font)
		DeleteObject(warning->font);
	
	free(warning);
}

static LRESULT PrererencesWarning_OnEraseBackground(HWND hwnd, HDC hdc)
{
	LRESULT result = PreferencesWarning_CallOrigWindowProc(hwnd, WM_ERASEBKGND, (WPARAM)hdc, 0L);
	if (0 != result)
	{
		PreferencesWarning_Draw(hwnd, hdc);	
	}
	return result;
}

static LRESULT CALLBACK PrererencesWarning_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_DESTROY:
			PrererencesWarning_OnDestroy(hwnd);
			return 0;
		case WM_ERASEBKGND:
			return PrererencesWarning_OnEraseBackground(hwnd, (HDC)wParam);
	}
	
	return PreferencesWarning_CallOrigWindowProc(hwnd, uMsg, wParam, lParam);
}