#include "./main.h"
#include "./dropWindowInternal.h"
#include "./resource.h"
#include "./wasabiApi.h"
#include "./imageLoader.h"
#include "./embededEditControl.h"
#include "./playlistDropTarget.h"
#include "./formatData.h"
#include "./skinWindow.h"
#include "./meterbar.h"
#include "./toolbar.h"
#include "./document.h"
#include "./itemView.h"
#include "./imageLoader.h"
#include "./headerCallback.h"
#include <shlwapi.h>
#include <commctrl.h>
#include <strsafe.h>

#define IDC_TITLEEDITOR		1000
#define IDC_TOOLTIP			1001

static LRESULT CALLBACK DropboxHeader_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#define HEADER_DEFAULTHEIGHT	52
#define HEADER_DEFAULTWIDTH		72

#define CLIENT_OFFSET_LEFT		3
#define CLIENT_OFFSET_TOP		2
#define CLIENT_OFFSET_RIGHT		3
#define CLIENT_OFFSET_BOTTOM		4

typedef struct __PLTITLE
{
	INT height;
	INT width;
	INT clipWidth;
	HFONT font;
	LPTSTR text;
} PLTITLE;

typedef enum
{
	DBHF_NORMAL		= 0x00000001,
	DBHF_MENUHOVER		= 0x00000002,
	DBHF_MENUPRESSED		= 0x00000004,
	DBHF_NODOCUMENT		= 0x00000010,
	DBHF_ASYNCACTIVE		= 0x00000040,
	DBHF_CAPTUREMASK		= 0x000F0000,
	DBHF_CAPTURENONE	= 0x00000000,
	DBHF_CAPTUREMENU		= 0x00010000,
	DBHF_CAPTURETITLE	= 0x00020000,
	DBHF_CAPTUREMETER	= 0x00030000,
	DBHF_CAPTURETOOL	= 0x00040000,
} DROPBOXHEADER_FLAGS;

typedef struct __DROPBOXHEADER
{	
	IMAGELISTDRAWPARAMS image;
	PLTITLE	title;
	RECT rcTitle;
	DWORD flags;
	Meterbar *meterbar;
	Toolbar  *toolbar;
	QUERYTHEMECOLOR GetThemeColor;
	QUERYTHEMEBRUSH GetThemeBrush;
	PlaylistDropTarget *dropTarget;
	HWND hRestoreFocus;
	HWND hTooltip;
	HBITMAP hbmpMenuOverlay;

} DROPBOXHEADER;

typedef struct __BUTTONCLICK
{
	POINT	pt;
	DWORD	timestamp;
} BUTTONCLICK;

static BOOL IsDoubleClick(POINT pt)
{
	static BUTTONCLICK leftButton;
	
	BOOL doubleClick = FALSE;
	DWORD timestamp = GetTickCount();

	if (timestamp > leftButton.timestamp && 
		GetDoubleClickTime() >= (timestamp - leftButton.timestamp))
	{
		INT offset;
		offset = GetSystemMetrics(SM_CYDOUBLECLK);
		if (leftButton.pt.y >= (pt.y - offset) && leftButton.pt.y <= (pt.y + offset))
		{
			offset = GetSystemMetrics(SM_CXDOUBLECLK);
			if (leftButton.pt.x >= (pt.x - offset) && leftButton.pt.x <= (pt.x + offset))
				doubleClick = TRUE;
		}
	}

	leftButton.pt = pt;
	leftButton.timestamp = timestamp;
	
	return doubleClick;
}

#define GetHeader(__hwnd) ((DROPBOXHEADER*)(LONG_PTR)(LONGX86)GetWindowLongPtr((__hwnd), 0))

static UINT WAML_NOTIFY_DRAGDROP = 0;

BOOL DropboxHeader_RegisterClass(HINSTANCE hInstance)
{
	WNDCLASS wc;
	if (GetClassInfo(hInstance, NWC_DROPBOXHEADER, &wc)) return TRUE;
	
	
	WAML_NOTIFY_DRAGDROP = RegisterWindowMessageW(WAMLM_DRAGDROP);

	ZeroMemory(&wc, sizeof(WNDCLASS));


	wc.hInstance		= hInstance;
	wc.lpszClassName	= NWC_DROPBOXHEADER;
	wc.lpfnWndProc	= DropboxHeader_WindowProc;
	wc.style			= CS_PARENTDC;
	wc.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground	= NULL;
	wc.cbWndExtra	= sizeof(DROPBOXHEADER*);
	
	return ( 0 != RegisterClass(&wc));
}

#define STYLEOP_ADD		0
#define STYLEOP_SET		1
#define STYLEOP_REMOVE	2

#define CHECKSTYLEKEYWORD(__pszTest, __cchTest, __kwName, __styleOp, __kwValue, __result, __retCode)\
	{if(CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, (__pszTest), (__cchTest), (__kwName), -1)){\
		switch(__styleOp){\
			case STYLEOP_ADD: *(__result) |= (__kwValue); break;\
			case STYLEOP_SET: *(__result) = (__kwValue); break;\
			case STYLEOP_REMOVE: *(__result) &= ~(__kwValue); break;\
		}\
		return (KWPARSER_FOUND | (__retCode));}}\

static UINT CALLBACK DropboxHeader_OnKeyword(LPCTSTR pszKeyword, INT cchKeyword, void *user)
{
	DWORD *windowStyle = ((DWORD*)user);
	CHECKSTYLEKEYWORD(pszKeyword, cchKeyword, TEXT("none"), STYLEOP_SET, 0, windowStyle, KWPARSER_ABORT);
	CHECKSTYLEKEYWORD(pszKeyword, cchKeyword, TEXT("mini"), STYLEOP_SET, DBS_CAPTION, windowStyle, KWPARSER_ABORT);
	CHECKSTYLEKEYWORD(pszKeyword, cchKeyword, TEXT("normal"),STYLEOP_SET, DBS_HEADER, windowStyle, KWPARSER_ABORT);
	return KWPARSER_CONTINUE;
}


BOOL DropboxHeader_ParseStyles(LPCTSTR pszString, DWORD *pStyles)
{		
    INT foundKw = ParseKeywords(pszString, -1, TEXT(",;"), TRUE, DropboxHeader_OnKeyword, pStyles);
	return (0 != foundKw);
}

BOOL DropboxHeader_FormatStyles(LPTSTR pszBuffer, INT cchBufferMax, DWORD styles)
{
	HRESULT hr = E_FAIL;
	styles = (DBS_HEADER & styles);
	*pszBuffer = TEXT('\0');
	switch(styles)
	{
		case 0:
			hr = StringCchCopy(pszBuffer, cchBufferMax, TEXT("none"));
			break;
		case DBS_CAPTION:
			hr = StringCchCopy(pszBuffer, cchBufferMax, TEXT("mini"));
			break;
		case DBS_HEADER:
			hr = StringCchCopy(pszBuffer, cchBufferMax, TEXT("normal"));
			break;
	}
	return SUCCEEDED(hr);
}


static HRGN MakeRectRegion(HRGN rgnToUse, LONG left, LONG top, LONG right, LONG bottom)
{
	if (NULL == rgnToUse)
		rgnToUse = CreateRectRgn(left, top, right, bottom);
	else
		SetRectRgn(rgnToUse, left, top, right, bottom);
	return rgnToUse;
}

static HWND DropboxHeader_SetCapture(HWND hwnd, UINT controlFlag)
{
	DROPBOXHEADER *pdh = GetHeader(hwnd);
	if (NULL == pdh) return NULL;
	
	controlFlag &= DBHF_CAPTUREMASK;
	if (DBHF_CAPTURENONE == controlFlag) return NULL;

	HWND hPrev = (GetCapture() != hwnd) ? SetCapture(hwnd) : hwnd;
	pdh->flags = (~DBHF_CAPTUREMASK & pdh->flags) | controlFlag;
	return hPrev;
}
#define CheckCaptureFlag(__flags, __testControl) (0 == (DBHF_CAPTUREMASK & (__flags)) || (__testControl) == (DBHF_CAPTUREMASK & (__flags)))

static void DropboxHeader_MeasureTitle(HWND hwnd)
{
	DROPBOXHEADER *pdh = GetHeader(hwnd);
	if (NULL == pdh) return;
	
	
	HDC hdc = GetDCEx(hwnd, NULL, DCX_CACHE);
	if (NULL == hdc) 
	{
		pdh->title.height = 0;
		pdh->title.width = 0;
		pdh->title.clipWidth = 0;
		return;
	}
	
	HFONT hfo = (HFONT)SelectObject(hdc, pdh->title.font);
		
	if (NULL == pdh->title.text || TEXT('\0') == pdh->title.text)
	{
		TEXTMETRIC tm;
		GetTextMetrics(hdc, &tm);
		pdh->title.height = tm.tmHeight;
		pdh->title.width = 0;
		
	}
	else
	{
		SIZE size;
		INT cchLen = lstrlen(pdh->title.text);
		GetTextExtentPoint32(hdc, pdh->title.text, cchLen, &size);
		pdh->title.height =	size.cy;
		pdh->title.width = size.cx;
	}

	if (NULL != hfo) SelectObject(hdc, hfo);
	ReleaseDC(hwnd, hdc);
	pdh->title.clipWidth = pdh->title.width;
}

static void DropboxHeader_MeasureTitleClippedWidth(HWND hwnd, INT maxWidth)
{
	DROPBOXHEADER *pdh = GetHeader(hwnd);
	if (NULL == pdh) return;
	
	if (maxWidth >= pdh->title.width)
	{
		pdh->title.clipWidth  = pdh->title.width;
		return;
	}
	
	HDC hdc = GetDCEx(hwnd, NULL, DCX_CACHE);
	if (NULL == hdc) 
	{
		pdh->title.clipWidth  = 0;
		return;
	}

	HFONT hfo = (HFONT)SelectObject(hdc, pdh->title.font);
		
	if (NULL == pdh->title.text || TEXT('\0') == pdh->title.text)
		pdh->title.clipWidth = 0;
	else
	{
		RECT rc;
		SetRect(&rc, 0, 0, maxWidth, 0);
		DrawText(hdc, pdh->title.text, -1, &rc, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS | DT_CALCRECT);
		pdh->title.clipWidth = rc.right - rc.left;
	}

	if (NULL != hfo) SelectObject(hdc, hfo);
	ReleaseDC(hwnd, hdc);
}


