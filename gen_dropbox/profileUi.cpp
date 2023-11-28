#include "./main.h"
#include "./plugin.h"
#include "./wasabiApi.h"
#include "./profileManager.h"
#include "./fontHelper.h"
#include "./skinWindow.h"
#include "./resource.h"
#include "./dropWindow.h"
#include "./guiObjects.h"
#include "./preferences.h"

#include <shlwapi.h>
#include <strsafe.h>

#define ITEMMETRICS_MARGIN_LEFT		4
#define ITEMMETRICS_MARGIN_TOP		1
#define ITEMMETRICS_MARGIN_RIGHT		4
#define ITEMMETRICS_MARGIN_BOTTOM	1

#define ITEMMETRICS_TEXT_MARGIN_LEFT		4
#define ITEMMETRICS_TEXT_MARGIN_TOP		2
#define ITEMMETRICS_TEXT_MARGIN_RIGHT	4
#define ITEMMETRICS_TEXT_MARGIN_BOTTOM	6

#define ITEMMETRICS_DESCRIPTION_OFFSET_CY	2
#define ITEMMETRICS_DESCRIPTION_OFFSET_CX	24
#define ITEMMETRICS_DESCRIPTION_LINES_COUNT	3

#define ITEMMETRICS_LISTHEIGHT_MINITEMS		1
#define ITEMMETRICS_LISTHEIGHT_MAXITEMS		5


#define ITEMMETRICS_GLOW_HEIGHT				8
#define ITEMMETRICS_GLOW_FOCUSHEIGHT			2
#define ITEMMETRICS_GLOW_HOTHEIGHT			2

#define IDC_EMPTYVIEW		10000

#define ODS_PRESSED		ODS_CHECKED
#define ODS_FOCUSED		(ODS_FOCUS | ODS_SELECTED)


typedef enum __PofileColor
{
	pcWindow = 0,
	pcWindowText,
	pcTitle,
	pcDescription,
	pcTitleHot,
	pcDescriptionHot,
	pcItem,
	pcItemGlow,
	pcItemFrame,
	pcFocused,
	pcFocusedGlow,
	pcFocusedFrame,
	pcPressed,
	pcPressedGlow,
	pcPressedFrame,
	pcLast,
} ProfileColor;


#define PROFILESELECTOR_PROP		TEXT("DropboxProfileSelector")
#define PROFILELIST_WNDPROC_PROP		TEXT("DropboxProfileListWndProc")
#define PROFILELIST_HOTLIGHT_PROP	TEXT("DropboxProfileListHotlight")
#define PROFILELIST_PRESSED_PROP	TEXT("DropboxProfileListPressed")

#define ProfileList_GetHotlight(__hwnd) (((INT)(INT_PTR)GetProp((__hwnd), PROFILELIST_HOTLIGHT_PROP)) - 1)
#define ProfileList_SetHotlight(__hwnd, __index) SetProp((__hwnd), PROFILELIST_HOTLIGHT_PROP, (HANDLE)(INT_PTR)((__index) + 1))

#define ProfileList_GetPressed(__hwnd) (((INT)(INT_PTR)GetProp((__hwnd), PROFILELIST_PRESSED_PROP)) - 1)
#define ProfileList_SetPressed(__hwnd, __index) SetProp((__hwnd), PROFILELIST_PRESSED_PROP, (HANDLE)(INT_PTR)((__index) + 1))



class ProfileUi : public ProfileCallback
{
protected:
	ProfileUi(HWND hwndHost) 
		: ref(1), hwnd(hwndHost), fontItemTitle(NULL), fontItemDescription(NULL),
			backBuffer(NULL), backDC(NULL), origBuffer(NULL)
	{
		UpdateFonts();
		UpdateColors(TRUE);
	}

	virtual ~ProfileUi() 
	{
		RemoveProp(hwnd, PROFILESELECTOR_PROP);
		PLUGIN_PROFILEMNGR->UnregisterCallback(GUID_NULL, this);
		ReleaseFonts();
		ReleaseBuffer();
	}

public:
	static ProfileUi *CreateInstance(HWND hwnd)
	{
		if (NULL == hwnd) return NULL;

		ProfileUi *instance = new ProfileUi(hwnd);
		if (NULL != instance)
		{
			if (!SetProp(hwnd, PROFILESELECTOR_PROP, (HANDLE)instance))
			{
				instance->Release();
				instance = NULL;
			}
			else
			{
				PLUGIN_PROFILEMNGR->RegisterCallback(GUID_NULL, instance);
			}
		}
		return instance;
	}

	static ProfileUi *GetInstance(HWND hwnd)
	{
		return (ProfileUi*)GetProp(hwnd, PROFILESELECTOR_PROP);
	}

	ULONG AddRef(void) { return InterlockedIncrement((LONG*)&ref); }
	ULONG Release(void) 
	{
		if (0 == ref)
			return ref;
	
		LONG r = InterlockedDecrement((LONG*)&ref);
		if (0 == r)
			delete(this);
		return r;
	}
	
	void Notify(UINT eventId, const UUID *profileUid) 
	{
		SendMessage(hwnd, PMM_PROFILECHANGED, (WPARAM)eventId, (LPARAM)profileUid);
	}

	HFONT GetTitleFont() { return fontItemTitle; }
	HFONT GetDescriptionFont() { return fontItemDescription; }
	HDC GetBackDC(HDC hdc, INT cx, INT cy) 
	{
		if (NULL == backDC)
		{
			backDC = CreateCompatibleDC(hdc);
			if (NULL == backDC) return NULL;
		}

		BITMAP bi;
		if (NULL == backBuffer ||
			sizeof(BITMAP) != GetObject(backBuffer, sizeof(BITMAP), &bi) ||
			bi.bmWidth < cx || bi.bmHeight < cy)
		{

			if (NULL != backBuffer)
			{
				SelectObject(backDC, origBuffer);
				origBuffer = NULL;

				DeleteObject(backBuffer);
			}

			backBuffer = CreateCompatibleBitmap(hdc, cx, cy);

			if (NULL == backBuffer) return NULL;
			origBuffer = (HBITMAP)SelectObject(backDC, backBuffer);
		}


		return backDC; 
	}

	COLORREF GetColor(INT colorIndex) { return colorTable[colorIndex]; }

	void SkinChanged()
	{
		UpdateFonts();
		UpdateColors(TRUE);
	}

protected:
	void UpdateFonts()
	{	
		
		INT fontHeight = FontHelper_GetSysFontHeight();

		LOGFONT lf;
		ZeroMemory(&lf, sizeof(LOGFONT));
		
		lf.lfItalic = FALSE;
		lf.lfUnderline = FALSE;
		lf.lfStrikeOut = FALSE;
		lf.lfCharSet = DEFAULT_CHARSET;
		lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
		lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
		lf.lfQuality = DEFAULT_QUALITY;
		lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;

		LPCTSTR pszFonts[] = { TEXT("MS Shell Dlg 2"), TEXT("MS Shell Dlg"), };
		FontHelper_PickFontName(NULL, &lf, pszFonts, ARRAYSIZE(pszFonts));

		lf.lfHeight = fontHeight;
		lf.lfWeight = FW_SEMIBOLD;
		fontItemTitle = CreateFontIndirect(&lf);

		lf.lfHeight = fontHeight;
		lf.lfWeight = FW_DONTCARE;
		fontItemDescription = CreateFontIndirect(&lf);
	}

