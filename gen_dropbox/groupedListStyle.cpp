#include "./groupedList.h"
#include "./groupedListView.h"

#include "./imageLoader.h"
#include "./guiObjects.h"

#include <tmschema.h>
#include <windows.h>
#include <shlwapi.h>
#include <strsafe.h>


GroupedListViewStyle::GroupedListViewStyle() :
 	pngInstance(NULL), pngResource(NULL), bitmap(NULL), buttonTheme(NULL)
{
}

GroupedListViewStyle::~GroupedListViewStyle()
{
	CloseThemes();
	CloseImages();
	CloseFonts();
	
}
void GroupedListViewStyle::PopulateColors(HWND hwndHost)
{
	COLORREF rgbWnd = GetSysColor(COLOR_WINDOW);
	COLORREF rgbTxt = GetSysColor(COLOR_WINDOWTEXT);
	COLORREF rgbHil = GetSysColor(COLOR_HIGHLIGHT);
	COLORREF rgbGry = GetSysColor(COLOR_GRAYTEXT);
	COLORREF rgbFace = GetSysColor(COLOR_3DFACE);
	COLORREF rgbHot = GetSysColor(COLOR_HIGHLIGHTTEXT);

	
	NMGLCOLOR nmcolor;
	nmcolor.hdr.code = GLVN_COLORCHANGING;
	nmcolor.hdr.hwndFrom = hwndHost;
	nmcolor.hdr.idFrom = GetDlgCtrlID(hwndHost);
	HWND hParent = GetParent(hwndHost);

#define ASSIGNCOLOR_CB(__colorId, __colorRgb)\
	{nmcolor.colorId = (__colorId);\
		nmcolor.rgb= (__colorRgb);\
		if (NULL != hParent)\
			SendMessage(hParent, WM_NOTIFY, (WPARAM)nmcolor.hdr.idFrom, (LPARAM)&nmcolor);\
			szColors[__colorId] = nmcolor.rgb;}

	ASSIGNCOLOR_CB(uiColorWindow, rgbWnd);
	ASSIGNCOLOR_CB(uiColorItem, rgbWnd);
	ASSIGNCOLOR_CB(uiColorItemText, rgbTxt);
	ASSIGNCOLOR_CB(uiColorItemHighlighted, BlendColorsF(rgbHil, rgbWnd, 0.50f));
	ASSIGNCOLOR_CB(uiColorItemTextHighlighted, rgbTxt);
	ASSIGNCOLOR_CB(uiColorItemDisabled, rgbWnd);
	ASSIGNCOLOR_CB(uiColorItemTextDisabled, rgbGry);
	ASSIGNCOLOR_CB(uiColorItemPressed, rgbHil);
	ASSIGNCOLOR_CB(uiColorItemTextPressed, rgbTxt);
	ASSIGNCOLOR_CB(uiColorGroup, BlendColorsF(rgbFace, rgbWnd, 0.80f));
	ASSIGNCOLOR_CB(uiColorGroupRight, BlendColorsF(rgbFace, rgbWnd, 0.50f));
	ASSIGNCOLOR_CB(uiColorGroupLine, rgbFace);

	ASSIGNCOLOR_CB(uiColorGroupIcon, rgbTxt);
	ASSIGNCOLOR_CB(uiColorGroupText, rgbTxt);//ColorAdjustLuma(GetDarkerColor(rgbHot, rgbHil), -200, TRUE));
	ASSIGNCOLOR_CB(uiColorGroupTextShadow, szColors[uiColorGroup]);

}