static BOOL DropboxHeader_CloseTitleEditor(HWND hwnd)
{
	HWND hEditor = GetDlgItem(hwnd, IDC_TITLEEDITOR);
	return (NULL != hEditor) ? DestroyWindow(hEditor) : FALSE;
}
static void DropboxHeader_AutoSizeTitleEditor(HWND hwnd, HRGN rgnInvalid, BOOL bRedraw)
{
	HWND hEditor = GetDlgItem(hwnd, IDC_TITLEEDITOR);
	if (NULL == hEditor) return;

	TCHAR pszText[512];
	
	INT cchText = (INT)SendMessage(hEditor, WM_GETTEXT, ARRAYSIZE(pszText), (LPARAM)pszText);
		
	HDC hdc = GetDCEx(hEditor, NULL, DCX_CACHE);
	if (NULL == hdc) return;
	
	HFONT hfo, hf = (HFONT)SendMessage(hEditor, WM_GETFONT, 0, 0L);
	if (NULL == hf) hf = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	hfo = (NULL != hf) ? (HFONT)SelectObject(hdc, hf) : NULL;

	SIZE sizeText;
	TEXTMETRIC tm;
	GetTextMetrics(hdc, &tm);
	
	if (cchText < 1 || !GetTextExtentPoint32(hdc, pszText, cchText, &sizeText))
		sizeText.cx = 0;
	DWORD margins = (DWORD)SendMessage(hEditor, EM_GETMARGINS, 0, 0L);
	sizeText.cx += LOWORD(margins) + HIWORD(margins) + tm.tmMaxCharWidth + tm.tmOverhang;
	

	if (NULL != hfo) SelectObject(hdc, hfo);
	ReleaseDC(hEditor, hdc);
	
	RECT rEditor, rc;
	if (GetClientRect(hEditor, &rEditor) && 
		GetClientRect(hwnd, &rc))
	{
		MapWindowPoints(hEditor, hwnd, (POINT*)&rEditor, 2);
		rc.right -= CLIENT_OFFSET_RIGHT;
		
		LONG r = (rEditor.left + sizeText.cx);
		if (r > rc.right) r = rc.right;
		if (r != rEditor.right)
		{				
			SetWindowPos(hEditor, NULL, 0, 0, r - rEditor.left, rEditor.bottom - rEditor.top, 
				SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE | SWP_NOOWNERZORDER |
				((!bRedraw) ? SWP_NOREDRAW : 0));

			if (NULL != rgnInvalid && (rEditor.right - rEditor.left) > (r - rEditor.left))
			{
				HRGN rgn = CreateRectRgn(r, rEditor.top, rEditor.right, rEditor.bottom);
				if (NULL != rgn)
				{
					CombineRgn(rgnInvalid, rgnInvalid, rgn, RGN_OR);
					DeleteObject(rgn);
				}
			}
		}
		
	}
	
}

static void CALLBACK DropboxHeader_OnTitleEditorClose(HWND hTitleEditor, DWORD closeCode)
{
	HWND hwnd = GetParent(hTitleEditor);
	if (NULL == hwnd) return;
	
	DROPBOXHEADER *pdh = GetHeader(hwnd);
	if (NULL != pdh && 
		hTitleEditor == GetFocus() &&
		NULL != pdh->hRestoreFocus &&
		IsWindow(pdh->hRestoreFocus) &&
		IsWindowVisible(pdh->hRestoreFocus) &&
		IsWindowEnabled(pdh->hRestoreFocus))
	{
		SetFocus(pdh->hRestoreFocus);
		pdh->hRestoreFocus = NULL;
	}

	
	if (IDOK == closeCode)
	{
		TCHAR szTitle[512];
		SendMessage(hTitleEditor, WM_GETTEXT, ARRAYSIZE(szTitle), (LPARAM)szTitle);
		HWND hDropBox = GetParent(hwnd);
		if (NULL != hDropBox)
			DropboxWindow_SetDocumentName(hDropBox, szTitle);
	}
	InvalidateRect(hwnd, NULL, TRUE);
	
}


static HWND DropboxHeader_BeginTitleEdit(HWND hwnd)
{
	DROPBOXHEADER *pdh = GetHeader(hwnd);

	if (NULL == pdh || 
		0 != ((DBHF_NODOCUMENT | DBHF_ASYNCACTIVE) & pdh->flags) ||
		!IsWindowVisible(hwnd))
	{
		HWND hEditor = GetDlgItem(hwnd, IDC_TITLEEDITOR);
		if (NULL != hEditor) DestroyWindow(hEditor);
		return NULL;
	}

	HWND hEditor = GetDlgItem(hwnd, IDC_TITLEEDITOR);
	if (NULL != hEditor) return hEditor;
	
	hEditor = CreateWindowEx(0, WC_EDIT, pdh->title.text, 
							WS_CHILD | WS_CLIPSIBLINGS | 
							ES_AUTOHSCROLL | ES_LEFT | ES_NOHIDESEL, 
							pdh->rcTitle.left, pdh->rcTitle.top, 
							100, pdh->title.height,  
							hwnd, (HMENU)IDC_TITLEEDITOR, NULL, NULL);
	if (NULL == hEditor)
		return NULL;

	if (!EmbedEditControl(hEditor, DropboxHeader_OnTitleEditorClose))
	{
		DestroyWindow(hEditor);
		return NULL;
	}

	SendMessage(hEditor, WM_SETFONT, (WPARAM)pdh->title.font, 0L);
	SendMessage(hEditor, EM_SETMARGINS, (WPARAM)(EC_LEFTMARGIN | EC_RIGHTMARGIN), MAKELPARAM(0, EC_RIGHTMARGIN));
	DropboxHeader_AutoSizeTitleEditor(hwnd, NULL, TRUE);
	
	SendMessage(hEditor, EM_SETSEL, 0, -1);

	ShowWindow(hEditor, SW_SHOWNORMAL);
	pdh->hRestoreFocus = SetFocus(hEditor);
	
	
	return hEditor;
}

static void AddRegionFromDiff(HRGN rgnDest, RECT *prcOld, RECT *prcNew, HRGN *prgnTmp)
{
	if (EqualRect(prcOld, prcNew))
		return;

	HRGN rgn = (NULL == prgnTmp) ? NULL : *prgnTmp;
					
	rgn = MakeRectRegion(rgn, prcNew->left, prcNew->top, prcNew->right, prcNew->bottom);
	CombineRgn(rgnDest, rgnDest, rgn, RGN_OR);

	if (prcOld->left < prcNew->left)
	{
		rgn = MakeRectRegion(rgn, prcOld->left, prcOld->top, min(prcNew->left, prcOld->right), prcOld->bottom);
		CombineRgn(rgnDest, rgnDest, rgn, RGN_OR);
	}
	if (prcOld->top < prcNew->top)
	{
		rgn = MakeRectRegion(rgn, prcOld->left, prcOld->top, prcOld->right, min(prcNew->top, prcOld->bottom));
		CombineRgn(rgnDest, rgnDest, rgn, RGN_OR);
	}
	if (prcOld->right > prcNew->right)
	{
		rgn = MakeRectRegion(rgn, max(prcNew->right, prcOld->left), prcOld->top, prcOld->right, prcOld->bottom);
		CombineRgn(rgnDest, rgnDest, rgn, RGN_OR);
	}
	if (prcOld->bottom > prcNew->bottom)
	{
		rgn = MakeRectRegion(rgn, prcOld->left, max(prcNew->bottom, prcOld->top), prcOld->right, prcOld->bottom);
		CombineRgn(rgnDest, rgnDest, rgn, RGN_OR);
	}

	if (NULL != prgnTmp)
	{
		*prgnTmp = rgn;
	}
	else
	{
		if (NULL != rgn)
		{
			DeleteObject(rgn);
		}
	}
}

static void DropboxHeader_UpdateLayout(HWND hwnd, HRGN rgnInvalid)
{
	DROPBOXHEADER *pdh = GetHeader(hwnd);
	if (NULL == pdh) return;

	RECT rc;
	if (!GetClientRect(hwnd, &rc)) return;
	
	rc.left += CLIENT_OFFSET_LEFT;
	rc.top += CLIENT_OFFSET_TOP;
	rc.right -= CLIENT_OFFSET_RIGHT;
	rc.bottom -= CLIENT_OFFSET_BOTTOM;

	RECT rcOld;
	HRGN rgn = NULL;

	SetRect(&rcOld, pdh->image.x, pdh->image.y, pdh->image.x + pdh->image.cx, pdh->image.y + pdh->image.cy);

	pdh->image.x = rc.left;
	pdh->image.y = rc.top;
	
	if (NULL == pdh->image.himl ||
		!ImageList_GetIconSize(pdh->image.himl, &pdh->image.cx, &pdh->image.cy))
	{
		pdh->image.cx = 0;
		pdh->image.cy = 0;
	}
	
	if (NULL != rgnInvalid)
	{
		RECT rcNew;
		SetRect(&rcNew, pdh->image.x, pdh->image.y, pdh->image.x + pdh->image.cx, pdh->image.y + pdh->image.cy);
		AddRegionFromDiff(rgnInvalid, &rcOld, &rcNew, &rgn);
	}
	
	if (-1 == pdh->title.height || -1 == pdh->title.width)
		DropboxHeader_MeasureTitle(hwnd);
		
	CopyRect(&rcOld, &pdh->rcTitle);
	
	pdh->rcTitle.left = pdh->image.x + pdh->image.cx + 2;
	pdh->rcTitle.top = rc.top;
	pdh->rcTitle.right = pdh->rcTitle.left + pdh->title.width + 2;
	if (pdh->rcTitle.right >= rc.right || pdh->title.clipWidth != pdh->title.width) 
	{
		if (pdh->rcTitle.right >= rc.right)
		{
			pdh->rcTitle.right = rc.right;
			if (pdh->rcTitle.right < (pdh->rcTitle.left + 2))
				pdh->title.clipWidth = 0;
		}
		DropboxHeader_MeasureTitleClippedWidth(hwnd, pdh->rcTitle.right - pdh->rcTitle.left - 2);
	}
	pdh->rcTitle.bottom = pdh->rcTitle.top + pdh->title.height + 1;
	if (pdh->rcTitle.bottom > rc.bottom) 
		pdh->rcTitle.bottom = rc.bottom;
	
	if (NULL != rgnInvalid)
	{
		AddRegionFromDiff(rgnInvalid, &rcOld, &pdh->rcTitle, &rgn);
	}

	LONG clientTop = pdh->rcTitle.bottom;

	if (NULL != pdh->meterbar)
	{
		RECT rcTmp;
				
		rcTmp.left = pdh->rcTitle.left;
		rcTmp.right = rc.right;

		rcTmp.top = clientTop;
		rcTmp.bottom = rcTmp.top + pdh->meterbar->GetPrefferedHeight(hwnd);
		if (rcTmp.bottom > rc.bottom) 
			rcTmp.bottom = rc.bottom;
		clientTop = rcTmp.bottom;
		

		pdh->meterbar->GetBounds(&rcOld);
		pdh->meterbar->SetBoundsIndirect(&rcTmp);

		if (NULL != rgnInvalid)
		{
			AddRegionFromDiff(rgnInvalid, &rcOld, &rcTmp, &rgn);
		}
	}

	if (NULL != pdh->toolbar)
	{
		RECT rcTmp;
		LONG bottom = rc.bottom + CLIENT_OFFSET_BOTTOM - 1;
		SetRect(&rcTmp, pdh->rcTitle.left, bottom - pdh->toolbar->GetHeight(), rc.right + CLIENT_OFFSET_RIGHT, bottom);
		    			
		if (rcTmp.top < clientTop)
			rcTmp.top = clientTop;
	
		pdh->toolbar->GetBounds(&rcOld);
		pdh->toolbar->SetBoundsIndirect(&rcTmp);

		if (NULL != rgnInvalid)
		{
			AddRegionFromDiff(rgnInvalid, &rcOld, &rcTmp, &rgn);
		}
	}

	if (NULL != rgn)
		DeleteObject(rgn);

}

static COLORREF PremultiplyColor(COLORREF rgbOrig, BYTE alpha)
{		
	UINT r = (alpha * ((UINT)GetRValue(rgbOrig)) + 127)/255;
	UINT g = (alpha * ((UINT)GetGValue(rgbOrig)) + 127)/255;
	UINT b = (alpha * ((UINT)GetBValue(rgbOrig)) + 127)/255;
	return ((alpha << 24) | RGB(r, g, b));
}