	void ReleaseFonts()
	{
		if (NULL != fontItemDescription)
		{
			DeleteObject(fontItemDescription);
			fontItemDescription = NULL;
		}

		if (NULL != fontItemTitle)
		{
			DeleteObject(fontItemTitle);
			fontItemTitle = NULL;
		}
	}

	void ReleaseBuffer()
	{		
		if (NULL != origBuffer && NULL != backDC)
		{
			SelectObject(backDC, origBuffer);
			origBuffer = NULL;
		}

		if (NULL != backBuffer)
		{
			DeleteObject(backBuffer);
			backBuffer = NULL;
		}
		
		if (NULL != backDC)
		{
			DeleteDC(backDC);
			backDC = NULL;
		}
	}

	void UpdateColors(BOOL skinned)
	{
		INT d1, d2;
		COLORREF rgbFrame1, rgbFrame2, rgbBack, rgbHighlight;

		rgbBack = GetSkinColor(COLOR_WINDOW);
		rgbHighlight = GetSkinColor(COLOR_HIGHLIGHT);

		rgbFrame1 = BlendColorsF(GetSkinColor(COLOR_DIALOGTEXT), GetSkinColor(COLOR_DIALOG), 0.3f);
		d1 = GetColorDistance(rgbBack, rgbFrame1);
		if (d1 < 0) d1 = -d1;

		rgbFrame2 = rgbHighlight;
		d2 = GetColorDistance(rgbBack, rgbFrame2);
		if (d2 < 0) d2 = -d2;
		if (d2 < 50)
		{
			rgbFrame2 = GetSkinColor(COLOR_HIGHLIGHTTEXT);
			d2 = GetColorDistance(rgbBack, rgbFrame2);
			if (d2 < 0) d2 = -d2;
		}
		if (d2 < 160)
		{
			COLORREF rgbTmp = rgbFrame1;
			rgbFrame1 = rgbFrame2;
			rgbFrame2 = rgbTmp;
		}


		colorTable[pcWindow] = GetSkinColor(COLOR_DIALOG);
		colorTable[pcWindowText] = GetSkinColor(COLOR_DIALOGTEXT);
		colorTable[pcTitle] = BlendColorsF(GetSkinColor(COLOR_WINDOWTEXT), rgbBack, 0.8f);
		colorTable[pcDescription] = BlendColorsF(GetSkinColor(COLOR_WINDOWTEXT), rgbBack, 0.6f);
		colorTable[pcTitleHot] = BlendColorsF(GetSkinColor(COLOR_WINDOWTEXT), rgbBack, 0.9f);
		colorTable[pcDescriptionHot] = BlendColorsF(GetSkinColor(COLOR_WINDOWTEXT), rgbBack, 0.7f);
		
		colorTable[pcItem] = rgbBack;
		colorTable[pcItemFrame] = rgbFrame1;
		colorTable[pcItemGlow] = BlendColorsF(colorTable[pcItemFrame], colorTable[pcItem], 0.15f);
		
		colorTable[pcFocused] = BlendColorsF(rgbFrame2, rgbBack, 0.04f);
		colorTable[pcFocusedFrame] = rgbFrame2;
		colorTable[pcFocusedGlow] = BlendColorsF(colorTable[pcFocusedFrame], colorTable[pcFocused], 0.1f);
		
		colorTable[pcPressed] = BlendColorsF(rgbFrame2, rgbBack, 0.15f);
		colorTable[pcPressedGlow] = BlendColorsF(rgbFrame2, rgbBack, 0.25f);
		colorTable[pcPressedFrame] = rgbFrame2;
	}

protected:
	ULONG ref;
	HWND hwnd;

	HFONT	fontItemTitle;
	HFONT	fontItemDescription;
	HBITMAP backBuffer;
	HBITMAP origBuffer;
	HDC		backDC;
	COLORREF colorTable[pcLast];
};

static INT_PTR CALLBACK ProfileUi_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK ProfileList_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static void ProfileList_PaintBackground(HWND hwnd, HDC hdc, const RECT *prcPaint);

extern HWND ProfileUiEmpty_CreateView(HWND hParent, INT_PTR controlId);

HWND ProfileManager::CreateView(HWND hParent, INT x, INT y, INT cx, INT cy, INT_PTR controlId)
{
	HWND hInstance = WASABI_API_CREATEDIALOGPARAMW(IDD_PROFILEUI, hParent, 
		ProfileUi_DialogProc, 0);

	if (NULL == hInstance)
		return NULL;

	SetWindowPos(hInstance, NULL, x, y, cx, cy, SWP_NOACTIVATE | SWP_NOZORDER);
	
	if (0 != controlId)
		SetWindowLongPtr(hInstance, GWLP_ID, (LONG)controlId);
	
	return hInstance;
}

static INT ProfileUi_MeasureListItem(HWND hwnd)
{
	HWND hControl = GetDlgItem(hwnd, IDC_PROFILELIST);
	if (NULL == hControl) return 0;
	
	ProfileUi *instance = ProfileUi::GetInstance(hwnd);
	if (NULL == instance) return 0;
		
	INT itemHeight = 0;
	
	HDC hdc = GetDCEx(hControl, NULL, DCX_CACHE | DCX_NORESETATTRS);
	if (NULL != hdc)
	{
		TEXTMETRIC tm;
		itemHeight  = ITEMMETRICS_MARGIN_TOP + ITEMMETRICS_MARGIN_BOTTOM + 
					ITEMMETRICS_TEXT_MARGIN_TOP + ITEMMETRICS_TEXT_MARGIN_BOTTOM +
					ITEMMETRICS_DESCRIPTION_OFFSET_CY;
		
		HFONT hfo = (HFONT)SelectObject(hdc, instance->GetTitleFont());
		if (GetTextMetrics(hdc, &tm))
			itemHeight += tm.tmHeight;

		SelectObject(hdc, instance->GetDescriptionFont());

		if (GetTextMetrics(hdc, &tm))
			itemHeight += ((tm.tmHeight + 1) * ITEMMETRICS_DESCRIPTION_LINES_COUNT);

		SelectObject(hdc, hfo);
		ReleaseDC(hControl, hdc);
	}
	return itemHeight;			
}


static INT ProfileUi_GetIdealListHeight(HWND hwnd)
{
	HWND hList = GetDlgItem(hwnd, IDC_PROFILELIST);
	if (NULL == hList) 	return 0;

	INT lineCount = (INT)SendMessage(hList, LB_GETCOUNT, 0, 0L);
	if (lineCount < ITEMMETRICS_LISTHEIGHT_MINITEMS)
		lineCount = ITEMMETRICS_LISTHEIGHT_MINITEMS;
	else if (lineCount > ITEMMETRICS_LISTHEIGHT_MAXITEMS)
		lineCount = ITEMMETRICS_LISTHEIGHT_MAXITEMS;

	INT lineHeight = ProfileUi_MeasureListItem(hwnd);

	return lineHeight * lineCount;
}