void GroupedListViewStyle::PopulateMetrics(HDC hdc)
{
	szMetrics[uiMetricLevelOffset] = 64;
	szMetrics[uiMetricItemInterval] = 2;
	szMetrics[uiMetricGroupInterval] = 8;

	szMetrics[uiMetricRadiobuttonCX] = 12;
	szMetrics[uiMetricRadiobuttonCY] = 12;
	szMetrics[uiMetricItemFontCY] = 14;
	szMetrics[uiMetricTitleFontCY] = 14;
	szMetrics[uiMetricGroupFontCY] = 14;
	
	szMetrics[uiMetricIconCX] = 0;
	szMetrics[uiMetricIconCY] = 0;	

	PopulateThemeMetrics(hdc);
	PopulateBitmapMetrics();
	PopulateFontMetrics(hdc);
	
	
}

void GroupedListViewStyle::PopulateBitmapMetrics()
{
	BITMAP bm;
	if (NULL == bitmap || sizeof(BITMAP) != GetObject(bitmap, sizeof(BITMAP), &bm))
		ZeroMemory(&bm, sizeof(BITMAP));
	
	szMetrics[uiMetricIconCX] = abs(bm.bmHeight);
	szMetrics[uiMetricIconCY] = abs(bm.bmHeight);
}

void GroupedListViewStyle::PopulateThemeMetrics(HDC hdc)
{
	szMetrics[uiMetricRadiobuttonCX] = 12;
	szMetrics[uiMetricRadiobuttonCY] = 12;

	if (NULL != buttonTheme)
	{
		HRESULT hr;
		SIZE partSize;
		hr = UxGetThemePartSize((UXTHEME)buttonTheme, hdc, BP_RADIOBUTTON, 
								RBS_UNCHECKEDNORMAL, NULL, TS_DRAW, &partSize);
		if (SUCCEEDED(hr))
		{
			szMetrics[uiMetricRadiobuttonCX] = partSize.cx;
			szMetrics[uiMetricRadiobuttonCY] = partSize.cy;
		}
	}

}

void GroupedListViewStyle::PopulateFontMetrics(HDC hdc)
{
	HFONT fontOld = NULL, fontTest;
	TEXTMETRIC tm;

	fontTest = GetFont(uiFontItem);
	if (NULL != fontTest)
	{
		HFONT hf = (HFONT)SelectObject(hdc, fontTest);
		if (NULL == fontOld) fontOld = hf;
	}
	else if(NULL != fontOld)
	{
		SelectObject(hdc, fontOld);
		fontOld = NULL;
	}
			
	if (GetTextMetrics(hdc, &tm))
		szMetrics[uiMetricItemFontCY] = tm.tmHeight;

	fontTest = GetFont(uiFontGroup);
	if (NULL != fontTest)
	{
		HFONT hf = (HFONT)SelectObject(hdc, fontTest);
		if (NULL == fontOld) fontOld = hf;
	}
	else if(NULL != fontOld)
	{
		SelectObject(hdc, fontOld);
		fontOld = NULL;
	}
			
	if (GetTextMetrics(hdc, &tm))
		szMetrics[uiMetricGroupFontCY] = tm.tmHeight;

	fontTest = GetFont(uiFontTitle);
	if (NULL != fontTest)
	{
		HFONT hf = (HFONT)SelectObject(hdc, fontTest);
		if (NULL == fontOld) fontOld = hf;
	}
	else if(NULL != fontOld)
	{
		SelectObject(hdc, fontOld);
		fontOld = NULL;
	}
			
	if (GetTextMetrics(hdc, &tm))
		szMetrics[uiMetricTitleFontCY] = tm.tmHeight;
	
	if (NULL != fontOld)
		SelectObject(hdc, fontOld);
}