static void DropboxHeader_DrawArrow(HBITMAP hbmp, RECT *prcBox, COLORREF rgb)
{
	DIBSECTION dibsec;
	if (!hbmp || sizeof(DIBSECTION) != GetObject(hbmp, sizeof(DIBSECTION), &dibsec) ||
		BI_RGB != dibsec.dsBmih.biCompression || 1 != dibsec.dsBmih.biPlanes || dibsec.dsBm.bmBitsPixel < 24) 
		return;

	LONG pitch;
	INT step = (dibsec.dsBm.bmBitsPixel>>3);
	LPBYTE line, cursor;
	pitch = dibsec.dsBmih.biWidth * step;
	while (pitch%4) pitch++;
	BYTE r,g,b;
	r = GetRValue(rgb); g = GetGValue(rgb); b = GetBValue(rgb);

	INT ofs = (dibsec.dsBmih.biHeight > 0) ? (dibsec.dsBmih.biHeight - prcBox->bottom + 2) : (prcBox->bottom - 8);
	line = ((BYTE*)dibsec.dsBm.bmBits) + pitch * ofs + (prcBox->right - 9) * step;
	INT cx = 7, cy = 5, x;
	for (; cy-- != 0; line += pitch)
	{	
		switch(cy)
		{
			case 0:
			case 1:
				for (x = cx, cursor = line; x-- != 0; cursor += step) 
				{ cursor[0] = b; cursor[1] = g; cursor[2] = r;}
				break;
			case 2:
				for (x = (cx - 2), cursor = line + step; x-- != 0; cursor += step) 
				{ cursor[0] = b; cursor[1] = g; cursor[2] = r;}
				break;
			case 3:
				for (x = (cx - 4), cursor = line + step * 2; x-- != 0; cursor += step) 
				{ cursor[0] = b; cursor[1] = g; cursor[2] = r;}
			case 4:
				for (x = (cx - 6), cursor = line + step * 3; x-- != 0; cursor += step) 
				{ cursor[0] = b; cursor[1] = g; cursor[2] = r;}
				break;
		}
		
	}
}

static void DropboxHeader_LoadImage(HWND hwnd, HINSTANCE hInstance, LPCTSTR pszResourceName)
{
	DROPBOXHEADER *pdh = GetHeader(hwnd);
	if (NULL == pdh)
		return;

	if (NULL != pdh->image.himl)
	{
		ImageList_Destroy(pdh->image.himl);
		pdh->image.himl = NULL;
	}
		
	INT imageCX, imageCY;
	HBITMAP hbmp = ImageLoader_LoadPng(hInstance, pszResourceName, &imageCX, &imageCY);
	if (NULL == hbmp)
		return;

	pdh->image.himl = ImageList_Create(imageCX, imageCY, ILC_COLOR32, 2, 1);
	if (NULL != pdh->image.himl)
	{
		INT index;
		IMAGEINFO imageInfo;
		index = ImageList_Add(pdh->image.himl, hbmp, NULL);

		COLORREF rgbBk = pdh->GetThemeColor(COLOR_BTNFACE);
		COLORREF rgbArrow = pdh->GetThemeColor(COLOR_BTNTEXT);
		COLORREF rgbFade = pdh->GetThemeColor(COLOR_BTNSHADOW);

		if ( -1 != index && ImageList_GetImageInfo(pdh->image.himl, index, &imageInfo))
			BlendOnColor(imageInfo.hbmImage, &imageInfo.rcImage, TRUE, rgbBk);
		
		index = ImageList_Add(pdh->image.himl, hbmp, NULL); 
		if ( -1 != index && ImageList_GetImageInfo(pdh->image.himl, index, &imageInfo))
		{			
			ColorOverImage(imageInfo.hbmImage, &imageInfo.rcImage, TRUE, PremultiplyColor(rgbFade, 0x36));
			DropboxHeader_DrawArrow(imageInfo.hbmImage, &imageInfo.rcImage, rgbArrow);
			BlendOnColor(imageInfo.hbmImage, &imageInfo.rcImage, TRUE, rgbBk);
			
		}

		index = ImageList_Add(pdh->image.himl, hbmp, NULL); 
		if ( -1 != index && ImageList_GetImageInfo(pdh->image.himl, index, &imageInfo))
		{			
			ColorOverImage(imageInfo.hbmImage, &imageInfo.rcImage, TRUE, PremultiplyColor(rgbFade, 0x59));
			DropboxHeader_DrawArrow(imageInfo.hbmImage, &imageInfo.rcImage, rgbArrow);
			BlendOnColor(imageInfo.hbmImage, &imageInfo.rcImage, TRUE, rgbBk);
		}
	}

	pdh->image.rgbBk = CLR_DEFAULT;
	pdh->image.rgbFg = CLR_DEFAULT;
	pdh->image.fStyle = ILD_NORMAL;
	DeleteObject(hbmp);
}

static BOOL DropboxHeader_SetTitle(HWND hwnd, LPCTSTR pszText, BOOL bInvalidate)
{
	DROPBOXHEADER *pdh = GetHeader(hwnd);
	if (NULL == pdh)
		return FALSE;

	if (NULL != pdh->title.text)
	{	
		free(pdh->title.text);
		pdh->title.text = NULL;
	}

	if (NULL != pszText)
	{
		INT cbLen = (lstrlen(pszText) +  1) * sizeof(TCHAR);
		pdh->title.text = (TCHAR*)malloc(cbLen);
		if (NULL == pdh->title.text)
			return FALSE;
		CopyMemory(pdh->title.text, pszText, cbLen);
	}

	pdh->title.height = -1;
	pdh->title.width = -1;
	
	if (bInvalidate)
	{
		RECT rcOld;
		CopyRect(&rcOld, &pdh->rcTitle);

		DropboxHeader_UpdateLayout(hwnd, NULL);
		
		InvalidateRect(hwnd, &rcOld, TRUE);
		InvalidateRect(hwnd, &pdh->rcTitle, TRUE);
	}
	return TRUE;
}
static void DropboxHeader_DisplayContextMenu(HWND hwnd)
{
	DROPBOXHEADER *pdh = GetHeader(hwnd);
	if (NULL == pdh) return;
	
	HWND hParent = GetParent(hwnd);
	if (NULL == hParent) return;

	HMENU hMenu = DropboxWindow_GetMenu(hParent, DBMENU_WINDOWCONTEXT);
	if (NULL == hMenu) return;

	RECT rcImage;
		SetRect(&rcImage, pdh->image.x, pdh->image.y, 
			pdh->image.x + pdh->image.cx, pdh->image.y + pdh->image.cy);
	
	BOOL styleForced = FALSE;

	if (0 == (DBHF_MENUPRESSED & pdh->flags))
	{	
		styleForced = TRUE;
		pdh->flags |= DBHF_MENUPRESSED;
		InvalidateRect(hwnd, &rcImage, FALSE);
		UpdateWindow(hwnd);
	}
	
	POINT ptMenu;
	ptMenu.x = rcImage.left;
	ptMenu.y = rcImage.bottom + 1;
	MapWindowPoints(hwnd, HWND_DESKTOP, &ptMenu, 1);
	
	DWORD windowStyle = GetWindowStyle(hwnd);

	TrackPopup(hMenu,TPM_LEFTALIGN | TPM_TOPALIGN | TPM_VERPOSANIMATION, 
				ptMenu.x, ptMenu.y, hParent, 0 == (DBS_SKINWINDOW & windowStyle));
	
	DropboxWindow_ReleaseMenu(hParent, DBMENU_WINDOWCONTEXT, hMenu);

	pdh->flags &= ~DBHF_MENUPRESSED;
	InvalidateRect(hwnd, &rcImage, FALSE);
}


static void DropboxHeader_UpdateSkinStyle(HWND hwnd)
{
	DROPBOXHEADER *pdh = GetHeader(hwnd);
	if (NULL == pdh) return;

	DWORD windowStyle = GetWindowStyle(hwnd);
	if (DBS_SKINWINDOW & windowStyle)
	{
		pdh->GetThemeColor = GetSkinColor;
		pdh->GetThemeBrush = GetSkinBrush;
	}
	else
	{
		pdh->GetThemeColor = GetSystemColor;
		pdh->GetThemeBrush = GetSystemBrush;
	}
}

static HBRUSH DropboxHeader_GetBkBrush(HWND hwnd)
{
	HBRUSH hb = NULL;
	DWORD windowStyle = GetWindowStyle(hwnd);

	if (0 != (DBS_SKINWINDOW & windowStyle)) 
		hb = GetSkinBrush(COLOR_BTNFACE);

	if (NULL == hb)
		hb = GetSystemBrush(COLOR_BTNFACE);
	
	return hb;
}