static INT ProfileUi_CompareListItem(HWND hwnd, LCID locale, UINT itemId1, ULONG_PTR itemData1, UINT itemId2, ULONG_PTR itemData2)
{
	TCHAR szText1[512], szText2[512];

	if (((ULONG_PTR)-1) == itemData1 || NULL == itemData1 ||
		FAILED(((Profile*)itemData1)->GetName(szText1, ARRAYSIZE(szText1))))
	{
		szText1[0] = TEXT('\0');
	}

	if (((ULONG_PTR)-1) == itemData2 || NULL == itemData2 ||
		FAILED(((Profile*)itemData2)->GetName(szText2, ARRAYSIZE(szText2))))
	{
		szText2[0] = TEXT('\0');
	}
	return (CompareString(locale, 0, szText1, -1, szText2, -1) - 2);
}

static void SetVertex(TRIVERTEX *pVertex, INT x, INT y, COLORREF rgb)
{
	pVertex->x = x;
	pVertex->y = y;
	pVertex->Red    = GetRValue(rgb) << 8;
	pVertex->Green  = GetGValue(rgb) << 8;
	pVertex->Blue   = GetBValue(rgb) << 8;
	pVertex->Alpha  = 0x0000;
}

static void ProfileUi_FrameRect(HDC hdc, const RECT *itemRect, INT widthLeft, INT widthTop, INT widthRight, INT widthBottom, COLORREF rgbFrame)
{
	RECT rcPart;
	
	SetBkColor(hdc, rgbFrame);	
	SetRect(&rcPart, itemRect->left, itemRect->top, itemRect->right, itemRect->top + widthTop); 
	ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcPart, NULL, 0, NULL);
	SetRect(&rcPart, itemRect->left, itemRect->bottom - widthBottom, itemRect->right, itemRect->bottom); 
	ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcPart, NULL, 0, NULL);
	SetRect(&rcPart, itemRect->left, itemRect->top + widthTop, 
				itemRect->left + widthLeft, itemRect->bottom - widthBottom); 
	ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcPart, NULL, 0, NULL);
	SetRect(&rcPart, itemRect->right - widthRight, itemRect->top + widthTop, 
				itemRect->right, itemRect->bottom - widthBottom); 
	ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcPart, NULL, 0, NULL);
}

static void ProfileUi_FillBackground(HDC hdc, const RECT *itemRect, COLORREF rgbBk, COLORREF rgbGlow, INT glowHeight)
{
	RECT fillRect;
	if (rgbGlow != rgbBk && (itemRect->bottom - itemRect->top) >= glowHeight * 2)
	{
		
		TRIVERTEX szVertex[4];
		GRADIENT_RECT gradientRect[2];
		gradientRect[0].UpperLeft  = 0;
		gradientRect[0].LowerRight = 1;
		gradientRect[1].UpperLeft  = 2;
		gradientRect[1].LowerRight = 3;
		
		SetRect(&fillRect, itemRect->left, itemRect->top + glowHeight, 
					itemRect->right, itemRect->bottom - (glowHeight + 3));
			
		SetVertex(&szVertex[0], itemRect->left, itemRect->top, rgbGlow);
		SetVertex(&szVertex[1], itemRect->right, itemRect->top + glowHeight, rgbBk);
		SetVertex(&szVertex[2], itemRect->left, itemRect->bottom - glowHeight - 3, rgbBk);
		SetVertex(&szVertex[3], itemRect->right, itemRect->bottom, rgbGlow);

		if (!GradientFill(hdc, szVertex, ARRAYSIZE(szVertex), gradientRect, 1, GRADIENT_FILL_RECT_V) ||
			!GradientFill(hdc, szVertex, ARRAYSIZE(szVertex), gradientRect + 1, 1, GRADIENT_FILL_RECT_V))
			CopyRect(&fillRect, itemRect);
	}
	else
		CopyRect(&fillRect, itemRect);

	if (fillRect.top < fillRect.bottom)
	{
		COLORREF rgbBkOld = SetBkColor(hdc, rgbBk);
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &fillRect, NULL, 0, NULL);
		if (rgbBkOld != rgbBk) SetBkColor(hdc, rgbBkOld);
	}
	
}

