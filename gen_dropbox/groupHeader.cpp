#include "main.h"
#include "./groupHeader.h"
#include "./guiObjects.h"
#include "./fontHelper.h"
#include "./imageLoader.h"
#include "./resource.h"
#include "../nu/windowsTheme.h"

#include <windows.h>
#include <shlwapi.h>
#include <tmschema.h>
#include <commctrl.h>
#include <strsafe.h>


#define HEADER_MIN_HEIGHT 40

typedef struct __GROUPHEADER
{
	COLORREF rgbBk;
	COLORREF rgbBk2;
	COLORREF rgbText;
	HFONT	 textFont;
	HBITMAP	 backBitmap;
} GROUPHEADER;


#define GetGroupHeader(__hwnd) ((GROUPHEADER*)(LONG_PTR)(LONGX86)GetWindowLongPtr((__hwnd), 0))

static LRESULT CALLBACK GroupHeader_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


BOOL GroupHeader_RegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX  wc;
	if (GetClassInfoEx(hInstance, NWC_GROUPHEADER, &wc)) 
		return TRUE;

	ZeroMemory(&wc, sizeof(WNDCLASSEX));

	wc.cbSize		= sizeof(WNDCLASSEX);
	wc.hInstance		= hInstance;
	wc.lpszClassName	= NWC_GROUPHEADER;
	wc.lpfnWndProc	= GroupHeader_WindowProc;
	wc.style			= 0;
	wc.hIcon			= NULL;
	wc.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground	= NULL;
	wc.cbWndExtra	= sizeof(GROUPHEADER*);
	
	return ( 0 != RegisterClassEx(&wc));
}


static HFONT GroupHeader_GetDefaultFont()
{
	static HFONT defaultFont = NULL;

	if (NULL == defaultFont)
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
		lf.lfQuality = DEFAULT_QUALITY;
		lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
		
		lf.lfHeight += (lf.lfHeight < 0) ? -2 : 2;
		LPCTSTR pszFonts[] = { TEXT("Tahoma"), TEXT("MS Shell Dlg 2"), TEXT("MS Shell Dlg"), };
		FontHelper_PickFontName(NULL, &lf, pszFonts, ARRAYSIZE(pszFonts));
		
		defaultFont = CreateFontIndirect(&lf);
	}

	return defaultFont;
}

static INT GroupHeader_GetIdealHeightReal(HWND hwnd)
{
	GROUPHEADER *header = GetGroupHeader(hwnd);
	if (NULL == header) return 0;

	HDC hdc = GetDCEx(hwnd, NULL, DCX_CACHE | DCX_WINDOW | DCX_NORESETATTRS);
	if (NULL == hdc) return 0;
	
	HFONT originalFont = (HFONT)SelectObject(hdc, header->textFont);

	TEXTMETRIC tm;
	if (!GetTextMetrics(hdc, &tm))
		tm.tmHeight = 0;
	
	if (originalFont != header->textFont) SelectObject(hdc, originalFont);
	ReleaseDC(hwnd, hdc);

	INT height = tm.tmHeight * 2;
	if (height < HEADER_MIN_HEIGHT)  
		height = HEADER_MIN_HEIGHT;

	return height;
}

static BOOL GroupHeader_GetTextRect(HDC hdc, const RECT *prcClient, RECT *prcText)
{
	TEXTMETRIC tm;

	if (NULL == prcText) return FALSE;

	if (NULL == prcClient ||  NULL == hdc || !GetTextMetrics(hdc, &tm))
	{
		SetRectEmpty(prcText);
		return FALSE;
	}
	
	prcText->top = ((prcClient->bottom - prcClient->top) - tm.tmHeight) / 2;
	if (prcText->top < prcClient->top) prcText->top = prcClient->top;
	
	prcText->bottom = prcText->top + tm.tmHeight;
	if (prcText->bottom > prcClient->bottom) prcText->bottom = prcClient->bottom;

	prcText->left = prcClient->left + tm.tmAveCharWidth * 2;
	if (prcText->left > prcClient->right) prcText->left = prcClient->right;

	prcText->right = prcClient->right;
	
	return TRUE;
}