static void DropboxHeader_Paint(HWND hwnd, PAINTSTRUCT *pps)
{	
	DROPBOXHEADER *pdh = GetHeader(hwnd);
	if (NULL == pdh) return;

	HDC hdc = pps->hdc;
	RECT rc;
	GetClientRect(hwnd, &rc);

	DWORD windowStyle = GetWindowStyle(hwnd);
	
	SetBkColor(hdc, pdh->GetThemeColor(COLOR_BTNFACE));
	SetTextColor(hdc, pdh->GetThemeColor(COLOR_BTNTEXT));
	SetBkMode(hdc, OPAQUE);
	
	HWND hEditor = GetDlgItem(hwnd, IDC_TITLEEDITOR);

	BOOL drawImage = (pdh->image.himl && 
					pdh->image.x < pps->rcPaint.right && 
					(pdh->image.x + pdh->image.cx ) > pps->rcPaint.left &&
					pdh->image.y < pps->rcPaint.bottom && 
					(pdh->image.y + pdh->image.cy ) > pps->rcPaint.top);
	
	BOOL drawText = (NULL == hEditor && 
					NULL != pdh->title.text && 
					TEXT('\0') != *pdh->title.text &&
					pdh->rcTitle.left < pdh->rcTitle.right && pdh->rcTitle.top < pdh->rcTitle.bottom &&
					pdh->rcTitle.left < pps->rcPaint.right && pdh->rcTitle.right > pps->rcPaint.left &&
					pdh->rcTitle.top < pps->rcPaint.bottom && pdh->rcTitle.bottom > pps->rcPaint.top);
	
	BOOL drawMeterbar = (NULL != pdh->meterbar);

	BOOL drawToolbar = (NULL != pdh->toolbar && 
						0 != (Toolbar::FlagVisible & pdh->toolbar->GetFlags()));

	if (pps->fErase)
	{
		RECT rcFill;
		SetRect(&rcFill, pps->rcPaint.left, pps->rcPaint.top, 	pps->rcPaint.right, 
				pps->rcPaint.bottom - ((pps->rcPaint.bottom == rc.bottom) ? 1 : 0));

		if (drawText || drawImage || drawMeterbar || drawToolbar)
		{
			HRGN rgn1 = CreateRectRgnIndirect(&rcFill);
			HRGN rgn2 = NULL;

			if (drawImage)
			{
				rgn2 = MakeRectRegion(rgn2, pdh->image.x, pdh->image.y, pdh->image.x + pdh->image.cx, pdh->image.y + pdh->image.cy);
				CombineRgn(rgn1, rgn1, rgn2, RGN_DIFF);
			}
			
			if (drawText)
			{
				rgn2 = MakeRectRegion(rgn2, pdh->rcTitle.left + 1, pdh->rcTitle.top + 1, 
								pdh->rcTitle.left +pdh->title.clipWidth, pdh->rcTitle.top + pdh->title.height - 1);
				CombineRgn(rgn1, rgn1, rgn2, RGN_DIFF);
			}
			
			RECT rcBar;
			if (drawMeterbar)
			{
				pdh->meterbar->GetBounds(&rcBar);
				rgn2 = MakeRectRegion(rgn2, rcBar.left, rcBar.top, rcBar.right, rcBar.bottom);
				CombineRgn(rgn1, rgn1, rgn2, RGN_DIFF);
			}

			if (drawToolbar && pdh->toolbar->GetBounds(&rcBar))
			{
				rgn2 = MakeRectRegion(rgn2, rcBar.left, rcBar.top, rcBar.right, rcBar.bottom);
				CombineRgn(rgn1, rgn1, rgn2, RGN_DIFF);
			}
			
			FillRgn(hdc, rgn1, DropboxHeader_GetBkBrush(hwnd));
			
			DeleteObject(rgn1);
			if (NULL != rgn2)
				DeleteObject(rgn2);
		}
		else
			ExtTextOut(hdc, 0,0, ETO_OPAQUE, &rcFill, NULL, 0, NULL);
		
		if(rcFill.bottom != rc.bottom)
		{
			rcFill.top = rcFill.bottom;
			rcFill.bottom = rc.bottom;
			
			COLORREF rgbOld = SetBkColor(hdc, pdh->GetThemeColor(COLOR_BTNSHADOW));
			ExtTextOut(hdc, 0,0, ETO_OPAQUE, &rcFill, NULL, 0, NULL);
			SetBkColor(hdc, rgbOld);
		}
	}

	if (drawText)
	{	
		RECT rt;
		SetRectEmpty(&rt);
		SetRect(&rt, pdh->rcTitle.left, pdh->rcTitle.top, pdh->rcTitle.right, pdh->rcTitle.bottom);

		UINT flags = DT_LEFT | DT_TOP | DT_NOPREFIX | DT_SINGLELINE | DT_NOCLIP;
		if (pdh->rcTitle.right == rc.right - CLIENT_OFFSET_RIGHT)
			flags |= DT_END_ELLIPSIS;
	
		//	else 
	//		flags |= DT_NOCLIP;


		HFONT hfo = (HFONT)SelectObject(hdc, pdh->title.font);
		
		if (0 != (DBHS_ACTIVEHEADER & windowStyle))
		{
			WORD h,l,s;
			INT offset, lumaAdjust;
			COLORREF rgbText, rgbOutline;
			rgbText = GetTextColor(hdc);
			ColorRGBToHLS(rgbText, &h, &l, &s);
			if (l < 100) 
			{
				offset = -1;
				lumaAdjust = 400;
			}
			else if (l > 160)
			{
				offset = 1;
				lumaAdjust = -400;
			}
			else
			{
				offset = 0;
				lumaAdjust = 0;
			}
						
			rgbOutline = (0 != lumaAdjust) ? ColorAdjustLuma(GetBkColor(hdc), lumaAdjust, TRUE) : rgbText;
			
			if (rgbOutline != rgbText)
			{				
				OffsetRect(&rt, offset, offset);
				SetTextColor(hdc, rgbOutline);
				DrawText(hdc, pdh->title.text, -1, &rt, flags);
				SetTextColor(hdc, rgbText);
				OffsetRect(&rt, -offset, -offset);

				SetBkMode(hdc, TRANSPARENT);
			}
		}
					
		DrawText(hdc, pdh->title.text, -1, &rt, flags);

		if (0 != (DBHS_ACTIVEHEADER & windowStyle))
			SetBkMode(hdc, OPAQUE);
		

		if (NULL != hfo) SelectObject(hdc, hfo);
	}

	if (drawImage)
	{	
		BOOL ignoreMenuImage = FALSE;
		if (NULL != pdh->hbmpMenuOverlay)
		{
			DIBSECTION ds;
			if (sizeof(DIBSECTION) == GetObject(pdh->hbmpMenuOverlay, sizeof(DIBSECTION), &ds))
			{
				ignoreMenuImage = StretchDIBits(hdc, pdh->image.x, pdh->image.y + ds.dsBmih.biHeight - 1, ds.dsBmih.biWidth, -ds.dsBmih.biHeight, 
									0, 0, pdh->image.cx, pdh->image.cy, ds.dsBm.bmBits, (BITMAPINFO*)&ds.dsBmih, DIB_RGB_COLORS, SRCCOPY);
			}
		}

		if (!ignoreMenuImage)
		{
			if (0 != (DBHF_MENUPRESSED & pdh->flags)) pdh->image.i = 2;
			else if (0 != (DBHF_MENUHOVER & pdh->flags)) pdh->image.i = 1;
			else pdh->image.i = 0;

			pdh->image.hdcDst = hdc;
			pdh->image.dwRop = SRCCOPY;
			
			ImageList_DrawIndirect(&pdh->image);
		}
	}

	if (drawMeterbar)
	{
		if (!pdh->meterbar->Draw(hdc, &pps->rcPaint))
		{
			RECT rcBar;
			if (pdh->meterbar->GetBounds(&rcBar) &&
				IntersectRect(&rcBar, &rcBar, &pps->rcPaint))
			{
				FillRect(hdc, &rcBar, DropboxHeader_GetBkBrush(hwnd));
			}
		}
	}
	
	if (drawToolbar)
	{
		if (!pdh->toolbar->Draw(hdc, &pps->rcPaint))
		{	/* disaster recovery */
			RECT rcBar;
			if (pdh->toolbar->GetBounds(&rcBar) &&
				IntersectRect(&rcBar, &rcBar, &pps->rcPaint))
			{
				FillRect(hdc, &rcBar, DropboxHeader_GetBkBrush(hwnd));
			}
		}
	}
	
}

static void DropboxHeader_AsyncActivated(HWND hwnd, BOOL bActivated)
{
	DROPBOXHEADER *pdh = GetHeader(hwnd);
	if (NULL == pdh) return;

	if (bActivated) 
		pdh->flags |= DBHF_ASYNCACTIVE;
	else
		pdh->flags &= ~DBHF_ASYNCACTIVE;

	DropboxHeader_CloseTitleEditor(hwnd);

	if (NULL != pdh->toolbar)
		pdh->toolbar->SetFlags((0 == (DBHF_ASYNCACTIVE & pdh->flags)) ? 0 : Toolbar::FlagDisabled, Toolbar::FlagDisabled);
	


	InvalidateRect(hwnd, NULL, TRUE);

}

static BOOL DropboxHeader_HitTestTitle(HWND hwnd, POINT pt)
{
	DROPBOXHEADER *pdh = GetHeader(hwnd);
	if (NULL == pdh) return FALSE;
	RECT rcTitle, rc;
	
	CopyRect(&rcTitle, &pdh->rcTitle);
		
	if ((rcTitle.right - rcTitle.left) < 48) 
		rcTitle.right = (rcTitle.left + 48);
	else 
		rcTitle.right += 20;

	if (GetClientRect(hwnd, &rc))
	{
		if (rcTitle.right > (rc.right - CLIENT_OFFSET_RIGHT)) 
			rcTitle.right = (rc.right - CLIENT_OFFSET_RIGHT);
	}

	return (PtInRect(&rcTitle, pt));
}

static BOOL DropboxHeader_HitTestMenu(HWND hwnd, POINT pt, RECT *prcMenu)
{
	DROPBOXHEADER *pdh = GetHeader(hwnd);
	if (NULL == pdh) return FALSE;

	RECT rcMenu;
	SetRect(&rcMenu, pdh->image.x, pdh->image.y, pdh->image.x + pdh->image.cx, pdh->image.y + pdh->image.cy);
	
	if (PtInRect(&rcMenu, pt))
	{
		if (NULL != prcMenu)
			CopyRect(prcMenu, &rcMenu);
		return TRUE;
	}
	return FALSE;
}

static void DropboxHeader_RelayTooltipEvent(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	DROPBOXHEADER *pdh = GetHeader(hwnd);
	if (NULL == pdh || NULL == pdh->hTooltip) return;

	DWORD pts = GetMessagePos();

	MSG msg;
	msg.hwnd = hwnd;
	POINTSTOPOINT(msg.pt, pts);
	msg.message = uMsg;
	msg.wParam = wParam;
	msg.lParam = lParam;
	msg.time = GetMessageTime();
	SendMessage(pdh->hTooltip, TTM_RELAYEVENT, 0, (LPARAM)&msg);
}
static BOOL DropboxHeader_SetToolbarImagelist(Toolbar *toolbar)
{	
	BITMAPINFOHEADER header;
	BYTE *pixels;
	HBITMAP hbmp = ImageLoader_LoadPngEx(plugin.hDllInstance, MAKEINTRESOURCE(IDR_TOOLBAR_IMAGE), &header, (void**)&pixels);
	if (NULL == hbmp)
		return FALSE;
	
	ColorizeImage(pixels, header.biWidth, abs(header.biHeight), header.biBitCount, 
			toolbar->GetTextBkColor(), toolbar->GetTextColor());
	INT side = toolbar->GetHeight();
	INT initial = header.biWidth / side;
	HIMAGELIST  himl = ImageList_Create(side, side * 4, ILC_COLOR24, initial, 1); 
	if (NULL != himl)
	{
		INT index = ImageList_Add(himl, hbmp, NULL);
		if (-1 == index)
		{
			ImageList_Destroy(himl);
			himl = NULL;
		}
	}
	DeleteObject(hbmp);
	
	toolbar->SetImageList(himl);
	toolbar->Invalidate();
	
	return TRUE;
}