static void ProfileUi_PaintListItem(ProfileUi *instance, HDC hdc, ULONG_PTR itemData, const RECT *itemRect, UINT itemState)
{
	Profile *profile = (Profile*)itemData;
	if (NULL == profile) return;
	

	RECT rcPaint;
	
	ProfileUi_FrameRect(hdc, itemRect, 
				ITEMMETRICS_MARGIN_LEFT, ITEMMETRICS_MARGIN_TOP, 
				ITEMMETRICS_MARGIN_RIGHT, ITEMMETRICS_MARGIN_BOTTOM, 
				instance->GetColor(pcWindow));
	
	SetRect(&rcPaint, itemRect->left + ITEMMETRICS_MARGIN_LEFT, 
				itemRect->top + ITEMMETRICS_MARGIN_TOP, 
				itemRect->right - ITEMMETRICS_MARGIN_RIGHT, 
				itemRect->bottom - ITEMMETRICS_MARGIN_BOTTOM);
		
	/*COLORREF rgbGray = GetSkinColor(COLOR_GRAYTEXT);
	
	if (ODS_FOCUSED == (ODS_FOCUSED & itemState))
		frameIndex = (rgbGray == GetDarkerColor(rgbGray, GetSkinColor(COLOR_HIGHLIGHT))) ? 
						COLOR_GRAYTEXT : COLOR_HIGHLIGHT;
	else
		frameIndex = (rgbGray == GetDarkerColor(rgbGray, GetSkinColor(COLOR_HIGHLIGHT))) ? 
						COLOR_HIGHLIGHT : COLOR_GRAYTEXT;*/
	
	COLORREF rgbBk, rgbGlow, rgbFrame, rgbTitle, rgbDesc;
	INT glowHeight;
	INT borderVert = 1;

	rgbTitle = instance->GetColor(pcTitle);
	rgbDesc = instance->GetColor(pcDescription);

	if (0 != (ODS_PRESSED & itemState))
	{
		rgbBk = instance->GetColor(pcPressed);
		rgbGlow = instance->GetColor(pcPressedGlow);
		rgbFrame = instance->GetColor(pcPressedFrame);
		glowHeight = (rcPaint.bottom - rcPaint.top - 2 * borderVert) / 2;
	}
	else if (ODS_FOCUSED == (ODS_FOCUSED & itemState))
	{
		rgbBk = instance->GetColor(pcFocused);
		rgbGlow = instance->GetColor(pcFocusedGlow);
		rgbFrame = instance->GetColor(pcFocusedFrame);
		borderVert = 1;
		glowHeight = ITEMMETRICS_GLOW_FOCUSHEIGHT;
		
	}
	else
	{
		rgbBk = instance->GetColor(pcItem);
		rgbGlow = instance->GetColor(pcItemGlow);
		rgbFrame = instance->GetColor(pcItemFrame);
		glowHeight = ITEMMETRICS_GLOW_HEIGHT;
	}

	if (0 != (ODS_HOTLIGHT & itemState))
	{
		rgbGlow = BlendColorsF(rgbFrame, rgbBk, 0.6f);
		glowHeight = ITEMMETRICS_GLOW_HOTHEIGHT;
	}
	
	if (0 != ((ODS_HOTLIGHT | ODS_PRESSED) & itemState) || 
		ODS_FOCUSED == (ODS_FOCUSED & itemState))
	{
		rgbTitle = instance->GetColor(pcTitleHot);
		rgbDesc = instance->GetColor(pcDescriptionHot);
	}
	ProfileUi_FrameRect(hdc, &rcPaint, 1, borderVert, 1, borderVert, rgbFrame);
	InflateRect(&rcPaint, -1, -borderVert);
	ProfileUi_FillBackground(hdc, &rcPaint, rgbBk, rgbGlow, glowHeight);

	RECT textRect;
	SetRect(&textRect, rcPaint.left + ITEMMETRICS_TEXT_MARGIN_LEFT, rcPaint.top + ITEMMETRICS_TEXT_MARGIN_TOP, 
			rcPaint.right - ITEMMETRICS_TEXT_MARGIN_RIGHT, rcPaint.bottom - ITEMMETRICS_TEXT_MARGIN_BOTTOM);

	TCHAR szText[256];
	if (NULL == profile || FAILED(profile->GetName(szText, ARRAYSIZE(szText))))
		szText[0] = TEXT('\0');

	if (TEXT('\0') == szText[0])
		StringCchCopy(szText, ARRAYSIZE(szText), TEXT("Unknown"));
		
	SetBkMode(hdc, TRANSPARENT);
		
	HFONT hfo = (HFONT)SelectObject(hdc, instance->GetTitleFont());

	SetTextColor(hdc, rgbTitle);
	DrawText(hdc, szText, -1, &textRect, DT_LEFT | DT_TOP | DT_NOPREFIX | DT_SINGLELINE | DT_END_ELLIPSIS);
	
	TEXTMETRIC tm;
	if (GetTextMetrics(hdc, &tm))
	{
		textRect.top += (tm.tmHeight + ITEMMETRICS_DESCRIPTION_OFFSET_CY);
	}

	if (NULL == profile || FAILED(profile->GetDescription(szText, ARRAYSIZE(szText))))
		szText[0] = TEXT('\0');
	
	if (TEXT('\0') != szText[0])
	{
		textRect.left += ITEMMETRICS_DESCRIPTION_OFFSET_CX;
		SetTextColor(hdc, rgbDesc);
		SelectObject(hdc, instance->GetDescriptionFont());
		DrawText(hdc, szText, -1, &textRect, DT_LEFT | DT_TOP | DT_NOPREFIX | DT_WORD_ELLIPSIS | DT_WORDBREAK);
	}

	SelectObject(hdc, hfo);
	

}

static void ProfileUi_ReleaseListItem(HWND hwnd, UINT itemId, ULONG_PTR itemData)
{
	if (NULL != itemData)
		((Profile*)itemData)->Release();
}



static void ProfileSelect_AttachList(HWND hwnd)
{
	HWND hList = GetDlgItem(hwnd, IDC_PROFILELIST);
	if (NULL == hList) return;
	
	WNDPROC windowProc = (WNDPROC)(LONG_PTR)SetWindowLongPtr(hList, GWLP_WNDPROC, (LONGX86)(LONG_PTR)ProfileList_WindowProc);
	if (NULL != windowProc && !SetProp(hList, PROFILELIST_WNDPROC_PROP, windowProc))
	{
		SetWindowLongPtr(hList, GWLP_WNDPROC, (LONGX86)(LONG_PTR)windowProc);
		windowProc = NULL;
	}
}


static void ProfileUi_UpdateLayout(HWND hwnd, BOOL bRedraw)
{
	HWND hControl;
	RECT clientRect, controlRect;
	if (!GetClientRect(hwnd, &clientRect))
		return;

	ProfileUi *instance = ProfileUi::GetInstance(hwnd);
	if (NULL == instance) return;
		
	LONG listTop = clientRect.top;
	LONG listBottom = clientRect.bottom;
	UINT swpFlags = SWP_NOZORDER | SWP_NOACTIVATE | ((FALSE == bRedraw) ? SWP_NOREDRAW : 0);

	if (NULL != (hControl = GetDlgItem(hwnd, IDC_PROFILELIST))
		&& GetWindowRect(hControl, &controlRect))
	{
		MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&controlRect, 2);

		LONG cx = (clientRect.right - clientRect.left) - 2 * (controlRect.left - clientRect.left);
		LONG cy = listBottom - listTop;
		LONG y = listTop;

		LONG idealCY = ProfileUi_GetIdealListHeight(hwnd);
		if (cy > idealCY) 
		{			
			y = clientRect.top + ((clientRect.bottom - clientRect.top) - idealCY) /2;
			if (y < listTop) y = listTop;
			cy = idealCY;
		}
		 
		if (SetWindowPos(hControl, NULL, clientRect.left, y, cx, cy, swpFlags))
		{
			if (FALSE != bRedraw)
				InvalidateRect(hControl, NULL, TRUE);
		}
	}

	if (NULL != (hControl = GetDlgItem(hwnd, IDC_EMPTYVIEW)))
	{
		CopyRect(&controlRect, &clientRect);
		InflateRect(&controlRect, -4, -4);
		SetWindowPos(hControl, NULL, controlRect.left, controlRect.top, 
			controlRect.right - controlRect.left, controlRect.bottom - controlRect.top, swpFlags);
	}
}

static void ProfileUi_ShowEmpty(HWND hwnd, BOOL fShow)
{
	HWND hEmpty = GetDlgItem(hwnd, IDC_EMPTYVIEW);
	HWND hList = GetDlgItem(hwnd, IDC_PROFILELIST);

	if (fShow)
	{
		if (NULL != hEmpty) return;
		hEmpty = ProfileUiEmpty_CreateView(hwnd, IDC_EMPTYVIEW);
		if (NULL != hEmpty)
		{
			ProfileUi_UpdateLayout(hwnd, FALSE);
			ShowWindow(hEmpty, SW_SHOWNA);
			ShowWindow(hList, SW_HIDE);
		}

	}
	else
	{
		if (NULL != hEmpty)
		{
			if (NULL != hList)
				ShowWindow(hList, SW_SHOWNA);
			DestroyWindow(hEmpty);
		}
	}
}