static void GroupHeader_ValidateBkBitmap(HWND hwnd, HDC hdc)
{
	GROUPHEADER *header = GetGroupHeader(hwnd);
	if (NULL == header) return;

	RECT clientRect;
    if (!GetClientRect(hwnd, &clientRect)) 
		return;

	DIBSECTION ds;
	if (NULL != header->backBitmap &&
		sizeof(DIBSECTION) == GetObject(header->backBitmap, sizeof(DIBSECTION), &ds) &&
			ds.dsBm.bmHeight == (clientRect.bottom - clientRect.top))
	{

		return;  // image is uptodate
	}

	if (NULL != header->backBitmap)
	{
		DeleteObject(header->backBitmap);
		header->backBitmap = NULL;
	}

	BYTE *patternPixels;
	BITMAPINFOHEADER patternHeader;
	HBITMAP patternBitmap = ImageLoader_LoadPngEx(plugin.hDllInstance, MAKEINTRESOURCE(IDR_GROUPHEADER_IMAGE), 
								&patternHeader, (void**)&patternPixels);
	if (NULL == patternPixels)
		return;

	ColorizeImageEx(patternPixels, patternHeader.biWidth, abs(patternHeader.biHeight), 
					patternHeader.biBitCount, header->rgbBk2, header->rgbBk, FALSE);
	

	BYTE *bitmapPixels;
	BITMAPINFOHEADER bitmapHeader;

	INT bitmapCX, bitmapCY;
	bitmapCX = (patternHeader.biWidth > (clientRect.right - clientRect.left)) ? 
		(clientRect.right - clientRect.left) : patternHeader.biWidth;
	bitmapCY = clientRect.bottom - clientRect.top;
	
	ZeroMemory(&bitmapHeader, sizeof(BITMAPINFOHEADER));
	bitmapHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitmapHeader.biCompression = BI_RGB;
	bitmapHeader.biBitCount = 24;
	bitmapHeader.biPlanes = 1;
	bitmapHeader.biWidth = bitmapCX;
	bitmapHeader.biHeight = -bitmapCY;

	header->backBitmap = CreateDIBSection(NULL, (LPBITMAPINFO)&bitmapHeader, DIB_RGB_COLORS, (void**)&bitmapPixels, NULL, 0);
	if (NULL != header->backBitmap)
	{
		HDC hdcBack = CreateCompatibleDC(hdc);
		HBITMAP originalBitmap = (HBITMAP)SelectObject(hdcBack, header->backBitmap);

		INT stretchCY = abs(patternHeader.biHeight);
		if (stretchCY > bitmapCY)
			stretchCY = bitmapCY;

		StretchDIBits(hdcBack, 0, 0, bitmapCX, stretchCY, 
					0, 0, bitmapCX, stretchCY,
					patternPixels, (BITMAPINFO*)&patternHeader, DIB_RGB_COLORS, SRCCOPY);
	
		if (stretchCY < bitmapCY)
		{
			INT stretchMode = SetStretchBltMode(hdcBack, COLORONCOLOR);

			StretchBlt(hdcBack, 0, stretchCY, bitmapCX, bitmapCY -stretchCY, 
						hdcBack, 0, stretchCY - 1, bitmapCX, 1, SRCCOPY);

			if (COLORONCOLOR != stretchMode)
					SetStretchBltMode(hdcBack, stretchMode);
		}
	
		StretchBlt(hdcBack, 1, bitmapCY -1, bitmapCX, 1, hdcBack, 0, bitmapCY - 1, 1, 1, SRCCOPY);
		
		SelectObject(hdcBack, originalBitmap);
		DeleteDC(hdcBack);
	}

	DeleteObject(patternBitmap);
}