static LRESULT DropboxHeader_OnCreateWindow(HWND hwnd, CREATESTRUCT *pcs)
{
	DROPBOXHEADER *pdh;
	pdh = (DROPBOXHEADER*)malloc(sizeof(DROPBOXHEADER));
	if (NULL == pdh)
	{
		DestroyWindow(hwnd);
		return -1;
	}

	ZeroMemory(pdh, sizeof(DROPBOXHEADER));
	SetLastError(ERROR_SUCCESS);
	if (!SetWindowLongPtr(hwnd, 0, (LONGX86)(LONG_PTR)pdh) && ERROR_SUCCESS != GetLastError())
	{
		free(pdh);
		DestroyWindow(hwnd);
		return -1;
	}
		
	DropboxHeader_UpdateSkinStyle(hwnd);
	
	pdh->image.cbSize = sizeof(IMAGELISTDRAWPARAMS) - 3 * sizeof(DWORD);
	pdh->image.rgbBk = CLR_DEFAULT;
	pdh->image.rgbFg = CLR_DEFAULT;
	pdh->image.fStyle = ILD_NORMAL;

	pdh->title.font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	
	pdh->meterbar = new Meterbar(Meterbar::FlagVisible | Meterbar::FlagUnitCount);
	if (NULL != pdh->meterbar)
		DropboxHeader_RegisterMeterbarCallback(hwnd, pdh->meterbar);
    	

	pdh->toolbar = new Toolbar(18);
	if (NULL != pdh->toolbar)
		DropboxHeader_RegisterToolbarCallback(hwnd, pdh->toolbar);

	if (NULL != pdh->toolbar)
	{
		pdh->toolbar->InsertButton(0, ID_PLAY, 2, MAKEINTRESOURCE(IDS_TOOLBUTTON_PLAY), MAKEINTRESOURCE(IDS_TOOLBUTTON_PLAY_DESC), 0);
		pdh->toolbar->InsertButton(1, ID_ENQUEUE, 9, MAKEINTRESOURCE(IDS_TOOLBUTTON_ENQUEUE), MAKEINTRESOURCE(IDS_TOOLBUTTON_ENQUEUE_DESC), 0);
		pdh->toolbar->InsertFlexSpacer(2);
		pdh->toolbar->InsertButton(3, DBMENU_ARRANGEBY, 8, MAKEINTRESOURCE(IDS_TOOLBUTTON_REORDER), MAKEINTRESOURCE(IDS_TOOLBUTTON_REORDER_DESC), ToolbarButton::FlagDropdownButton);
		pdh->toolbar->InsertSeparator(4);
		pdh->toolbar->InsertButton(5, ID_DELETE, 4, MAKEINTRESOURCE(IDS_TOOLBUTTON_DELETE), MAKEINTRESOURCE(IDS_TOOLBUTTON_DELETE_DESC), 0);
		pdh->toolbar->InsertSeparator(6);
		pdh->toolbar->InsertButton(7, ID_DOCUMENT_NEW, 6, MAKEINTRESOURCE(IDS_TOOLBUTTON_NEWPL), MAKEINTRESOURCE(IDS_TOOLBUTTON_NEWPL_DESC), 0);
		pdh->toolbar->InsertButton(8, ID_DOCUMENT_OPEN, 5, MAKEINTRESOURCE(IDS_TOOLBUTTON_OPENPL), MAKEINTRESOURCE(IDS_TOOLBUTTON_OPENPL_DESC), 0);
		pdh->toolbar->InsertButton(9, ID_DOCUMENT_SAVE, 7, MAKEINTRESOURCE(IDS_TOOLBUTTON_SAVEPL), MAKEINTRESOURCE(IDS_TOOLBUTTON_SAVEPL_DESC), 0);
	}

	pdh->dropTarget = PlaylistDropTarget::RegisterWindow(hwnd, GetParent(hwnd));
	DropboxHeader_RegisterPlDropCallback(hwnd, pdh->dropTarget);

	DropboxHeader_SetDocument(hwnd, NULL);
	SendMessage(hwnd, DBM_SKINCHANGED, 0, 0);
	return 0;
}

static void DropboxHeader_OnDestroy(HWND hwnd)
{
	DROPBOXHEADER *pdh = GetHeader(hwnd);
	SetWindowLongPtr(hwnd, 0, 0L);
	if (!pdh) return;
	
	if (NULL != pdh->image.himl)
		ImageList_Destroy(pdh->image.himl);

	if (NULL != pdh->meterbar)
		delete(pdh->meterbar);

	if (NULL != pdh->toolbar)
		delete(pdh->toolbar);
		
	if(NULL != pdh->dropTarget)
		pdh->dropTarget->Release();

	if (NULL != pdh->hTooltip)
		DestroyWindow(pdh->hTooltip);

	if (NULL != pdh->hbmpMenuOverlay)
		DeleteObject(pdh->hbmpMenuOverlay);

	free(pdh);
}

static BOOL DropboxHeader_OnAgjustRect(HWND hwnd, RECT *prcProposed)
{
	if (NULL == prcProposed)
		return FALSE;

	DROPBOXHEADER *pdh = GetHeader(hwnd);
	if (NULL == pdh)
	{
		prcProposed->bottom = prcProposed->top;
		prcProposed->right = prcProposed->left;
		return TRUE;
	}

	LONG height = HEADER_DEFAULTHEIGHT;
	height = pdh->image.cy + CLIENT_OFFSET_TOP + CLIENT_OFFSET_BOTTOM;
	LONG width = HEADER_DEFAULTWIDTH;
	width = pdh->image.cx + CLIENT_OFFSET_LEFT + CLIENT_OFFSET_RIGHT;
	
	if ((prcProposed->bottom - prcProposed->top) < height) 
	{
		prcProposed->bottom = prcProposed->top;
		prcProposed->right = prcProposed->left;
	}
	else
		prcProposed->bottom = prcProposed->top + height;

	if ((prcProposed->right - prcProposed->left) < width) 
	{
		prcProposed->bottom = prcProposed->top;
		prcProposed->right = prcProposed->left;
	}
		
	return TRUE;
}

static void DropboxHeader_OnPaint(HWND hwnd)
{
	PAINTSTRUCT ps;
	if (BeginPaint(hwnd, &ps))
	{
		if (ps.rcPaint.left != ps.rcPaint.right) DropboxHeader_Paint(hwnd, &ps);
		EndPaint(hwnd, &ps);
	}
}

static void DropboxHeader_OnPrintClient(HWND hwnd, HDC hdc, UINT options)
{
	PAINTSTRUCT ps;
	ZeroMemory(&ps, sizeof(PAINTSTRUCT));
	ps.hdc = hdc;
	GetClientRect(hwnd, &ps.rcPaint);
	ps.fErase = (0 != (PRF_ERASEBKGND & options));
	DropboxHeader_Paint(hwnd, &ps);
}

static void DropboxHeader_OnWindowPosChanged(HWND hwnd, WINDOWPOS *pwp)
{
	if (SWP_NOSIZE == ((SWP_NOSIZE | SWP_FRAMECHANGED) & pwp->flags)) return;
	
	HRGN rgnInvalid = NULL;
	if (0 == (SWP_NOREDRAW & pwp->flags))
	{
		rgnInvalid = CreateRectRgn(1, 1, 1, 1);
	}
	
	DropboxHeader_UpdateLayout(hwnd, rgnInvalid);
	
	HWND hEditor = GetDlgItem(hwnd, IDC_TITLEEDITOR);
	if (NULL != hEditor)
	{
		RECT re;
		GetClientRect(hEditor, &re);
		LONG l = re.right - re.left;
		DWORD editorStyle = GetWindowStyle(hEditor);
		if (0 != (WS_VISIBLE & editorStyle))
			SetWindowLongPtr(hEditor, GWL_STYLE, editorStyle & ~WS_VISIBLE);
		
		DropboxHeader_AutoSizeTitleEditor(hwnd, rgnInvalid, 0 != (SWP_NOREDRAW & pwp->flags));

		GetClientRect(hEditor, &re);
		if ((re.right - re.left) > l)
		{
			DWORD margins = (DWORD)SendMessage(hEditor, EM_GETMARGINS, 0, 0L);
			re.left += LOWORD(margins);
			re.right -= HIWORD(margins);
			INT first = (INT)SendMessage(hEditor, EM_CHARFROMPOS, 0, MAKELPARAM(re.left + 1, re.top + 1));
			if (first > 0)
			{
				INT n = (INT)SendMessage(hEditor, EM_CHARFROMPOS, 0, 
								MAKELPARAM(re.left - ((re.right - re.left) - l) + 1, re.top + 1));
				if (n < 0) n = 0;
				
				if (n != first)
				{
					INT begin, end;
					SendMessage(hEditor, EM_GETSEL, (WPARAM)&begin, (LPARAM)&end);
					SendMessage(hEditor, EM_SETSEL, n, n);
					SendMessage(hEditor, EM_SETSEL, begin, end);
				}
			}
		}
		
		if (0 != (WS_VISIBLE & editorStyle))
		{
			SetWindowLongPtr(hEditor, GWL_STYLE, editorStyle);
			if(0 == (SWP_NOREDRAW & pwp->flags))
			{
				InvalidateRect(hEditor, NULL, TRUE);
				UpdateWindow(hEditor);
			}
		}
	}

	if (NULL != rgnInvalid)
	{
		InvalidateRgn(hwnd, rgnInvalid, FALSE);
		DeleteObject(rgnInvalid);
	}
}

static LRESULT DropboxHeader_OnGetText(HWND hwnd, LPTSTR pszBuffer, INT cchBufferMax)
{
	if (NULL == pszBuffer || cchBufferMax < 0)
		return 0;
	DROPBOXHEADER *pdh = GetHeader(hwnd);
	if (NULL == pdh) 
	{
		pszBuffer = TEXT('\0');
		return 0;
	}
	size_t remaining;
	HRESULT hr = StringCchCopyEx(pszBuffer, cchBufferMax, pdh->title.text, NULL, &remaining, STRSAFE_IGNORE_NULLS);
	if (FAILED(hr))
	{		
		pszBuffer = TEXT('\0');
		return 0;
	}
	return (cchBufferMax - remaining);
}


static LRESULT DropboxHeader_OnSetText(HWND hwnd, LPCTSTR pszText)
{
	return DropboxHeader_SetTitle(hwnd, pszText, TRUE);
}

static void DropboxHeader_OnSetFont(HWND hwnd, HFONT hFont, BOOL bRedraw)
{
	DROPBOXHEADER *pdh = GetHeader(hwnd);
	if (NULL == pdh) return;
	pdh->title.height = -1;
	pdh->title.width = -1;

	if (NULL == hFont)
		hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

	pdh->title.font = hFont;

	HWND hEditor = GetDlgItem(hwnd, IDC_TITLEEDITOR);
	if (NULL != hEditor)
	{
		SendMessage(hEditor, WM_SETFONT, (WPARAM)pdh->title.font, 0L);
		SendMessage(hEditor, EM_SETMARGINS, (WPARAM)(EC_LEFTMARGIN | EC_RIGHTMARGIN), MAKELPARAM(0, EC_RIGHTMARGIN));
	}
	
	DropboxHeader_UpdateLayout(hwnd, NULL);
	InvalidateRect(hwnd, &pdh->rcTitle, TRUE);
}


static void DropboxHeader_OnMouseMove(HWND hwnd, UINT uFlags, POINTS pts)
{
	DROPBOXHEADER *pdh = GetHeader(hwnd);
	if (NULL == pdh) return;

	POINT pt;
	POINTSTOPOINT(pt, pts);
	

	if (CheckCaptureFlag(pdh->flags, DBHF_CAPTUREMENU))
	{
		UINT imageFlags = 0x00;
		RECT rcTest;
		if (DropboxHeader_HitTestMenu(hwnd, pt, &rcTest))
		{
			imageFlags |= DBHF_MENUHOVER;
			if (MK_LBUTTON & uFlags) imageFlags |= DBHF_MENUPRESSED;
		}
		else
		{
			SetRect(&rcTest, pdh->image.x, pdh->image.y, pdh->image.x + pdh->image.cx, pdh->image.y + pdh->image.cy);
			if ((MK_LBUTTON & uFlags) && GetCapture() == hwnd)
				imageFlags |= DBHF_MENUHOVER;
		}
		

		if (imageFlags != ((DBHF_MENUHOVER | DBHF_MENUPRESSED) & pdh->flags))
		{
			pdh->flags &= ~(DBHF_MENUHOVER | DBHF_MENUPRESSED);
			pdh->flags |= imageFlags;
			
			InvalidateRect(hwnd, &rcTest, FALSE);
			
			TRACKMOUSEEVENT tm;
			tm.cbSize = sizeof(TRACKMOUSEEVENT);
			tm.dwFlags = TME_LEAVE;
			if (0 == imageFlags) tm.dwFlags |= TME_CANCEL;
			tm.hwndTrack = hwnd;
			TrackMouseEvent(&tm);
		}

	}

	if (NULL != pdh->toolbar &&
		CheckCaptureFlag(pdh->flags, DBHF_CAPTURETOOL))
	{
		pdh->toolbar->MouseMove(pt, uFlags);
	}

	if (CheckCaptureFlag(pdh->flags, DBHF_CAPTURETITLE) &&
		DropboxHeader_HitTestTitle(hwnd, pt))
	{
		if (pdh->title.width > (pdh->rcTitle.right - pdh->rcTitle.left))
			DropboxHeader_ShowTip(hwnd, pdh->title.text, &pdh->rcTitle);
	}
}