static INT ProfileUi_LoadProfiles(HWND hwnd, const GUID *selectUid)
{
	HWND hList = GetDlgItem(hwnd, IDC_PROFILELIST);
	if (NULL == hList) 	return 0;

	SendMessage(hwnd, WM_SETREDRAW, FALSE, 0L);
	SendMessage(hList, LB_RESETCONTENT, 0, 0L);
	
	Profile *szProfiles[256];

	INT cchLoaded = PLUGIN_PROFILEMNGR->LoadProfiles(szProfiles, ARRAYSIZE(szProfiles));
	
	for (INT i = 0; i < cchLoaded; i++)
	{
		if (CB_ERR == SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)szProfiles[i]))
			szProfiles[i]->Release();
	}

	
	ProfileUi_ShowEmpty(hwnd, 0 == cchLoaded);
	ProfileUi_UpdateLayout(hwnd, FALSE);
	SendMessage(hwnd, WM_SETREDRAW, TRUE, 0L);
	RedrawWindow(hwnd,NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
	return cchLoaded;
}
static INT_PTR ProfileUi_OnInitDialog(HWND hwnd, HWND hFocus, LPARAM lParam)
{
	ProfileUi::CreateInstance(hwnd);

	HWND hList = GetDlgItem(hwnd, IDC_PROFILELIST);
	SendMessage(hList, LB_SETITEMHEIGHT,  0, (LPARAM)ProfileUi_MeasureListItem(hwnd));

	INT iLoaded = ProfileUi_LoadProfiles(hwnd, NULL);
	if (0 == iLoaded)
	{

	}
	
	ProfileSelect_AttachList(hwnd);
	MlSkinWindowEx(hList, SKINNEDWND_TYPE_LISTBOX, SWS_USESKINCURSORS);

	MONITORINFO mi;
	mi.cbSize = sizeof(MONITORINFO);
	HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
	if (NULL != hMonitor && GetMonitorInfo(hMonitor, &mi))
	{
		RECT windowRect;
		GetWindowRect(hwnd, &windowRect);
		INT offsetX = mi.rcWork.left + ((mi.rcWork.right - mi.rcWork.left) - (windowRect.right - windowRect.left)) / 2 - windowRect.left;
		INT offsetY = mi.rcWork.top + ((mi.rcWork.bottom - mi.rcWork.top) - (windowRect.bottom - windowRect.top)) / 2 - windowRect.top;
		SetWindowPos(hwnd, NULL, windowRect.left + offsetX, windowRect.top + offsetY, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
	}
	SendMessage(hwnd, DM_REPOSITION, 0, 0L);

	if (NULL != hList)
	{
		SendMessage(hList, LB_SETCARETINDEX, (WPARAM)0, 0L);
		SendMessage(hList, LB_SETCURSEL, (WPARAM)0, 0L);
		
		if (IsWindowEnabled(hList))
            PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)hList, TRUE);
	}

	if (1 == iLoaded)
	{
		PostMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDC_PROFILELIST, LBN_DBLCLK), (LPARAM)hList);
	}
	
	return FALSE;
}


static void ProfileUi_OnDestroy(HWND hwnd)
{
	ProfileUi *instance = ProfileUi::GetInstance(hwnd);
	if (NULL != instance) instance->Release();
}

static void ProfileUi_OnCommand(HWND hwnd, INT controlId, INT eventId, HWND hControl)
{
	switch(controlId)
	{
		case IDOK:
		case IDCANCEL:
			break;
		case IDC_PROFILELIST:
			switch(eventId)
			{
				case LBN_SETFOCUS:
					if (0 != (0x8000 & GetAsyncKeyState(VK_TAB)))
					{						
						INT index = 0;
						if (0 != (0x8000 & GetAsyncKeyState(VK_SHIFT)))
						{							
							index = (INT)SendMessage(hControl, LB_GETCOUNT, 0, 0L);
							if (index > 0) index--;
						}
						SendMessage(hControl, LB_SETCARETINDEX, index, FALSE);
						SendMessage(hControl, LB_SETCURSEL, index, FALSE);
					}
					break;
				case LBN_DBLCLK:
					{
						HWND hParent = GetParent(hwnd);
						if (NULL != hParent)
							SendMessage(hParent, WM_COMMAND, 
									MAKEWPARAM(GetDlgCtrlID(hwnd), PMN_PROFILESELECTED), 
									(LPARAM)hwnd);
					}
					break;
			}
			break;
		case IDC_BUTTON_CREATE:
			switch(eventId)
			{
				case BN_CLICKED:
					Plugin_ShowPreferences();
					break;
			}
			break;
	}
}

static LRESULT ProfileUi_OnDeleteItem(HWND hwnd, INT controlId, DELETEITEMSTRUCT *pdis)
{
	switch(controlId)
	{
		case IDC_PROFILELIST:
			ProfileUi_ReleaseListItem(hwnd, pdis->itemID, pdis->itemData);
			return TRUE;
	}
	return FALSE;
}

static LRESULT ProfileUi_OnMeasureItem(HWND hwnd, INT controlId, MEASUREITEMSTRUCT *pmis)
{
	
	switch(controlId)
	{
		case IDC_PROFILELIST:
			pmis->itemHeight = ProfileUi_MeasureListItem(hwnd);
			return TRUE;
	}
	return FALSE;
}

static BOOL ProfileUi_DrawItemBackBuffer(HWND hwnd, DRAWITEMSTRUCT *pdis)
{
	ProfileUi *instance = ProfileUi::GetInstance(hwnd);
	if (NULL == instance)
		return FALSE;
			
	RECT rcItem;
	SetRect(&rcItem, 0, 0, pdis->rcItem.right - pdis->rcItem.left, pdis->rcItem.bottom - pdis->rcItem.top);

	HDC backDC = instance->GetBackDC(pdis->hDC, rcItem.right, rcItem.bottom);
	if (NULL == backDC) return FALSE;
	
	ProfileUi_PaintListItem(instance, backDC, pdis->itemData, &rcItem, pdis->itemState);
	return BitBlt(pdis->hDC, pdis->rcItem.left, pdis->rcItem.top, rcItem.right, rcItem.bottom, backDC, 0, 0, SRCCOPY);
}

static LRESULT ProfileUi_OnDrawItem(HWND hwnd, INT controlId, DRAWITEMSTRUCT *pdis)
{
	switch(controlId)
	{
		case IDC_PROFILELIST:
			if (-1 ==pdis->itemID)
			{
				ProfileList_PaintBackground(pdis->hwndItem, pdis->hDC, &pdis->rcItem);
			}
			else
			{
				if (!ProfileUi_DrawItemBackBuffer(hwnd, pdis))
				{
					ProfileUi *instance = ProfileUi::GetInstance(hwnd);
					if (NULL != instance)
					{
						ProfileUi_PaintListItem(instance, pdis->hDC, pdis->itemData, 
								&pdis->rcItem, pdis->itemState);
					}
				}
			}
			return TRUE;
	}
	return FALSE;
}

static LRESULT ProfileUi_OnCompareItem(HWND hwnd, INT controlId, COMPAREITEMSTRUCT *pcis)
{
	switch(controlId)
	{
		case IDC_PROFILELIST:
			return ProfileUi_CompareListItem(hwnd, pcis->dwLocaleId, pcis->itemID1, pcis->itemData1, 
						pcis->itemID2, pcis->itemData2);
	}
	return 0;
}

