#include "./main.h"
#include "./simpleView.h"
#include "./plugin.h"
#include "./wasabiApi.h"
#include "./fileInfoInterface.h"
#include "./configIniSection.h"
#include "./configManager.h"
#include "./resource.h"
#include "./cfpInterface.h"
#include "./listSelectHook.h"
#include "./skinWindow.h"
#include "./document.h"
#include "./guiObjects.h"


#include <shlobj.h>
#include <strsafe.h>

#define COLUMN_OFFSET_X			0
#define DISABLEDTEXT_ALPHA		180

typedef enum __SIMPLEVIEWFLAGS
{
	INDEXCOLUMN_INVALIDWIDTH = 0x00000001,
	WINDOW_HASFOCUS = 0x00000002,
	MULTISELECT_STARTED = 0x00000004,
	WINDOW_EMPTY = 0x00000008,
} SIMPLEVIEWFLAGS;

SimpleView::SimpleView(HWND hView) 
	: BaseListView(hView), flags(0), prevWidth(-1), wpcReentryFilter(FALSE), itemHeight(0)
{	

	HWND hParent = GetParent(hwnd);
	Profile *profile = (NULL != hParent) ? DropboxWindow_GetProfile(hParent) : NULL;
    if (NULL != profile)
		Load(profile);

	DWORD styleEx = LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT;
	SendMessage(hwnd, LVM_SETEXTENDEDLISTVIEWSTYLE, styleEx, styleEx);
	SendMessage(hwnd, LVM_SETUNICODEFORMAT, (WPARAM)TRUE, 0L);

	RECT rc;
	if (GetClientRect(hwnd, &rc))
	{
		LVCOLUMN lvc;
		lvc.mask = LVCF_WIDTH | LVCF_TEXT;
		lvc.cx = 40;
		lvc.pszText = TEXT("Data");
		SendMessage(hwnd, LVM_INSERTCOLUMN, 0, (LPARAM)&lvc);
	}

	metaKeyList.push_back(METAKEY_FORMATTEDTITLE);
	METAKEY szMetaKeys[12];
	 
	painter.SetStyle(0, PAINTER_OPAQUE);

	INT columnId = (INT)(SHORT)painter.GetStyle(PAINTER_RIGHTCOLUMNMASK);
	INT metaCount = ColumnIdToMetaKey(columnId, szMetaKeys, ARRAYSIZE(szMetaKeys));
	for (int i = 0; i < metaCount; i++)
		metaKeyList.push_back(szMetaKeys[i]);
}


SimpleView::~SimpleView()
{	
	//SaveSettings();
}


STDMETHODIMP SimpleView::SetSkinned(BOOL bSkinned)
{
	if (bSkinned == skinned)
		return S_OK;

	HRESULT hr;
	if (bSkinned)
	{
		hr = MlSkinWindowEx(hwnd, SKINNEDWND_TYPE_SCROLLWND, SWS_USESKINCOLORS | SWS_USESKINCURSORS);
		if (SUCCEEDED(hr))
		{
			MLSkinnedScrollWnd_SetMode(hwnd, SCROLLMODE_LISTVIEW);
			MLSkinnedScrollWnd_ShowHorzBar(hwnd, FALSE);
			MLSkinnedWnd_EnableReflection(hwnd, TRUE);
			skinned = TRUE;
		}
	}
	else
	{
		hr = MlUnskinWindow(hwnd);
		if (SUCCEEDED(hr))
			skinned = FALSE;
	}
	OnSkinChanged();
	return hr;
}