static BOOL GroupHeader_DrawBackground(HWND hwnd, HDC hdc, HBITMAP bitmap, const RECT *prcPaint)
{
	DIBSECTION ds;
	BOOL filledOk = TRUE;
	RECT clientRect;

	if (NULL == bitmap ||
		!GetClientRect(hwnd, &clientRect) ||
		 sizeof(DIBSECTION) != GetObject(bitmap, sizeof(DIBSECTION), &ds))
	{
		return FALSE;
	}
		
	INT bitmapCX = ds.dsBm.bmWidth;
	INT bitmapCY = abs(ds.dsBm.bmHeight);
	
	INT paintRight = prcPaint->right;
	if (paintRight > bitmapCX) paintRight = bitmapCX;
	
	INT stretchMode = SetStretchBltMode(hdc, COLORONCOLOR);
	if (paintRight > prcPaint->left)
	{
		filledOk = StretchDIBits(hdc, prcPaint->left, prcPaint->bottom -1, 
				paintRight - prcPaint->left, prcPaint->top - prcPaint->bottom, 
				prcPaint->left, prcPaint->top, paintRight - prcPaint->left, prcPaint->bottom - prcPaint->top,
				ds.dsBm.bmBits, (BITMAPINFO*)&ds.dsBmih, DIB_RGB_COLORS, SRCCOPY);
	}

	if (paintRight < prcPaint->right)
	{
		StretchDIBits(hdc, paintRight, prcPaint->bottom -1, 
				prcPaint->right - paintRight, prcPaint->top - prcPaint->bottom, 
				bitmapCX - 1, prcPaint->top, 1, prcPaint->bottom - prcPaint->top,
				ds.dsBm.bmBits, (BITMAPINFO*)&ds.dsBmih, DIB_RGB_COLORS, SRCCOPY);
	}

	if (COLORONCOLOR != stretchMode)
		SetStretchBltMode(hdc, stretchMode);

	return filledOk;
}

static void GroupHeader_Paint(HWND hwnd, HDC hdc, const RECT *prcPaint, BOOL fErase)
{
	GROUPHEADER *header = GetGroupHeader(hwnd);
	if (NULL == header) return;

	RECT clientRect, partRect;
	if (!GetClientRect(hwnd, &clientRect)) return;

	COLORREF rgbBk, rgbFg;
	rgbBk = header->rgbBk;
	rgbFg = header->rgbText;

	COLORREF originalBk = SetBkColor(hdc, rgbBk);
	COLORREF originalFg = SetTextColor(hdc, rgbFg);

	HFONT originalFont = (HFONT)SelectObject(hdc, header->textFont);

	TCHAR szText[256];
	INT cchText = GetWindowText(hwnd, szText, ARRAYSIZE(szText));

	if (fErase)
	{	
		GroupHeader_ValidateBkBitmap(hwnd, hdc);
		BOOL filledOk = GroupHeader_DrawBackground(hwnd, hdc, header->backBitmap, prcPaint);
		if (!filledOk)
		{
			COLORREF originalColor = SetBkColor(hdc, RGB(255, 0, 0));
			ExtTextOut(hdc, 0, 0, ETO_OPAQUE, prcPaint, NULL, 0, NULL);
			SetBkColor(hdc, originalColor);
		}
	}
	
	if (0 != cchText)
	{	
		GroupHeader_GetTextRect(hdc, &clientRect, &partRect);
		INT originalMode = SetBkMode(hdc, TRANSPARENT);
		UINT originalAlign = SetTextAlign(hdc, TA_LEFT | TA_TOP);

		/* shadow */
		/*{
			OffsetRect(&partRect, 1, 1);
			SetTextColor(hdc, header->rgbBk);
			ExtTextOut(hdc, partRect.left, partRect.top, ETO_CLIPPED, &partRect, szText, cchText, NULL);
			OffsetRect(&partRect, -1, -1);
			SetTextColor(hdc, header->rgbText);
		}*/

		ExtTextOut(hdc, partRect.left, partRect.top, ETO_CLIPPED, &partRect, szText, cchText, NULL);
		
		if (TRANSPARENT != originalMode) SetBkMode(hdc, originalMode);
		if ((TA_LEFT | TA_TOP) != originalAlign) SetTextAlign(hdc, originalAlign);
	}

	SelectObject(hdc, originalFont);
	if (originalBk != rgbBk) SetBkColor(hdc, originalBk);
	if (originalFg != rgbFg) SetTextColor(hdc, originalFg);

}