static INT_PTR ProfileUi_SetWindowScheme(HWND hwnd, HDC hdc)
{
	ProfileUi *instance = ProfileUi::GetInstance(hwnd);
	if (NULL == instance) return FALSE;

	SetTextColor(hdc, instance->GetColor(pcWindowText));
	SetBkColor(hdc, instance->GetColor(pcWindow));
	return (INT_PTR)GetSkinBrush(COLOR_DIALOG);
}
static INT_PTR ProfileUi_OnDialogColor(HWND hwnd, HDC hdc, HWND hDialog)
{
	return ProfileUi_SetWindowScheme(hwnd, hdc);
}

static INT_PTR ProfileUi_OnListboxColor(HWND hwnd, HDC hdc, HWND hList)
{
	return ProfileUi_SetWindowScheme(hwnd, hdc);
}

static INT_PTR ProfileUi_OnStaticColor(HWND hwnd, HDC hdc, HWND hStatic)
{
	return ProfileUi_SetWindowScheme(hwnd, hdc);
}

static INT_PTR ProfileUi_OnCharToItem(HWND hwnd, INT virtualKey, INT caretPos, HWND hControl)
{
	return -1;
}

static INT_PTR ProfileUi_OnKeyToItem(HWND hwnd, INT virtualKey, INT caretPos, HWND hControl)
{
	switch(virtualKey)
	{
		case VK_TAB:
			{
				BOOL reverseTab = (0 != (0x8000 & GetAsyncKeyState(VK_SHIFT)));
				INT focused = (INT)SendMessage(hControl, LB_GETCURSEL, 0, 0L);
				if (LB_ERR == focused) focused = 0;
				INT count = (INT)SendMessage(hControl, LB_GETCOUNT, 0, 0L);
				if (count > 0)
				{
					if (!reverseTab)
					{
						focused++;
						if (focused >= count)
						{
							PostMessage(hwnd, WM_NEXTDLGCTL, 0, FALSE);
							return -1;
						}
					}
					else
					{
						focused--;
						if (focused < 0)
						{
							PostMessage(hwnd, WM_NEXTDLGCTL, 1, FALSE);
							return -1;
						}
					}
					
					SendMessage(hControl, LB_SETCARETINDEX, focused, FALSE);
					SendMessage(hControl, LB_SETCURSEL, focused, FALSE);
				}
			}
			break;
	}
	return -1;
}


static void ProfileUi_OnWindowPosChanged(HWND hwnd, WINDOWPOS *pwp)
{
	if (SWP_NOSIZE == ((SWP_NOSIZE | SWP_FRAMECHANGED) & pwp->flags)) return;
	ProfileUi_UpdateLayout(hwnd, ((0 == (SWP_NOREDRAW & pwp->flags))));
}

static void ProfileUi_OnSkinChanged(HWND hwnd)
{
	ProfileUi *instance = ProfileUi::GetInstance(hwnd);
	if (NULL != instance) instance->SkinChanged();
	SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED);
}
static BOOL ProfileUi_OnSetProfile(HWND hwnd, UUID *profileUid)
{
	HWND hList = GetDlgItem(hwnd, IDC_PROFILELIST);
	if(NULL == hList || NULL == profileUid) return FALSE;

	INT itemCount = (INT)SendMessage(hList, LB_GETCOUNT, 0, 0L);
	UUID testUid;
	for (INT i = 0; i < itemCount; i++)
	{
		Profile *profile = (Profile*)SendMessage(hList, LB_GETITEMDATA, (WPARAM)i, 0L);
		if (NULL != profile && LB_ERR != ((INT)(INT_PTR)profile) &&
			SUCCEEDED(profile->GetUID(&testUid)) &&
			IsEqualGUID(testUid, *profileUid))
		{
			SendMessage(hList, LB_SETCARETINDEX, i, FALSE);
			SendMessage(hList, LB_SETCURSEL, i, 0L);
			return TRUE;
		}
		
	}
	return FALSE;
}

static Profile* ProfileUi_OnGetProfile(HWND hwnd)
{
		
	INT selectedIndex;
	HWND hList = GetDlgItem(hwnd, IDC_PROFILELIST);
	if (NULL != hList && 
		LB_ERR != (selectedIndex = (INT)SendMessage(hList, LB_GETCURSEL, 0, 0L)))
	{
		Profile *profile = (Profile*)SendMessage(hList, LB_GETITEMDATA, (WPARAM)selectedIndex, 0L);
		if (LB_ERR == ((INT)(INT_PTR)profile))
			profile = NULL;
		
		return profile;
	}

	return NULL;
}

static INT ProfileUi_OnGetIdealHeight(HWND hwnd)
{
	LONG idealCY = ProfileUi_GetIdealListHeight(hwnd);
	return idealCY + 2 * 8;
}

static void ProfileUi_OnProfileChanged(HWND hwnd, UINT eventId, const GUID *profileUid)
{
	switch(eventId)
	{
		case ProfileCallback::eventNameChanged:
			ProfileUi_LoadProfiles(hwnd, profileUid);
			break;

		case ProfileCallback::eventProfileCreated:
			ProfileUi_LoadProfiles(hwnd, profileUid);
			break;

		case ProfileCallback::eventProfileDeleted:
			ProfileUi_LoadProfiles(hwnd, NULL);
			break;
	}
}

static INT_PTR CALLBACK ProfileUi_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG: return ProfileUi_OnInitDialog(hwnd, (HWND)wParam, lParam);
		case WM_DESTROY:		ProfileUi_OnDestroy(hwnd); break;
		case WM_COMMAND:		ProfileUi_OnCommand(hwnd, LOWORD(wParam), HIWORD(wParam), (HWND)lParam); return TRUE;
		case WM_DELETEITEM:	MSGRESULT(hwnd, ProfileUi_OnDeleteItem(hwnd, (INT)wParam, (DELETEITEMSTRUCT*)lParam));	
		case WM_MEASUREITEM:	MSGRESULT(hwnd, ProfileUi_OnMeasureItem(hwnd, (INT)wParam, (MEASUREITEMSTRUCT*)lParam));	
		case WM_DRAWITEM:	MSGRESULT(hwnd, ProfileUi_OnDrawItem(hwnd, (INT)wParam, (DRAWITEMSTRUCT*)lParam));	
		case WM_COMPAREITEM:		return ProfileUi_OnCompareItem(hwnd, (INT)wParam, (COMPAREITEMSTRUCT*)lParam);
		case WM_CTLCOLORLISTBOX:	return ProfileUi_OnListboxColor(hwnd, (HDC)wParam, (HWND)lParam);
		case WM_CTLCOLORDLG:		return ProfileUi_OnDialogColor(hwnd, (HDC)wParam, (HWND)lParam);
		case WM_CTLCOLORSTATIC:	return ProfileUi_OnStaticColor(hwnd, (HDC)wParam, (HWND)lParam);
		case WM_CHARTOITEM:		return ProfileUi_OnCharToItem(hwnd, LOWORD(wParam), HIWORD(wParam), (HWND)lParam);
		case WM_VKEYTOITEM:		return ProfileUi_OnKeyToItem(hwnd, LOWORD(wParam), HIWORD(wParam), (HWND)lParam);
		case WM_WINDOWPOSCHANGED: ProfileUi_OnWindowPosChanged(hwnd, (WINDOWPOS*)lParam); return TRUE;
		case DBM_SKINCHANGED:	ProfileUi_OnSkinChanged(hwnd); return TRUE;
		case PMM_SETPROFILE:		MSGRESULT(hwnd, ProfileUi_OnSetProfile(hwnd, (UUID*)lParam));
		case PMM_GETPROFILE:		MSGRESULT(hwnd, ProfileUi_OnGetProfile(hwnd));
		case PMM_GETIDEALHEIGHT:	MSGRESULT(hwnd, ProfileUi_OnGetIdealHeight(hwnd));
		case PMM_PROFILECHANGED: ProfileUi_OnProfileChanged(hwnd, (UINT)wParam, (const GUID*)lParam); return TRUE; 
	}
	return FALSE;
}