void GroupedListViewStyle::Update(HWND hwndHost, UINT updateFlags)
{
	if (0 == updateFlags) return;

	if (0 != (updateFlagThemes & updateFlags))
	{
		CloseThemes();
		if (UxIsAppThemed()) OpenThemes(hwndHost);
		updateFlags |= (updateFlagColors | updateFlagImages | updateFlagMetrics);
	}
		
	if (0 != (updateFlagImages & updateFlags))
	{
		CloseImages();
		LoadImages();
		updateFlags |= updateFlagMetrics;
	}

	if (0 != (updateFlagColors & updateFlags))
	{
		PopulateColors(hwndHost);
	}

	if (0 != (updateFlagFonts & updateFlags))
	{			
		CloseFonts();
		LoadFonts(hwndHost);
		updateFlags |= updateFlagMetrics;
	}

	if (0 != (updateFlagMetrics & updateFlags))
	{
		HDC hdc = GetDCEx(hwndHost, NULL, DCX_CACHE | DCX_NORESETATTRS);
		PopulateMetrics(hdc);
		if (NULL != hdc) ReleaseDC(hwndHost, hdc);
	}
}

void GroupedListViewStyle::CloseImages()
{
	if (NULL != bitmap)
	{
		DeleteObject(bitmap);
		bitmap = NULL;
	}
}

void GroupedListViewStyle::LoadImages()
{
	CloseImages();

	BITMAPINFOHEADER header;
	BYTE *pixels;
	bitmap = ImageLoader_LoadPngEx(pngInstance, pngResource, &header, (void**)&pixels);
	if (NULL != bitmap)
	{
		INT h = abs(header.biHeight);
		ColorizeImage(pixels, header.biWidth, h, header.biBitCount, 
					szColors[GLStyle::uiColorGroup], szColors[GLStyle::uiColorGroupIcon]);
		
	}


}

void GroupedListViewStyle::OpenThemes(HWND hwndHost)
{
	CloseThemes();
	buttonTheme = UxOpenThemeData(hwndHost, L"Button");
}

void GroupedListViewStyle::CloseThemes()
{
	if (NULL != buttonTheme)
	{
		UxCloseThemeData(buttonTheme);
		buttonTheme = NULL;
	}
}


void GroupedListViewStyle::LoadFonts(HWND hwndHost)
{
	CloseFonts();

	HFONT baseFont = (HFONT)SendMessage(hwndHost, WM_GETFONT, 0, 0L);
	if (NULL == baseFont)
		baseFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

	szFonts[uiFontItem] = baseFont;
	szFonts[uiFontGroup] = baseFont;
	szFonts[uiFontTitle] = NULL;

	LOGFONT lf;
	if (GetObject(baseFont, sizeof(LOGFONT), &lf))
	{
		lf.lfWeight = FW_SEMIBOLD;
		szFonts[uiFontTitle] = CreateFontIndirect(&lf);
	}

}

void GroupedListViewStyle::CloseFonts()
{
	szFonts[uiFontItem] = NULL;
	szFonts[uiFontGroup] = NULL;
	if (NULL != szFonts[uiFontTitle])
	{
		DeleteObject(szFonts[uiFontTitle]);
		szFonts[uiFontTitle] = NULL;
	}
}
BOOL GroupedListViewStyle::SetPngResource(HINSTANCE hInstance, LPCTSTR pszResource)
{
	if (NULL != pngResource && !IS_INTRESOURCE(pngResource))
		free(pngResource);
	pngInstance = hInstance;
	if (IS_INTRESOURCE(pszResource))
		pngResource = (LPTSTR)pszResource;
	else
	{
		INT cch = lstrlen(pszResource) + 1;
		pngResource = (LPTSTR)malloc(sizeof(TCHAR) * cch);
		if (NULL != pngResource || FAILED(StringCchCopy(pngResource, cch, pszResource)))
		{
			if (NULL != pngResource)
			{
				free(pngResource);
				pngResource = NULL;
			}
		}
	}
	return TRUE;
}

BOOL GroupedListViewStyle::DrawIcon(HDC hdc, INT iconId, const RECT *prc)
{
	if (NULL == bitmap)
		return FALSE;

	DIBSECTION ds;
	if (sizeof(DIBSECTION) != GetObject(bitmap, sizeof(DIBSECTION), &ds))
		return FALSE;

	INT left = iconId * szMetrics[uiMetricIconCX];
	if (iconId >= ds.dsBm.bmWidth)
		return FALSE;

	return StretchDIBits(hdc, prc->left, prc->bottom - 1, 
							prc->right - prc->left, prc->top - prc->bottom, 
							left, 0, szMetrics[uiMetricIconCX], szMetrics[uiMetricIconCY],
							ds.dsBm.bmBits, (BITMAPINFO*)&ds.dsBmih, DIB_RGB_COLORS, SRCCOPY);

}