void SimpleView::OnWindowPosChanged(WINDOWPOS *pwp)
{	
	if (wpcReentryFilter)
		return;

	BOOL bProcessed = FALSE;

	DWORD windowStyle = GetWindowStyle(hwnd); // do not remove this line - it does magic!!!

	if (SWP_NOSIZE != ((SWP_NOSIZE & SWP_FRAMECHANGED) & pwp->flags)) 
	{		
		RECT rc;
		GetClientRect(hwnd, &rc);
		INT iTop, iLast;
		iLast = (INT)CallPrevWndProc(LVM_GETITEMCOUNT, 0, 0L)  - 1;
		iTop = (INT)CallPrevWndProc(LVM_GETTOPINDEX, 0, 0L);
		
		if (prevWidth != (rc.right - rc.left) || (iLast > 0 && iTop > 0))
		{
			prevWidth = rc.right - rc.left;
			
			wpcReentryFilter = TRUE;

			if (0 != (WS_VISIBLE & windowStyle))
				SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle & ~WS_VISIBLE);
	
			HWND hHeader = (HWND)CallPrevWndProc(LVM_GETHEADER, 0, 0L);
			if (NULL != hHeader)
			{
				HDITEM item;
				ZeroMemory(&item, sizeof(HDITEM));
				item.mask = HDI_WIDTH;
				item.cxy = prevWidth - COLUMN_OFFSET_X;
						
				SNDMSG(hHeader, HDM_SETITEM, 0, (LPARAM)&item);
			}	
					
			BaseListView::OnWindowPosChanged(pwp);

			if (iLast > 0 && iTop > 0)
			{
				RECT ri;
				ri.left = LVIR_BOUNDS;
				if (CallPrevWndProc(LVM_GETITEMRECT, (WPARAM)iLast, (LPARAM)&ri) &&
					(ri.bottom + (ri.bottom - ri.top)) < (rc.bottom - 2))
				{				
					SendMessage(hwnd, WM_VSCROLL, MAKEWPARAM(SB_LINEUP, 0), NULL);
					SendMessage(hwnd, WM_VSCROLL, MAKEWPARAM(SB_ENDSCROLL, 0), NULL);
				}
			}
		
			if (0 != (WS_VISIBLE & windowStyle))
			{
				SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle);
				if (0 == (SWP_NOREDRAW & pwp->flags))
					InvalidateRect(hwnd, NULL, TRUE);		
				if (0 != (SWP_FRAMECHANGED & pwp->flags))
					RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_FRAME);
			}
			
			wpcReentryFilter = FALSE;
			bProcessed = TRUE;
			
		}
		
		if (0 != (WINDOW_EMPTY & flags) && 0 == (SWP_NOREDRAW & pwp->flags))
			InvalidateRect(hwnd, NULL, TRUE);
		
	}
	
	if (!bProcessed)
		BaseListView::OnWindowPosChanged(pwp);
}



LRESULT SimpleView::OnPrePaint(NMLVCUSTOMDRAW *pcd)
{
	if (INDEXCOLUMN_INVALIDWIDTH & flags)
	{
		UpdateIndexColumnWidth();
		flags &= ~INDEXCOLUMN_INVALIDWIDTH;
	}
	
	DWORD windowStyle = GetWindowStyle(hwnd);
	
	COLORREF rgbBk, rgbFg;
	rgbFg = GetThemeColor(COLOR_WINDOWTEXT);
	rgbBk = GetThemeColor(COLOR_WINDOW);

	if (0 != (WS_DISABLED & windowStyle))
		rgbFg = BlendColors(rgbFg, rgbBk, DISABLEDTEXT_ALPHA);

	SetBkColor(pcd->nmcd.hdc, rgbBk);
	SetTextColor(pcd->nmcd.hdc, rgbFg);


	if (0 != (WS_DISABLED & windowStyle) && !IsRectEmpty(&pcd->nmcd.rc))
	{
		ExtTextOut(pcd->nmcd.hdc, 0, 0, ETO_OPAQUE, &pcd->nmcd.rc, NULL, 0, NULL);
	}

	return CDRF_DODEFAULT;
}
LRESULT SimpleView::OnListViewCustomDraw(NMLVCUSTOMDRAW *pcd)
{
	switch(pcd->nmcd.dwDrawStage)
	{
		case CDDS_PREPAINT:
			return  OnPrePaint(pcd);
	}
	return CDRF_DODEFAULT;
}
STDMETHODIMP SimpleView::ProcessNotification(NMHDR *pnmh, LRESULT *pResult)
{
	switch(pnmh->code)
	{
		case LVN_MARQUEEBEGIN:	*pResult = TRUE; return S_OK; // block bounding box selection
		case NM_CUSTOMDRAW:		*pResult = OnListViewCustomDraw((NMLVCUSTOMDRAW*)pnmh); return S_OK;
	}
	return BaseListView::ProcessNotification(pnmh, pResult);
}