static INT ProfileList_HitTest(HWND hwnd, POINTS pts)
{
	INT index = (INT)SendMessage(hwnd, LB_ITEMFROMPOINT, 0, *((LPARAM*)(&pts)));
	if (LB_ERR != index)
	{
		POINT pt;
		RECT itemRect;
		POINTSTOPOINT(pt, pts);
		if (LB_ERR == SendMessage(hwnd, LB_GETITEMRECT, index, (LPARAM)&itemRect) || !PtInRect(&itemRect, pt))
			index = LB_ERR;
	}
	return index;
}

static void ProfileList_UpdateHotlight(HWND hwnd, POINTS pts)
{	
	INT hotlightIndex = ProfileList_GetHotlight(hwnd);
	INT index = (INT)SendMessage(hwnd, LB_ITEMFROMPOINT, 0, *((LPARAM*)(&pts)));
	if (index == hotlightIndex)
		return;
		
	RECT itemRect;
	if (LB_ERR != hotlightIndex && 
		LB_ERR != SendMessage(hwnd, LB_GETITEMRECT, hotlightIndex, (LPARAM)&itemRect))
	{
		InvalidateRect(hwnd, &itemRect, FALSE);
	}

	if (LB_ERR != index)
	{
		POINT pt;
		POINTSTOPOINT(pt, pts);
		if (LB_ERR == SendMessage(hwnd, LB_GETITEMRECT, index, (LPARAM)&itemRect) || !PtInRect(&itemRect, pt))
		{
			index = LB_ERR;
			SetRectEmpty(&itemRect);
		}
	}
	else
		SetRectEmpty(&itemRect);

	ProfileList_SetHotlight(hwnd, index);
	if (LB_ERR != index)
	{
		
		TRACKMOUSEEVENT tm;
		tm.cbSize = sizeof(TRACKMOUSEEVENT);
		tm.dwFlags = TME_LEAVE;
		tm.hwndTrack = hwnd;
		TrackMouseEvent(&tm);
	}
	
	if (!IsRectEmpty(&itemRect))
	{
		InvalidateRect(hwnd, &itemRect, FALSE);
	}
	
	UpdateWindow(hwnd);
}

static void ProfileList_RemoveHotlight(HWND hwnd)
{
	INT hotlightIndex = ProfileList_GetHotlight(hwnd);
	RemoveProp(hwnd, PROFILELIST_HOTLIGHT_PROP);

	if (LB_ERR != hotlightIndex)
	{
		RECT itemRect;
		if (LB_ERR != SendMessage(hwnd, LB_GETITEMRECT, hotlightIndex, (LPARAM)&itemRect))
		{
			InvalidateRect(hwnd, &itemRect, FALSE);
			UpdateWindow(hwnd);
		}
	}
}

static void ProfileList_RemovePressed(HWND hwnd)
{
	INT pressedIndex = ProfileList_GetPressed(hwnd);
	RemoveProp(hwnd, PROFILELIST_PRESSED_PROP);

	if (LB_ERR != pressedIndex)
	{
		RECT itemRect;
		if (LB_ERR != SendMessage(hwnd, LB_GETITEMRECT, pressedIndex, (LPARAM)&itemRect))
		{
			InvalidateRect(hwnd, &itemRect, FALSE);
			UpdateWindow(hwnd);
		}

		if (GetCapture() == hwnd)
			ReleaseCapture();
	}
}

static void ProfileList_UpdatePressed(HWND hwnd, POINTS pts)
{
	ProfileList_RemovePressed(hwnd);
	ProfileList_UpdateHotlight(hwnd, pts);
	
	INT index = (INT)SendMessage(hwnd, LB_ITEMFROMPOINT, 0, *((LPARAM*)(&pts)));
	if (LB_ERR == index) return;
	
	POINT pt;
	RECT itemRect;
	POINTSTOPOINT(pt, pts);
	if (LB_ERR == SendMessage(hwnd, LB_GETITEMRECT, index, (LPARAM)&itemRect) || !PtInRect(&itemRect, pt))
		return;	

	ProfileList_SetPressed(hwnd, index);
	if (GetCapture() != hwnd)
		SetCapture(hwnd);
	
	InvalidateRect(hwnd, &itemRect, FALSE);
	UpdateWindow(hwnd);
}

static void ProfileList_HotlightCursor(HWND hwnd)
{
	POINT pt;
	GetCursorPos(&pt);
	MapWindowPoints(HWND_DESKTOP, hwnd, &pt, 1);
	LPARAM pts = POINTTOPOINTS(pt);
	ProfileList_UpdateHotlight(hwnd, MAKEPOINTS(pts));
			
}

static void ProfileList_PressSelected(HWND hwnd)
{
	INT selected = (INT)SendMessage(hwnd, LB_GETCURSEL, 0, 0L);
	if (LB_ERR == selected || ProfileList_GetPressed(hwnd) == selected)
		return;
	
	RECT itemRect;

	INT hotlight = ProfileList_GetHotlight(hwnd);
	if (LB_ERR != hotlight && SendMessage(hwnd, LB_GETITEMRECT, hotlight, (LPARAM)&itemRect))
		InvalidateRect(hwnd, &itemRect, FALSE);
	
	ProfileList_SetHotlight(hwnd, selected);
	ProfileList_SetPressed(hwnd, selected);

	if (GetCapture() != hwnd)
		SetCapture(hwnd);
	
	if (SendMessage(hwnd, LB_GETITEMRECT, selected, (LPARAM)&itemRect))
		InvalidateRect(hwnd, &itemRect, FALSE);

	UpdateWindow(hwnd);
}

static void ProfileList_PaintBackground(HWND hwnd, HDC hdc, const RECT *prcPaint)
{
	HWND hParent = GetParent(hwnd);
	
	HBRUSH hb;
	if (NULL != hParent)
		hb = (HBRUSH)SendMessage(hParent, WM_CTLCOLORLISTBOX, (WPARAM)hdc, (LPARAM)hwnd);
	else 
		hb = GetSysColorBrush(COLOR_WINDOW);
	
	FillRect(hdc, prcPaint, hb);
}