BOOL GroupedListViewStyle::DrawThemeBackground(HDC hdc, INT iPartId, INT iStateId, const RECT *pRect, const RECT *pClipRect)
{
	if (NULL != buttonTheme &&
		SUCCEEDED(UxDrawThemeBackground((UXTHEME)buttonTheme, hdc, iPartId, iStateId, pRect, pClipRect)))
	{
		return TRUE;
	}

	UINT uType = 0, uState = 0;
	switch(iPartId)
	{
		case BP_PUSHBUTTON:		
			uType = DFC_BUTTON; 
			uState = DFCS_BUTTONPUSH; 
			switch(iStateId)
			{
				case PBS_DISABLED: uState |= DFCS_INACTIVE; break;
				case PBS_PRESSED: uState |= DFCS_PUSHED; break;
				case PBS_HOT: uState |= DFCS_HOT; break;
			}
			break;
		case BP_RADIOBUTTON:
			uType = DFC_BUTTON; 
			uState = DFCS_BUTTONRADIO; 
			switch(iStateId)
			{
				case RBS_CHECKEDDISABLED:	uState |= DFCS_INACTIVE | DFCS_CHECKED; break;
				case RBS_CHECKEDHOT:		uState |= DFCS_HOT | DFCS_CHECKED; break;
				case RBS_CHECKEDNORMAL:		uState |= 0  | DFCS_CHECKED; break;
				case RBS_CHECKEDPRESSED:	uState |= DFCS_PUSHED | DFCS_CHECKED; break;
				case RBS_UNCHECKEDDISABLED:	uState |= DFCS_INACTIVE; break;
				case RBS_UNCHECKEDHOT:		uState |= DFCS_HOT; break;
				case RBS_UNCHECKEDNORMAL:	uState |= 0; break;
				case RBS_UNCHECKEDPRESSED:	uState |= DFCS_PUSHED; break;
			}
			break;
		case BP_CHECKBOX:
			uType = DFC_BUTTON; 
			uState = DFCS_BUTTONCHECK; 
			switch(iStateId)
			{
				case CBS_CHECKEDDISABLED:	uState |= DFCS_INACTIVE | DFCS_CHECKED; break;
				case CBS_CHECKEDHOT:		uState |= DFCS_HOT | DFCS_CHECKED; break;
				case CBS_CHECKEDNORMAL:		uState |= 0  | DFCS_CHECKED; break;
				case CBS_CHECKEDPRESSED:	uState |= DFCS_PUSHED | DFCS_CHECKED; break;
				case CBS_MIXEDDISABLED:		uState |= DFCS_INACTIVE | DFCS_CHECKED; break;
				case CBS_MIXEDHOT:			uState |= DFCS_HOT | DFCS_CHECKED; break;
				case CBS_MIXEDNORMAL:		uState |= 0  | DFCS_CHECKED; break;
				case CBS_MIXEDPRESSED:		uState |= DFCS_PUSHED | DFCS_CHECKED; break;
				case CBS_UNCHECKEDDISABLED:	uState |= DFCS_INACTIVE; break;
				case CBS_UNCHECKEDHOT:		uState |= DFCS_HOT; break;
				case CBS_UNCHECKEDNORMAL:	uState |= 0; break;
				case CBS_UNCHECKEDPRESSED:	uState |= DFCS_PUSHED; break;
			}
			break;
	}

	return (0 != uType) ? DrawFrameControl(hdc, (LPRECT)pRect, uType, uState) : FALSE;
}