static void DrawItemFocusRect(DRAWITEMSTRUCT *pdis)
{
	if (0 != (0x0200/*ODS_NOFOCUSRECT*/ & pdis->itemState))
		return;
	
	COLORREF rgbTextOld = SetTextColor(pdis->hDC, 0x000000);
	COLORREF rgbBkOld = SetBkColor(pdis->hDC, 0xFFFFFF);
	DrawFocusRect(pdis->hDC, &pdis->rcItem);
	SetTextColor(pdis->hDC, rgbTextOld);
	SetBkColor(pdis->hDC, rgbBkOld);
}

STDMETHODIMP SimpleView::DrawItem(DRAWITEMSTRUCT *pdis)
{	
	HWND hParent = GetParent(hwnd);
	
	/*if (NULL != pSpacer)
		pSpacer->AdjustItemRect(&pdis->rcItem);*/

	pdis->rcItem.left += COLUMN_OFFSET_X;

	IFileInfo *pFileInfo = NULL;

	if (NULL != pActiveDocument &&
		((size_t)pdis->itemID) < pActiveDocument->GetItemCount())
	{
		pFileInfo = pActiveDocument->GetItemDirect(pdis->itemID);
	}
	
		
	COLORREF rgbBkOld, rgbFgOld;

	if (0 != (ODS_SELECTED & pdis->itemState))
	{
		COLORREF rgbBk, rgbFg;
		rgbFg = GetThemeColor(COLOR_HIGHLIGHTTEXT);
		rgbBk = GetThemeColor(COLOR_HIGHLIGHT);

		if (0 != (WS_DISABLED & GetWindowLongPtr(hwnd, GWL_STYLE)))
			rgbFg = BlendColors(rgbFg, rgbBk, DISABLEDTEXT_ALPHA);

		rgbBkOld = SetBkColor(pdis->hDC, rgbBk);
		rgbFgOld = SetTextColor(pdis->hDC, rgbFg);
	}


	if (0 != (ODS_SELECTED & pdis->itemState) || NULL == pFileInfo)
		ExtTextOut(pdis->hDC, 0, 0, ETO_OPAQUE, &pdis->rcItem, NULL, 0, NULL);
	
	if (NULL != pFileInfo)
	{
		painter.Paint(pdis->hDC, pFileInfo, pdis->itemID + 1, pdis->itemState, &pdis->rcItem);
	}
	
	if (0 != (WINDOW_HASFOCUS & flags) && 
		0 != (ODS_FOCUS & pdis->itemState) &&
		0 == (UISF_HIDEFOCUS & uiState))
	{	
		DrawItemFocusRect(pdis);
	}

	if (0 != (ODS_SELECTED & pdis->itemState))
	{
		SetBkColor(pdis->hDC, rgbBkOld);
		SetTextColor(pdis->hDC, rgbFgOld);
	}

	return S_OK;
}

STDMETHODIMP SimpleView::MeasureItem(MEASUREITEMSTRUCT *pmis)
{	
	BOOL br = painter.MeasureLineHeight(hwnd, &itemHeight);
	pmis->itemHeight = itemHeight;
	return (br) ? S_OK : E_FAIL;
}

STDMETHODIMP_(DropboxViewMeta*) SimpleView::GetMeta()
{
	return simpleViewMeta;
}