static void ProfileList_Paint(HWND hwnd, HDC hdc, const RECT *prcPaint, BOOL fErase)
{

	HWND hParent;
	RECT fillRect;
	CopyRect(&fillRect, prcPaint);

	hParent = GetParent(hwnd);

	INT itemCount = (INT)SendMessage(hwnd, LB_GETCOUNT, 0, 0L);
	if (itemCount > 0)
	{
		INT itemHeight = (INT)SendMessage(hwnd, LB_GETITEMHEIGHT, 0, 0L);
		UINT itemTop = (INT)SendMessage(hwnd, LB_GETTOPINDEX, 0, 0L);
		if (LB_ERR == itemTop) itemTop = 0;

		DRAWITEMSTRUCT drawItem;
		drawItem.CtlID = GetDlgCtrlID(hwnd);
		drawItem.CtlType = ODT_LISTBOX;
		drawItem.hDC = hdc;
		drawItem.hwndItem = hwnd;
		drawItem.itemAction = ODA_DRAWENTIRE;

		drawItem.itemData = 0;
		drawItem.itemState = 0;
		
		if (LB_ERR == (INT)SendMessage(hwnd, LB_GETITEMRECT, itemTop, (LPARAM)&drawItem.rcItem))
			SetRectEmpty(&drawItem.rcItem);
		
		UINT hotlightItem = ProfileList_GetHotlight(hwnd);
		UINT pressedItem = ProfileList_GetPressed(hwnd);
		UINT focusedItem = (hwnd == GetFocus()) ? 
							(INT)SendMessage(hwnd, LB_GETCARETINDEX, 0, 0L) : LB_ERR;

		for (drawItem.itemID = itemTop; drawItem.itemID < (UINT)itemCount && drawItem.rcItem.top < fillRect.bottom; drawItem.itemID++)
		{
			if (drawItem.rcItem.bottom > fillRect.top)
			{
				drawItem.itemState = 0;
				if (drawItem.itemID == hotlightItem) drawItem.itemState |= ODS_HOTLIGHT;
				if (drawItem.itemID == pressedItem)	drawItem.itemState |= ODS_PRESSED;
				if (drawItem.itemID == focusedItem)	drawItem.itemState |= ODS_FOCUSED;
				drawItem.itemData = SendMessage(hwnd, LB_GETITEMDATA, drawItem.itemID, 0L);

				SendMessage(hParent, WM_DRAWITEM, (WPARAM)drawItem.CtlID, (LPARAM)&drawItem);
			}
			OffsetRect(&drawItem.rcItem, 0, itemHeight);
		}
		fillRect.top = drawItem.rcItem.top;

	}

	if (0 != fErase && fillRect.top < fillRect.bottom)
	{
		ProfileList_PaintBackground(hwnd, hdc, &fillRect);
	}
	
}

static void ProfileList_OnPaint(HWND hwnd)
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd, &ps);
	if (NULL == hdc)
		return;
	ProfileList_Paint(hwnd, ps.hdc, &ps.rcPaint, ps.fErase);
	EndPaint(hwnd, &ps);
}

static void ProfileList_OnPrintClient(HWND hwnd, HDC hdc, UINT options)
{
	RECT clientRect;
	GetClientRect(hwnd, &clientRect);
	ProfileList_Paint(hwnd, hdc, &clientRect, 0 != (PRF_ERASEBKGND & options));
}

static LRESULT CALLBACK ProfileList_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	WNDPROC windowProc = (WNDPROC)GetProp(hwnd, PROFILELIST_WNDPROC_PROP);
	if (NULL == windowProc)
		return DefWindowProc(hwnd, uMsg, wParam, lParam);

	switch(uMsg)
	{
		case WM_DESTROY:
			CallWindowProc(windowProc, hwnd, uMsg, wParam, lParam);
			RemoveProp(hwnd, PROFILELIST_WNDPROC_PROP);
			SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)windowProc);
			ProfileList_RemoveHotlight(hwnd);
			ProfileList_RemovePressed(hwnd);
			return 0;
		
		case WM_PAINT:		ProfileList_OnPaint(hwnd); return 0;
		case WM_PRINTCLIENT:	ProfileList_OnPrintClient(hwnd, (HDC)wParam, (UINT)lParam); return 0;
		case WM_ERASEBKGND: return 0;
			

		case WM_MOUSEMOVE:
			if (0 == (MK_LBUTTON & wParam) && LB_ERR == ProfileList_GetPressed(hwnd))
			{
				CallWindowProc(windowProc, hwnd, uMsg, wParam, lParam);
				ProfileList_UpdateHotlight(hwnd, MAKEPOINTS(lParam));
			}
			return 0;

		case WM_MOUSELEAVE:
			ProfileList_RemoveHotlight(hwnd);
			break;

		case WM_LBUTTONDOWN:
			CallWindowProc(windowProc, hwnd, uMsg, wParam, lParam);
			ProfileList_UpdatePressed(hwnd, MAKEPOINTS(lParam));
			return 0;

		case WM_LBUTTONUP:
			if (LB_ERR != ProfileList_GetPressed(hwnd) && 
				ProfileList_GetPressed(hwnd) == ProfileList_HitTest(hwnd, MAKEPOINTS(lParam)))
			{
				SendMessage(GetParent(hwnd), WM_COMMAND, 
					MAKEWPARAM(GetDlgCtrlID(hwnd), LBN_DBLCLK), (LPARAM)hwnd);
			}

			CallWindowProc(windowProc, hwnd, uMsg, wParam, lParam);
			ProfileList_RemovePressed(hwnd);
			ProfileList_UpdateHotlight(hwnd, MAKEPOINTS(lParam));
			return 0;

		case WM_KEYDOWN:
			if (VK_SPACE == wParam)
				ProfileList_PressSelected(hwnd);
			
			if (LB_ERR != ProfileList_GetPressed(hwnd))
				return 0;

			{
				INT pos = GetScrollPos(hwnd, SB_VERT);
				CallWindowProc(windowProc, hwnd, uMsg, wParam, lParam);
				if (pos != GetScrollPos(hwnd, SB_VERT))
					ProfileList_HotlightCursor(hwnd);
			}
			return 0;

		case WM_KEYUP:
			if (LB_ERR != ProfileList_GetPressed(hwnd))
			{
				if (VK_SPACE == wParam)
				{
					if (SendMessage(hwnd, LB_GETCURSEL, 0, 0L) == ProfileList_GetPressed(hwnd))
					{
						SendMessage(GetParent(hwnd), WM_COMMAND, 
								MAKEWPARAM(GetDlgCtrlID(hwnd), LBN_DBLCLK), (LPARAM)hwnd);
					}
					ProfileList_RemovePressed(hwnd);
					ProfileList_HotlightCursor(hwnd);
				}
				return 0;
			}
			
			CallWindowProc(windowProc, hwnd, uMsg, wParam, lParam);
			ProfileList_HotlightCursor(hwnd);
			return 0;

		case WM_MOUSEWHEEL:
			ProfileList_RemoveHotlight(hwnd);
			CallWindowProc(windowProc, hwnd, uMsg, wParam, lParam);
			ProfileList_HotlightCursor(hwnd);
			return 0;
	

		case WM_GETDLGCODE:
			{
				LRESULT result = CallWindowProc(windowProc, hwnd, uMsg, wParam, lParam);
				return result | DLGC_WANTTAB;
			}
			break;
	}

	return CallWindowProc(windowProc, hwnd, uMsg, wParam, lParam);
}