static void DropboxHeader_OnMouseLeave(HWND hwnd)
{
	DROPBOXHEADER *pdh = GetHeader(hwnd);
	if (NULL == pdh) return;

	if (0 != (DBHF_MENUHOVER & pdh->flags))
	{
		RECT rc;

		pdh->flags &= ~DBHF_MENUHOVER;
		
		SetRect(&rc, pdh->image.x, pdh->image.y, pdh->image.x + pdh->image.cx, pdh->image.y + pdh->image.cy);
		InvalidateRect(hwnd, &rc, FALSE);
	}

	if (NULL != pdh->toolbar)
		pdh->toolbar->MouseLeave();
}


static void DropboxHeader_OnLButtonDown(HWND hwnd, UINT uFlags, POINTS pts)
{
	DROPBOXHEADER *pdh = GetHeader(hwnd);
	if (NULL == pdh) return;

	RECT rcTest;
	POINT pt;
	POINTSTOPOINT(pt, pts);

	BOOL doubleClick = IsDoubleClick(pt);
	HWND hParent = GetParent(hwnd);
	
	if (CheckCaptureFlag(pdh->flags, DBHF_CAPTUREMENU) &&
		DropboxHeader_HitTestMenu(hwnd, pt, &rcTest))
	{
		if (0 == (DBHF_MENUPRESSED & pdh->flags))
		{
			pdh->flags |= DBHF_MENUPRESSED;
			InvalidateRect(hwnd, &rcTest, FALSE);
			DropboxHeader_SetCapture(hwnd, DBHF_CAPTUREMENU);
		}
		
		DropboxHeader_CloseTitleEditor(hwnd);
		return;
	}




	if (0 == ((DBHF_NODOCUMENT | DBHF_ASYNCACTIVE) & pdh->flags) &&
		CheckCaptureFlag(pdh->flags, DBHF_CAPTURETITLE) &&
		DropboxHeader_HitTestTitle(hwnd, pt))
	{
		if (doubleClick)
		{
			DWORD windowStyle = GetWindowStyle(hwnd);
			if (0 != (DBHS_ACTIVEHEADER & windowStyle))
				DropboxHeader_BeginTitleEdit(hwnd);
		}
		return;
	}

	if (0 == (DBHF_NODOCUMENT & pdh->flags) &&
		NULL != pdh->meterbar &&
		CheckCaptureFlag(pdh->flags, DBHF_CAPTUREMETER) &&
		pdh->meterbar->ButtonDown(Meterbar::MouseButtonLeft, pt, uFlags))
	{
		DropboxHeader_CloseTitleEditor(hwnd);
		return;		
	}

	
	if (NULL != pdh->toolbar &&
		CheckCaptureFlag(pdh->flags, DBHF_CAPTURETOOL) &&
		pdh->toolbar->ButtonDown(Toolbar::MouseButtonLeft, pt, uFlags))
	{
		DropboxHeader_CloseTitleEditor(hwnd);
		DropboxHeader_SetCapture(hwnd, DBHF_CAPTURETOOL);
	}

}
static void DropboxHeader_OnLButtonUp(HWND hwnd, UINT uFlags, POINTS pts)
{
	DROPBOXHEADER *pdh = GetHeader(hwnd);
	if (NULL == pdh) return;

	UINT capturedFlags = pdh->flags;

	if (GetCapture() == hwnd)
		ReleaseCapture();
	

	POINT pt;
	POINTSTOPOINT(pt, pts);
		
	RECT rcTest;

	if (CheckCaptureFlag(capturedFlags, DBHF_CAPTUREMENU) &&
		0 != (DBHF_MENUPRESSED & capturedFlags) &&
		DropboxHeader_HitTestMenu(hwnd, pt, &rcTest))
	{
		PostMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_WINDOW_CONTEXTMENU, 1), (LPARAM)hwnd);
	}

	if (NULL != pdh->toolbar &&
		CheckCaptureFlag(capturedFlags, DBHF_CAPTURETOOL))
	{
		pdh->toolbar->ButtonUp(Toolbar::MouseButtonLeft, pt, uFlags);
	}

}



static BOOL DropboxHeader_OnProcessCommand(HWND hwnd, INT commandId)
{
	switch (commandId)
	{			
		case ID_DOCUMENT_RENAME:
			if (!IsWindowVisible(hwnd))
				return FALSE; // let it go 
			DropboxHeader_BeginTitleEdit(hwnd);
			return TRUE;
		case ID_WINDOW_CONTEXTMENU:
			DropboxHeader_DisplayContextMenu(hwnd);
			return TRUE;
	}
	return FALSE;
}
static void DropboxHeader_OnCommand(HWND hwnd, INT ctrlId, INT eventId, HWND hCtrl)
{
	switch (ctrlId)
	{	
		case IDC_TITLEEDITOR:
			switch(eventId)
			{
				case EN_CHANGE:
					if (SendMessage(hCtrl, EM_GETMODIFY, 0, 0L))
					{
						HRGN rgnInvalid = CreateRectRgn(0, 0, 0, 0);
						DropboxHeader_AutoSizeTitleEditor(hwnd, rgnInvalid, FALSE);
						if (NULL != rgnInvalid)
						{							
							InvalidateRgn(hwnd, rgnInvalid, TRUE);
							DeleteObject(rgnInvalid);
							
						}
					}
					break;
			}
			return;
	}

	if (!DropboxHeader_OnProcessCommand(hwnd, ctrlId))
	{
		HWND hParent = GetParent(hwnd);
		if (NULL != hParent)
			DropboxWindow_BroadcastCommand(hParent, ctrlId, hwnd);
	}
	
}
static BOOL DropboxHeader_OnSetCursor(HWND hwnd, HWND hwndCursor, UINT hitTest, UINT uMsg)
{
	DROPBOXHEADER *pdh = GetHeader(hwnd);
	if (NULL == pdh) return FALSE;

	//if (0 != (DBHF_MENUHOVER & pdh->flags))
	//{
	//	HCURSOR hHand = (HCURSOR)LoadImage(NULL, MAKEINTRESOURCE(OCR_HAND), IMAGE_CURSOR, 
	//						0, 0, LR_DEFAULTCOLOR | LR_SHARED | LR_DEFAULTSIZE);
	//	if (NULL != hHand)
	//	{
	//		SetCursor(hHand);
	//		return TRUE;
	//	}
	//}
	return FALSE;
}

static void DropboxHeader_OnCaptureChanged(HWND hwnd, HWND hwndGainCapture)
{
	DROPBOXHEADER *pdh = GetHeader(hwnd);
	if (NULL == pdh) return;
	
	pdh->flags &= ~DBHF_CAPTUREMASK;

	POINT pt;
	GetCursorPos(&pt);
	MapWindowPoints(HWND_DESKTOP, hwnd, &pt, 1);
	
	SendMessage(hwnd, WM_MOUSEMOVE, 0, MAKELPARAM(pt.x, pt.y));
}

static LRESULT DropboxHeader_OnColorEdit(HWND hwnd, HDC hdcCtrl, HWND hwndCtrl)
{
	DROPBOXHEADER *pdh = GetHeader(hwnd);
	if (NULL == pdh) return 0;

	SetTextColor(hdcCtrl, pdh->GetThemeColor(COLOR_WINDOWTEXT));
	SetBkColor(hdcCtrl, pdh->GetThemeColor(COLOR_WINDOW));
	return (LRESULT)pdh->GetThemeBrush(COLOR_WINDOW);
}

static void DropboxHeader_OnStyleChanged(HWND hwnd, UINT nStyleType, STYLESTRUCT *pss)
{
	if (GWL_STYLE == nStyleType)
	{		
		if ((DBS_SKINWINDOW & pss->styleOld) != (DBS_SKINWINDOW & pss->styleNew))
		{
			DropboxHeader_UpdateSkinStyle(hwnd);			
		}
		if ((DBHS_ACTIVEHEADER & pss->styleOld) != (DBHS_ACTIVEHEADER& pss->styleNew))
		{
			InvalidateRect(hwnd, NULL, TRUE);	
		}
	}
}

static void DropboxHeader_OnSkinChanged(HWND hwnd)
{

	DropboxHeader_LoadImage(hwnd, plugin.hDllInstance, MAKEINTRESOURCE(IDR_DEFAULTPLAYLIST_IMAGE));
	InvalidateRect(hwnd, NULL, TRUE);
	
	DROPBOXHEADER *pdh = GetHeader(hwnd);
	if (NULL == pdh) return;

	if (NULL != pdh->toolbar)
	{
		pdh->toolbar->SetBkBrush(pdh->GetThemeBrush(COLOR_BTNFACE));
		pdh->toolbar->SetTextBkColor(pdh->GetThemeColor(COLOR_BTNFACE));
		pdh->toolbar->SetTextColor(pdh->GetThemeColor(COLOR_BTNTEXT));
		DropboxHeader_SetToolbarImagelist(pdh->toolbar);
	}

}

static void CALLBACK DropboxHeader_OnDocumentEvent(Document *pDocument, UINT eventId, LONG_PTR param, UINT_PTR user)
{
	DROPBOXHEADER *pdh = GetHeader((HWND)user);
	if (NULL == pdh) return;

	switch(eventId)
	{
		case Document::EventTitleChanged:
			{
				TCHAR szTitle[512];
				if (FAILED(pDocument->GetTitle(szTitle, ARRAYSIZE(szTitle))))
					szTitle[0] = TEXT('\0');
				DropboxHeader_SetTitle((HWND)user, szTitle, TRUE);
			}
			break;
		case Document::EventCountChanged:
			if (NULL != pdh->meterbar)
				pdh->meterbar->InvalidateMetrics(Meterbar::MetricsDocument);
			break;
		case Document::EventItemReadCompleted:
			if (NULL != pdh->meterbar)
				pdh->meterbar->InvalidateMetrics(Meterbar::MetricsDocument);
			break;
		case Document::EventAsyncStarted:
			DropboxHeader_AsyncActivated((HWND)user, TRUE);
			break;
		case Document::EventAsyncFinished:
			DropboxHeader_AsyncActivated((HWND)user, FALSE);
			break;

	}
}