STDMETHODIMP SimpleView::ConfigChanged(void)
{
	HWND hParent = GetParent(hwnd);
	Profile *profile = (NULL != hParent) ? DropboxWindow_GetProfile(hParent) : NULL;
    if (NULL != profile)
	{
		Load(profile);
		InvalidateRect(hwnd, NULL, TRUE);
	}
	return S_OK;
}
void SimpleView::UpdateIndexColumnWidth()
{
	INT itemsCount = (INT)CallPrevWndProc(LVM_GETITEMCOUNT, 0, 0L);
	TCHAR szText[64];
	INT cchText;
	for (cchText = 0; itemsCount > 0; cchText++) 
	{
		itemsCount= itemsCount/10; 
		szText[cchText] = TEXT('9');
	}
	szText[cchText++] = TEXT('.');
	szText[cchText] = TEXT('\0');
	
	SIZE size;
	if (!ItemPainter::MeasureString(hwnd, szText, cchText, &size))
		size.cx = 0;

	painter.SetColumnWidth(PAINTCOL_INDEX, size.cx);
}

void SimpleView::OnLButtonDown(UINT uFlags, POINTS pts)
{

	LVHITTESTINFO ht;
	POINTSTOPOINT(ht.pt, pts);
	INT index = (INT)CallPrevWndProc(LVM_HITTEST, 0, (LPARAM)&ht);
	if (-1 == index || 
		0 == CallPrevWndProc(LVM_GETITEMSTATE, (WPARAM)index, 
					(LPARAM)(LVIS_SELECTED)))
	{
		flags |= MULTISELECT_STARTED;
		MouseHook *pHook = ListViewSelectionHook::CreateHook(hwnd);
		if (NULL != pHook) 
		{
			if (hwnd !=GetFocus())
				SetFocus(hwnd);
			return;
		}
	}

	CallPrevWndProc(WM_LBUTTONDOWN, (WPARAM)uFlags, *((LPARAM*)&pts));
}

void SimpleView::OnRButtonDown(UINT uFlags, POINTS pts)
{

	LVHITTESTINFO ht;
	POINTSTOPOINT(ht.pt, pts);
	INT index = (INT)CallPrevWndProc(LVM_HITTEST, 0, (LPARAM)&ht);
	if (-1 == index || 
		0 == CallPrevWndProc(LVM_GETITEMSTATE, (WPARAM)index, 
					(LPARAM)(LVIS_SELECTED)))
	{
		flags |= MULTISELECT_STARTED;
		MouseHook *pHook = ListViewSelectionHook::CreateHook(hwnd);
		if (NULL != pHook) 
		{
			if (hwnd !=GetFocus())
				SetFocus(hwnd);
			return;
		}
	}

	CallPrevWndProc(WM_RBUTTONDOWN, (WPARAM)uFlags, *((LPARAM*)&pts));
}

void SimpleView::OnCommand(INT ctrlId, INT eventId, HWND hwndCtrl)
{
	switch(ctrlId)
	{
		case ID_SIMPLEVIEW_SHOWINDEX:
			painter.SetStyle(painter.GetStyle(PAINTER_DRAWINDEX) ^ PAINTER_DRAWINDEX, PAINTER_DRAWINDEX);
			break;
		case ID_SIMPLEVIEW_SHOWTYPEICON:
			painter.SetStyle(painter.GetStyle(PAINTER_DRAWTYPEICON) ^ PAINTER_DRAWTYPEICON, PAINTER_DRAWTYPEICON);
			break;
		case ID_SIMPLEVIEW_RIGHTCOLUMN_NONE:
			break;
		case ID_SIMPLEVIEW_RIGHTCOLUMN_LENGTH:
			break;
		case ID_SIMPLEVIEW_RIGHTCOLUMN_SIZE:
			break;
		case ID_SIMPLEVIEW_RIGHTCOLUMN_EXTENSION:
			break;
	}
	BaseListView::OnCommand(ctrlId, eventId, hwndCtrl);
}

void SimpleView::OnKeyDown(UINT vkCode, UINT flags)
{
	switch(vkCode)
	{
		case VK_LEFT:
		case VK_RIGHT:
			vkCode = 0;
			break;
		case VK_ADD:
			if (0x8000 & GetAsyncKeyState(VK_CONTROL))
				vkCode = 0;	
			break;
	}
	BaseListView::WindowProc(WM_KEYDOWN, (WPARAM)vkCode, (LPARAM)flags);
	
}