static LRESULT GroupHeader_OnCreateWindow(HWND hwnd, CREATESTRUCT *pcs)
{	
	GROUPHEADER *header;

	header = (GROUPHEADER*)malloc(sizeof(GROUPHEADER));
	if (NULL == header)
	{
		DestroyWindow(hwnd);
		return -1;
	}

	ZeroMemory(header, sizeof(GROUPHEADER));
	
	SetLastError(ERROR_SUCCESS);
	if (!SetWindowLongPtr(hwnd, 0, (LONGX86)(LONG_PTR)header) && ERROR_SUCCESS != GetLastError())
	{
		free(header);
		DestroyWindow(hwnd);
		return -1;
	}

	header->rgbBk = GetSysColor(COLOR_WINDOW);
	header->rgbBk2 = GetSysColor(COLOR_HIGHLIGHT);

	WORD h,l,s;
	ColorRGBToHLS(GetSysColor(COLOR_HIGHLIGHT), &h, &l, &s);
	header->rgbText = ColorHLSToRGB(h, 60, s);

	UXTHEME hTheme = UxOpenThemeData(hwnd, L"Edit");
	if (NULL != hTheme)
	{
		COLORREF themeColor;
		if (SUCCEEDED(UxGetThemeColor(hTheme, EP_EDITTEXT, ETS_NORMAL, TMT_BORDERCOLOR, &themeColor)))
			header->rgbBk2 = themeColor;
		UxCloseThemeData(hTheme);
	}

	header->textFont = GroupHeader_GetDefaultFont();
	
	return FALSE;
}

static void GroupHeader_OnDestroy(HWND hwnd)
{
	GROUPHEADER *header = GetGroupHeader(hwnd);
	SetWindowLongPtr(hwnd, 0, 0L);
	if (!header) return;
	
	if (NULL != header->backBitmap)
		DeleteObject(header->backBitmap);

	free(header);
}

static void GroupHeader_OnPaint(HWND hwnd)
{
	PAINTSTRUCT ps;
	if (BeginPaint(hwnd, &ps))
	{
		if (ps.rcPaint.left != ps.rcPaint.right)
			GroupHeader_Paint(hwnd, ps.hdc, &ps.rcPaint, ps.fErase);
		EndPaint(hwnd, &ps);
	}
}

static void GroupHeader_OnPrintClient(HWND hwnd, HDC hdc, UINT options)
{	
	RECT clientRect;
	if (GetClientRect(hwnd, &clientRect))
	GroupHeader_Paint(hwnd, hdc, &clientRect, 0 != (PRF_ERASEBKGND & options));
}

static void GroupHeader_OnEnable(HWND hwnd, BOOL fEnabled)
{
	InvalidateRect(hwnd, NULL, TRUE);
}

static void GroupHeader_OnSetFont(HWND hwnd, HFONT newFont, BOOL fRedraw)
{
	GROUPHEADER *header = GetGroupHeader(hwnd);
	if (NULL != header) header->textFont = newFont;

	if (FALSE != fRedraw)
		InvalidateRect(hwnd, NULL, TRUE);
}

static LRESULT GroupHeader_OnGetFont(HWND hwnd)
{
	GROUPHEADER *header = GetGroupHeader(hwnd);
	return (LRESULT)((NULL != header) ? header->textFont : NULL);
}

static LRESULT GroupHeader_OnGetIdealHeight(HWND hwnd)
{
	return GroupHeader_GetIdealHeightReal(hwnd);
}

static LRESULT GroupHeader_OnAdjustRect(HWND hwnd, RECT *prcAdjust)
{
	if (NULL == prcAdjust) return FALSE;

	INT idealHeight = GroupHeader_GetIdealHeightReal(hwnd);
	
	if ((prcAdjust->top + idealHeight) < prcAdjust->bottom)
		prcAdjust->bottom = prcAdjust->top + idealHeight;
	
	return TRUE;
}

static LRESULT CALLBACK GroupHeader_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_CREATE:				return GroupHeader_OnCreateWindow(hwnd, (CREATESTRUCT*)lParam);
		case WM_DESTROY:				GroupHeader_OnDestroy(hwnd); return 0;
		case WM_PAINT:				GroupHeader_OnPaint(hwnd); return 0;
		case WM_PRINTCLIENT:			GroupHeader_OnPrintClient(hwnd, (HDC)wParam, (UINT)lParam); return 0;
		case WM_ERASEBKGND:			return 0;
		case WM_SETFONT:				GroupHeader_OnSetFont(hwnd, (HFONT)wParam, (BOOL)LOWORD(lParam)); return 0;
		case WM_GETFONT:				return GroupHeader_OnGetFont(hwnd);
		
		case WM_ENABLE:				GroupHeader_OnEnable(hwnd, (BOOL)wParam); return 0;
		
		case GHM_GETIDEALHEIGHT:		return GroupHeader_OnGetIdealHeight(hwnd);
		case GHM_ADJUSTRECT:			return GroupHeader_OnAdjustRect(hwnd, (RECT*)lParam);
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}