static void CALLBACK DropboxHeader_OnViewEvent(DropboxView *pView, UINT eventId, LONG_PTR param, UINT_PTR user)
{
	DROPBOXHEADER *pdh = GetHeader((HWND)user);
	if (NULL == pdh) return;
	
	switch(eventId)
	{
		case DropboxView::EventSelectionChanged:
			if (NULL != pdh->meterbar)
				pdh->meterbar->InvalidateMetrics(Meterbar::MetricsSelection);
			break;

		case DropboxView::EventSelectionLengthChanged:
			if (NULL != pdh->meterbar)
				pdh->meterbar->InvalidateMetrics(Meterbar::MetricsSelection);
			break;
		
	}

}
static void DropboxHeader_OnSetDocument(HWND hwnd, Document *pDocument)
{
	DROPBOXHEADER *pdh = GetHeader(hwnd);
	if (NULL == pdh) return;

	TCHAR szTitle[512];

	if (NULL == pDocument)
	{
		pdh->flags |= DBHF_NODOCUMENT;
		szTitle[0] = TEXT('\0');
		if (NULL != pdh->meterbar)
			pdh->meterbar->SetFlags(0, Meterbar::FlagVisible);
		if (NULL != pdh->toolbar)
			pdh->toolbar->SetFlags(0, Toolbar::FlagVisible);
	}
	else 
	{
		pdh->flags &= ~DBHF_NODOCUMENT;
		if (FAILED(pDocument->GetTitle(szTitle, ARRAYSIZE(szTitle))))
			szTitle[0] = TEXT('\0');
		if (NULL != pdh->meterbar)
			pdh->meterbar->SetFlags(Meterbar::FlagVisible, Meterbar::FlagVisible);
		if (NULL != pdh->toolbar)
			pdh->toolbar->SetFlags(Toolbar::FlagVisible, Toolbar::FlagVisible);
	}
	
	DropboxHeader_SetTitle(hwnd, szTitle, FALSE);
	

	if (NULL != pdh->meterbar)
	{
		pdh->meterbar->InvalidateMetrics(Meterbar::MetricsDocument | Meterbar::MetricsSelection);
		pdh->meterbar->UpdateMetrics();
	}

	BOOL asyncActive = FALSE;
	if (NULL != pDocument)
		asyncActive = pDocument->QueryAsyncOpInfo(NULL);
	DropboxHeader_AsyncActivated(hwnd, asyncActive);

	if (NULL != pDocument)
		pDocument->RegisterCallback(DropboxHeader_OnDocumentEvent, (ULONG_PTR)hwnd);

	HRGN rgnInvalidate = CreateRectRgnIndirect(&pdh->rcTitle);
	DropboxHeader_UpdateLayout(hwnd, rgnInvalidate);
	InvalidateRgn(hwnd, rgnInvalidate, TRUE);
	DeleteObject(rgnInvalidate);
}

static void DropboxHeader_OnSetView(HWND hwnd, DropboxView *pView)
{

	if (NULL != pView)
	{
		pView->RegisterCallback(DropboxHeader_OnViewEvent, (ULONG_PTR)hwnd);
	}
	
	DROPBOXHEADER *pdh = GetHeader(hwnd);
	if (NULL != pdh  && NULL != pdh->meterbar)
	{
		pdh->meterbar->InvalidateMetrics(Meterbar::MetricsSelection);
		pdh->meterbar->UpdateMetrics();
	}


}

static void DropboxHeader_OnMediaLibraryDragDrop(HWND hwnd, INT code, mlDropItemStruct *pdis)
{
	DROPBOXHEADER *pdh = GetHeader(hwnd);
	if (NULL == pdh || NULL == pdh->dropTarget) 
		return;

	pdh->dropTarget->MediaLibraryDragDrop(code, pdis);

}
static void DropboxHeader_OnShowTip(HWND hwnd, LPCTSTR pszTipText, const RECT *prcBounds)
{
	DROPBOXHEADER *pdh = GetHeader(hwnd);
	if (NULL == pdh) return;

	TOOLINFO ti;
	ZeroMemory(&ti, sizeof(TOOLINFO));
	ti.cbSize = sizeof(TOOLINFO);
	ti.hwnd = hwnd;
	ti.hinst = plugin.hDllInstance;
	CopyRect(&ti.rect, prcBounds);
	ti.lpszText = (LPWSTR)pszTipText;
	ti.uId = 0;

	if (NULL == pdh->hTooltip)
	{
		INITCOMMONCONTROLSEX iccex; 
		iccex.dwICC = ICC_WIN95_CLASSES;
		iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
		InitCommonControlsEx(&iccex);

		pdh->hTooltip = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
									WS_POPUP,
									CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
									NULL, NULL, plugin.hDllInstance, NULL);
		if (NULL == pdh->hTooltip)
			return;
		
		SetWindowPos(pdh->hTooltip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		
		DWORD windowStyle = GetWindowStyle(hwnd);
		if (0 != (DBS_SKINWINDOW & windowStyle))
		{
			MlSkinWindow(pdh->hTooltip, SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT);
		}

		SendMessage(pdh->hTooltip, TTM_ADDTOOL, 0, (LPARAM)&ti);
		SendMessage(pdh->hTooltip, TTM_SETDELAYTIME, TTDT_INITIAL, MAKELPARAM(1000, 0)); 
		SendMessage(pdh->hTooltip, TTM_SETDELAYTIME, TTDT_RESHOW, MAKELPARAM(-2, 0)); 
		
		OSVERSIONINFO ov;
		ov.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		if (::GetVersionEx(&ov) && 
			ov.dwMajorVersion >= 6 && 
			VER_PLATFORM_WIN32_NT == ov.dwPlatformId)
		{
			RECT rcMargin;
			SetRect(&rcMargin, 3, 1, 3, 1);
			SendMessage(pdh->hTooltip, TTM_SETMARGIN, 0, (LPARAM)&rcMargin); 
		}
	}
	else
	{        
		TOOLINFO to;
		TCHAR szBuffer[512];

		to.cbSize = sizeof(TOOLINFO);
		to.hwnd = hwnd;
		to.uId = 0;
		to.lpszText = szBuffer;
		if (!SendMessage(pdh->hTooltip, TTM_GETTOOLINFO, 0, (LPARAM)&to))
			ZeroMemory(&to, sizeof(TOOLINFO));

		BOOL deactivated = FALSE;
		if (!EqualRect(&to.rect, &ti.rect))
		{
			if (FALSE == deactivated)
			{
				SendMessage(pdh->hTooltip, TTM_ACTIVATE, FALSE, 0);
				deactivated = TRUE;
			}

			SendMessage(pdh->hTooltip, TTM_NEWTOOLRECT, 0, (LPARAM)&ti);
		}

		if (CSTR_EQUAL != CompareString(LOCALE_USER_DEFAULT, 0, to.lpszText, -1, ti.lpszText, -1))
		{
			if (FALSE == deactivated)
			{
				SendMessage(pdh->hTooltip, TTM_ACTIVATE, FALSE, 0);
				deactivated = TRUE;
			}
			SendMessage(pdh->hTooltip, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);
		}

		if (deactivated)
			SendMessage(pdh->hTooltip, TTM_ACTIVATE, TRUE, 0);
	}
}

static void DropboxHeader_OnPopTip(HWND hwnd)
{
	DROPBOXHEADER *pdh = GetHeader(hwnd);
	if (NULL == pdh || NULL == pdh->hTooltip) return;

	SendMessage(pdh->hTooltip, TTM_POP, 0, 0);
}

static BOOL DropboxHeader_OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT *pmis)
{
	DROPBOXHEADER *pdh = GetHeader(hwnd);
	if (NULL == pdh) return FALSE;

	if (NULL != pdh->toolbar && pdh->toolbar->RelayMeasureItem(pmis))
		return TRUE;
	
	return FALSE;
}

static BOOL DropboxHeader_OnDrawItem(HWND hwnd, DRAWITEMSTRUCT *pdis)
{
	DROPBOXHEADER *pdh = GetHeader(hwnd);
	if (NULL == pdh) return FALSE;

	if (NULL != pdh->toolbar && pdh->toolbar->RelayDrawItem(pdis))
		return TRUE;

	return FALSE;
}

static void DropboxHeader_OnUpdateMetrics(HWND hwnd)
{
	DROPBOXHEADER *pdh = GetHeader(hwnd);
	if (NULL != pdh && NULL != pdh->meterbar) 
		pdh->meterbar->UpdateMetrics();
}


static BOOL DropboxHeader_OnGetPartRect(HWND hwnd, UINT partId, RECT *prcPart)
{
	if (NULL == prcPart)
		return FALSE;

	DROPBOXHEADER *pdh = GetHeader(hwnd);
	if (NULL != pdh) 
	{
		switch(partId)
		{
			case DBHP_MENU:		
				return SetRect(prcPart, pdh->image.x, pdh->image.y, pdh->image.x + pdh->image.cx,  pdh->image.y + pdh->image.cy);
 			case DBHP_TITLE:	
				return CopyRect(prcPart, &pdh->rcTitle);
			case DBHP_METERBAR:	
				if (NULL != pdh->meterbar)
					return pdh->meterbar->GetBounds(prcPart);
				break;
			case DBHP_TOOLBAR:
				if (NULL != pdh->toolbar)
					return pdh->toolbar->GetBounds(prcPart);
				break;
		}
	}
	SetRectEmpty(prcPart);
	return FALSE;
}

static UINT DropboxHeader_OnHitTest(HWND hwnd, POINTS  pts)
{
	POINT pt;
	POINTSTOPOINT(pt, pts);

	RECT rc;
	if (!GetClientRect(hwnd, &rc) || !PtInRect(&rc, pt))
		return DBHP_ERROR;

	DROPBOXHEADER *pdh = GetHeader(hwnd);
	if (NULL != pdh)
	{
		if (DropboxHeader_HitTestMenu(hwnd, pt, NULL))
			return DBHP_MENU;

		if (DropboxHeader_HitTestTitle(hwnd, pt))
			return DBHP_TITLE;
				
		if (NULL != pdh->meterbar && pdh->meterbar->HitTest(pt))
			return DBHP_METERBAR;

		if (NULL != pdh->toolbar && pdh->toolbar->HitTest(pt, NULL, NULL))
			return DBHP_TOOLBAR;
	}
	return DBHP_CLIENT;
}