void SimpleView::OnSetFont(HFONT hFont, BOOL bRedraw)
{
	CallPrevWndProc(WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
	RECT rc;
	GetWindowRect(hwnd, &rc);

	flags |= INDEXCOLUMN_INVALIDWIDTH;

	if (0 == (WINDOW_EMPTY & flags))
	{
		DWORD windowStyle = GetWindowLong(hwnd, GWL_STYLE);
		if (0 != (WS_VISIBLE & windowStyle))
			SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle & ~WS_VISIBLE);

		INT iTop = (INT)CallPrevWndProc(LVM_GETTOPINDEX, 0, 0L);
		
		CallPrevWndProc(LVM_ENSUREVISIBLE, (WPARAM)0, FALSE);

		SetWindowPos(hwnd, NULL, 0, 0, rc.right - rc.left + 1, rc.bottom - rc.top + 1, 
			SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOSENDCHANGING | SWP_NOREDRAW);
		SetWindowPos(hwnd, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, 
			SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOSENDCHANGING | SWP_NOREDRAW);

		if (iTop > 0)
		{
			RECT rcItem;
			rcItem.left = LVIR_BOUNDS;
			if(CallPrevWndProc(LVM_GETITEMRECT, iTop, (LPARAM)&rcItem))
				CallPrevWndProc(LVM_SCROLL, 0, (LPARAM)(iTop * (rcItem.bottom - rcItem.top)));
		}

		if (0 != (WS_VISIBLE & windowStyle))
			SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle);
	}
	
	if (bRedraw)
		RedrawWindow(hwnd, NULL, NULL, RDW_FRAME | RDW_INVALIDATE);
	
}
LRESULT SimpleView::OnSetItemCount(INT cItems, DWORD dwFlags)
{
	flags |= INDEXCOLUMN_INVALIDWIDTH;
	
	if (NULL != pActiveDocument && 0 == cItems)
		flags |= WINDOW_EMPTY;
	else
		flags &= ~WINDOW_EMPTY;
	
	INT page = (INT)CallPrevWndProc(LVM_GETCOUNTPERPAGE, 0, 0L);
	if (page >= cItems)
	{
		CallPrevWndProc(WM_VSCROLL, MAKEWPARAM(SB_TOP, 0), NULL);
		CallPrevWndProc(WM_VSCROLL, MAKEWPARAM(SB_ENDSCROLL, 0), NULL);
	}
	
	LRESULT result = BaseListView::OnSetItemCount(cItems, dwFlags);

	if (cItems > 0 && page < cItems)
	{
		INT iTop = (INT)CallPrevWndProc(LVM_GETTOPINDEX, 0, 0L);
		RECT rc, rcItem;
		rcItem.left = LVIR_BOUNDS;
		if(iTop > 0 && 
			GetClientRect(hwnd, &rc) &&
			CallPrevWndProc(LVM_GETITEMRECT, cItems - 1, (LPARAM)&rcItem) &&
			rcItem.top >= rc.top && rcItem.bottom < rc.bottom)
		{
			INT itemHeight = rcItem.bottom - rcItem.top;
			INT k = (rc.bottom - rcItem.bottom) / itemHeight;
			if (0 != k)
			{
				if (k > iTop) k = iTop;
				CallPrevWndProc(LVM_SCROLL, 0, (LPARAM)(-k * itemHeight));
			}
		}
	}
	return result;
}