static BOOL DropboxHeader_OnSetMenuOverlay(HWND hwnd, OVERLAYINFO *overlayInfo)
{
	DROPBOXHEADER *pdh = GetHeader(hwnd);
	if (NULL == pdh)
		return FALSE;

	if (NULL != pdh->hbmpMenuOverlay)
	{
		DeleteObject(pdh->hbmpMenuOverlay);
		pdh->hbmpMenuOverlay = NULL;
	}
	
	if (pdh->image.cx <= 0 || pdh->image.cy <= 0)
		return FALSE;

	BITMAPINFOHEADER compositionHeader;
	ZeroMemory(&compositionHeader, sizeof(BITMAPINFOHEADER));

	compositionHeader.biSize = sizeof(BITMAPINFOHEADER);
	compositionHeader.biCompression = BI_RGB;
	compositionHeader.biBitCount = 24;
	compositionHeader.biPlanes = 1;
	compositionHeader.biWidth = pdh->image.cx;
	compositionHeader.biHeight = -pdh->image.cy;
	
	BYTE *compositionPixels;
	pdh->hbmpMenuOverlay = CreateDIBSection(NULL, (LPBITMAPINFO)&compositionHeader, DIB_RGB_COLORS, (void**)&compositionPixels, NULL, 0);
	if (NULL == pdh->hbmpMenuOverlay)
		return FALSE;
	
	HDC compositionDC = CreateCompatibleDC(NULL);
	if (NULL != compositionDC)
	{
		COLORREF rgbBk = pdh->GetThemeColor(COLOR_BTNFACE);
		COLORREF rgbFg = pdh->GetThemeColor(COLOR_BTNTEXT);

		HBITMAP hbmpOld = (HBITMAP)SelectObject(compositionDC, pdh->hbmpMenuOverlay);
		INT oldX = pdh->image.x, oldY = pdh->image.y;
		
		pdh->image.x = 0;
		pdh->image.y = 0;
		pdh->image.i = 0;
		pdh->image.hdcDst = compositionDC;
		pdh->image.dwRop = SRCCOPY;
		if (!ImageList_DrawIndirect(&pdh->image))
		{
			RECT rcFill;
			SetRect(&rcFill, 0, 0, pdh->image.cx, pdh->image.cy);
			COLORREF rgbOld = SetBkColor(compositionDC, rgbBk);
			ExtTextOut(compositionDC, 0,0, ETO_OPAQUE, &rcFill, NULL, 0, NULL);
			SetBkColor(compositionDC, rgbOld);
		}

		pdh->image.x = oldX;
		pdh->image.y = oldY;

		
		BoxBlur(compositionPixels,pdh->image.cx, pdh->image.cy, compositionHeader.biBitCount, 2, TRUE, NULL);

		COLORREF rgbFade = pdh->GetThemeColor(COLOR_BTNFACE);
		rgbFade = ((200 << 24) | (0x00FFFFFF & rgbFade));
		ColorOverImageEx(compositionPixels, pdh->image.cx, pdh->image.cy, 0,0, pdh->image.cx, pdh->image.cy, compositionHeader.biBitCount, FALSE, rgbFade);

		BITMAPINFOHEADER overlayHeader;
		BYTE *overlayPixels;
		HBITMAP hOverlay = ImageLoader_LoadPngEx(overlayInfo->hInstance, overlayInfo->pszImage, &overlayHeader, (void**)&overlayPixels);
		if (NULL != hOverlay)
		{
			INT destX, destY, destWidth, destHeight;

			INT kx, ky;
			kx = (overlayHeader.biWidth > pdh->image.cx) ? (pdh->image.cx * 10000 / overlayHeader.biWidth) : 10000;
			ky = (abs(overlayHeader.biHeight) > pdh->image.cy) ? (pdh->image.cy * 10000 / abs(overlayHeader.biHeight)) : 10000;

			if (kx != ky)
			{
				if (kx < ky) ky = kx;
				else kx = ky;
				
			}

			destWidth = overlayHeader.biWidth * kx / 10000;
			destHeight = abs(overlayHeader.biHeight) * ky / 10000;
			destX = (pdh->image.cx - destWidth) / 2;
			destY = (pdh->image.cy - destHeight) / 2;
			
			ColorizeImageEx(overlayPixels, overlayHeader.biWidth, abs(overlayHeader.biHeight), overlayHeader.biBitCount, rgbBk, rgbFg, FALSE);

			if (1) // alpha blend
			{ 
				HDC hdcSrc = CreateCompatibleDC(compositionDC);
				if (NULL != hdcSrc)
				{
					HBITMAP hbmpSrcOld = (HBITMAP)SelectObject(hdcSrc, hOverlay);
					BLENDFUNCTION  bf;
					bf.BlendOp = AC_SRC_OVER;
					bf.BlendFlags = 0;
					bf.AlphaFormat = AC_SRC_ALPHA;
					bf.SourceConstantAlpha = 205;
					
					PremultiplyImage(overlayPixels, overlayHeader.biWidth, abs(overlayHeader.biHeight));
					BOOL result = AlphaBlend(compositionDC, destX, destY, destWidth, destHeight, hdcSrc, 0, 0, overlayHeader.biWidth, abs(overlayHeader.biHeight), bf);

					SelectObject(hdcSrc, hbmpSrcOld);
					DeleteDC(hdcSrc);
				}
			
			}
			else
			{
				StretchDIBits(compositionDC, destX, destY, destWidth, destHeight, 0, 0, overlayHeader.biWidth, abs(overlayHeader.biHeight), 
						overlayPixels, (BITMAPINFO*)&overlayHeader, DIB_RGB_COLORS, SRCCOPY);
			}

			DeleteObject(hOverlay);
		}

		SelectObject(compositionDC, hbmpOld);
		DeleteDC(compositionDC);
	}
	
	RECT rcImage;
	SetRect(&rcImage, pdh->image.x, pdh->image.y, pdh->image.x + pdh->image.cx,  pdh->image.y + pdh->image.cy);
	InvalidateRect(hwnd, &rcImage, FALSE);
	return TRUE;
}

static void DropboxHeader_OnRemoveMenuOverlay(HWND hwnd)
{
	DROPBOXHEADER *pdh = GetHeader(hwnd);
	if (NULL != pdh && NULL != pdh->hbmpMenuOverlay)
	{
		DeleteObject(pdh->hbmpMenuOverlay);
		pdh->hbmpMenuOverlay = NULL;
		
		RECT rcImage;
		SetRect(&rcImage, pdh->image.x, pdh->image.y, pdh->image.x + pdh->image.cx,  pdh->image.y + pdh->image.cy);
		InvalidateRect(hwnd, &rcImage, FALSE);
	}
}

static void DropboxHeader_OnProfileChanged(HWND hwnd)
{
	DROPBOXHEADER *pdh = GetHeader(hwnd);
	if (NULL == pdh) return;

	HWND hParent = GetParent(hwnd);
	Profile *profile = DropboxWindow_GetProfile(hParent);

	if (NULL != pdh->meterbar)
		delete(pdh->meterbar);
	
	pdh->meterbar = (NULL != profile) ? Meterbar::Load(profile) : NULL;
	if (NULL == pdh->meterbar)
		pdh->meterbar = new Meterbar(Meterbar::FlagVisible | Meterbar::FlagUnitCount);
	
	if (NULL != pdh->meterbar)
		DropboxHeader_RegisterMeterbarCallback(hwnd, pdh->meterbar);
	
	InvalidateRect(hwnd, NULL, TRUE);
}

static BOOL DropboxHeader_OnSave(HWND hwnd, Profile *profile)
{
	DROPBOXHEADER *pdh = GetHeader(hwnd);
	if (NULL == pdh || NULL == profile) return FALSE;

	if (NULL != pdh->meterbar)
		pdh->meterbar->Save(profile);
	
	return TRUE;
}

static LRESULT CALLBACK DropboxHeader_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_CREATE:				return DropboxHeader_OnCreateWindow(hwnd, (CREATESTRUCT*)lParam);
		case WM_DESTROY:				DropboxHeader_OnDestroy(hwnd); return 0;
		case WM_PAINT:				DropboxHeader_OnPaint(hwnd); return 0;
		case WM_PRINTCLIENT:			DropboxHeader_OnPrintClient(hwnd, (HDC)wParam, (UINT)lParam); return 0;
		case WM_WINDOWPOSCHANGED:	DropboxHeader_OnWindowPosChanged(hwnd, (WINDOWPOS*)lParam); return 0;
		case WM_SETTEXT:				return DropboxHeader_OnSetText(hwnd, (LPCTSTR)lParam); 
		case WM_GETTEXT:				return DropboxHeader_OnGetText(hwnd, (LPTSTR)lParam, (INT)wParam); 
		case WM_SETFONT:				DropboxHeader_OnSetFont(hwnd, (HFONT)wParam, (BOOL)LOWORD(lParam)); break;	
		case WM_MOUSEMOVE:			
			DropboxHeader_OnMouseMove(hwnd, (UINT)wParam, MAKEPOINTS(lParam));  
			DropboxHeader_RelayTooltipEvent(hwnd, uMsg, wParam, lParam);
			return 0;
		case WM_LBUTTONDOWN:			
			DropboxHeader_OnLButtonDown(hwnd, (UINT)wParam, MAKEPOINTS(lParam)); 
			DropboxHeader_RelayTooltipEvent(hwnd, uMsg, wParam, lParam);
			return 0;
		case WM_LBUTTONUP:			
			DropboxHeader_OnLButtonUp(hwnd, (UINT)wParam, MAKEPOINTS(lParam)); 
			DropboxHeader_RelayTooltipEvent(hwnd, uMsg, wParam, lParam);
			return 0;
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		//	DropboxHeader_RelayTooltipEvent(hwnd, uMsg, wParam, lParam);
			return 0;
		case WM_MOUSELEAVE:			DropboxHeader_OnMouseLeave(hwnd); return 0;
		case WM_COMMAND:				DropboxHeader_OnCommand(hwnd, LOWORD(wParam), HIWORD(wParam),  (HWND)lParam); return 0;
		case WM_SETCURSOR:			if (DropboxHeader_OnSetCursor(hwnd, (HWND)wParam, LOWORD(lParam), HIWORD(lParam))) return TRUE; break;
		case WM_CAPTURECHANGED:		DropboxHeader_OnCaptureChanged(hwnd, (HWND)lParam); return 0;
		case WM_CTLCOLOREDIT:		return DropboxHeader_OnColorEdit(hwnd, (HDC)wParam, (HWND)lParam); 
		case WM_STYLECHANGED:		DropboxHeader_OnStyleChanged(hwnd, (UINT)wParam, (STYLESTRUCT*)lParam); break;
		case WM_ERASEBKGND:			return 0;
		case WM_MEASUREITEM:			return DropboxHeader_OnMeasureItem(hwnd, (MEASUREITEMSTRUCT*)lParam);
		case WM_DRAWITEM:			return DropboxHeader_OnDrawItem(hwnd, (DRAWITEMSTRUCT*)lParam);
		case DBM_ADJUSTRECT:			return DropboxHeader_OnAgjustRect(hwnd, (RECT*)lParam);
		case DBM_SKINCHANGED:		DropboxHeader_OnSkinChanged(hwnd); return 0;
		case DBHM_SETDOCUMENT:		DropboxHeader_OnSetDocument(hwnd, (Document*)lParam); return 0;
		case DBHM_SETVIEW:			DropboxHeader_OnSetView(hwnd, (DropboxView*)lParam); return 0;
		case DBHM_SHOWTIP:			DropboxHeader_OnShowTip(hwnd, (LPCTSTR)wParam, (const RECT*)lParam); return 0;
		case DBHM_POPTIP:			DropboxHeader_OnPopTip(hwnd); return 0;
		case DBHM_PROCESSCOMMAND:	return DropboxHeader_OnProcessCommand(hwnd, (INT)wParam);
		case DBHM_UPDATEMETRICS:		DropboxHeader_OnUpdateMetrics(hwnd); return 0;
		case DBHM_GETPARTRECT:		return DropboxHeader_OnGetPartRect(hwnd, (UINT)wParam, (RECT*)lParam); 
		case DBHM_HITTEST:			return DropboxHeader_OnHitTest(hwnd, MAKEPOINTS(lParam));
		case DBHM_SETMENUOVERLAY:	return (LRESULT)DropboxHeader_OnSetMenuOverlay(hwnd, (OVERLAYINFO*)lParam);
		case DBHM_REMOVEMENUOVERLAY: DropboxHeader_OnRemoveMenuOverlay(hwnd); return 0;
		case DBHM_PROFILECHANGED:	DropboxHeader_OnProfileChanged(hwnd); return 0;
		case DBHM_SAVE:				return DropboxHeader_OnSave(hwnd, (Profile*)lParam); 

	}

	if (WAML_NOTIFY_DRAGDROP == uMsg && 0 != WAML_NOTIFY_DRAGDROP)
	{
		DropboxHeader_OnMediaLibraryDragDrop(hwnd, (INT)wParam, (mlDropItemStruct*)lParam); 
		return TRUE;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