void SimpleView::PaintEmptyList(HDC hdc, RECT *prcPaint, BOOL fErase)
{


	COLORREF rgbFg = GetThemeColor(COLOR_WINDOWTEXT);
	COLORREF rgbBk = GetThemeColor(COLOR_WINDOW);

	if (0 != (WS_DISABLED & GetWindowLongPtr(hwnd, GWL_STYLE)))
		rgbFg = BlendColors(rgbFg, rgbBk, DISABLEDTEXT_ALPHA);
	
	SetBkMode(hdc, OPAQUE);
	SetBkColor(hdc, rgbBk);
	SetTextColor(hdc, rgbFg);
	
	
	if (fErase)
	{	
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, prcPaint, NULL, 0, NULL);
	}

	TCHAR szText[256];
	

	WASABI_API_LNGSTRINGW_BUF(IDS_EMPTYLIST_MESSAGE, szText, ARRAYSIZE(szText));
	
	if (szText[0] != TEXT('\0'))
	{
		RECT rc, rcText;
		GetClientRect(hwnd, &rc);
		InflateRect(&rc, -2, -4);

		HFONT hf, hfo;
		hf = (HFONT)CallPrevWndProc(WM_GETFONT, 0, 0L);
		if (NULL == hf) hf = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
		hfo = (NULL != hf) ? (HFONT)SelectObject(hdc, hf) : NULL;

		CopyRect(&rcText, &rc);

        rcText.bottom = rcText.top + 
						DrawText(hdc, szText, -1, &rcText, DT_CALCRECT | DT_EXPANDTABS | DT_NOPREFIX | DT_NOCLIP);
		
		if ((rcText.right - rcText.left) <= (rc.right - rc.left) &&
			(rcText.bottom - rcText.top) <= (rc.bottom - rc.top))
		{
			LONG offsetY = ((rc.bottom - rc.top) - (rcText.bottom - rcText.top)) / 2;
			LONG offsetX = ((rc.right - rc.left) - (rcText.right - rcText.left)) / 2;

			if (offsetY || offsetX)
				OffsetRect(&rcText, offsetX, offsetY);
	
			DrawText(hdc, szText, -1, &rcText, DT_CENTER | DT_EXPANDTABS | DT_NOPREFIX | DT_NOCLIP);
		}

		if (NULL != hfo)
		SelectObject(hdc, hfo);
	}
}

LRESULT SimpleView::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	switch(uMsg)
	{		
		case WM_NCCALCSIZE:
			{				
				DWORD style = GetWindowStyle(hwnd);
				if (0 != (WS_HSCROLL & style))
				{
					SetWindowLongPtr(hwnd, GWL_STYLE, (style & ~WS_HSCROLL));
					SetWindowPos(hwnd, 0, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | 
									SWP_NOOWNERZORDER | SWP_FRAMECHANGED | SWP_NOREDRAW);
					return 0;
				}
			}
			break;
		case WM_ERASEBKGND:			return 0;
		case WM_SETFOCUS:			flags  |= WINDOW_HASFOCUS; break;
		case WM_KILLFOCUS:			flags &= ~WINDOW_HASFOCUS; break;
		case WM_LBUTTONDOWN:			OnLButtonDown((UINT)wParam, MAKEPOINTS(lParam)); return 0;
		case WM_RBUTTONDOWN:			OnRButtonDown((UINT)wParam, MAKEPOINTS(lParam)); return 0;
		case WM_KEYDOWN:			OnKeyDown((UINT)wParam, (UINT)lParam); return 0;
		case WM_SETFONT:			OnSetFont((HFONT)wParam, LOWORD(lParam)); return 0;
		case WM_PAINT:
			if (0 != (WINDOW_EMPTY & flags))
			{
				PAINTSTRUCT ps;
				if (BeginPaint(hwnd, &ps))
				{
					PaintEmptyList(ps.hdc, &ps.rcPaint, ps.fErase);
					EndPaint(hwnd, &ps);
					return 0;
				}
			}
			break;
		case WM_PRINTCLIENT:
			if (0 != (WINDOW_EMPTY & flags))
			{
				RECT rcPaint;
				if (GetClientRect(hwnd, &rcPaint))
				{
					PaintEmptyList((HDC)wParam, &rcPaint, 0 != (PRF_ERASEBKGND & lParam));
					return 0;
				}
			}
			break;
	}


	return BaseListView::WindowProc(uMsg, wParam, lParam);
}

