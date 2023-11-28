#include "./main.h"
#include "./listSelectHook.h"
#include <commctrl.h>

#define SCROLLTIMER_DELAY	25
#define SCROLLTIMER_ID		87

static int GetItemOutsideIndex(HWND hList, POINT *ppt)
{
	LVHITTESTINFO ht;
	ht.pt = *ppt;

	INT iItem = (INT)SendMessage(hList, LVM_HITTEST, 0, (LPARAM)&ht);
	if (-1 ==iItem)
	{
		RECT rc;
		rc.left = LVIR_BOUNDS;
		INT topItem = (INT)SendMessage(hList, LVM_GETTOPINDEX, 0, 0L);
		if (SendMessage(hList, LVM_GETITEMRECT, (WPARAM)topItem, (LPARAM)&rc))
		{
			if (ppt->y < rc.top) iItem = topItem - 1;
			else 
			{
				GetClientRect(hList, &rc);
				ht.pt.y = rc.bottom - 1;
				iItem = (INT)SendMessage(hList, LVM_HITTEST, 0, (LPARAM)&ht);
				if (-1== iItem) iItem = (INT)SendMessage(hList, LVM_GETITEMCOUNT, 0, 0L);
			}
		}
	}
	return iItem;
}

static void GetVScrollRect(HWND hwnd, RECT *prc)
{
	SetRectEmpty(prc);
	if (0 != (WS_VSCROLL & GetWindowLongPtr(hwnd, GWL_STYLE)))
	{
		RECT rcWindow, rc;
		if(GetWindowRect(hwnd, &rcWindow) && GetClientRect(hwnd, &rc))
		{
			MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&rcWindow, 2);
			if ((rc.left - rcWindow.left) > (rcWindow.right - rc.right))
				SetRect(prc, rcWindow.left + (rcWindow.right - rc.right), rc.top, rc.left, rc.bottom);
			else
				SetRect(prc, rc.right, rc.top, rcWindow.right - (rc.left - rcWindow.left), rc.bottom);
		}
	}
}

ListViewSelectionHook::ListViewSelectionHook(HWND hList, POINT *ppt) : 
	MouseHook(hList), currentItem(-1), timerActivated(FALSE), bDestroying(FALSE), 
	hbmpBk(NULL), headerHeight(0), rgnInvalid(NULL), rgnScrollbar(NULL)
{
	GetClientRect(hList, &clientRect);

	startPoint.y = ppt->y;
	startPoint.x = clientRect.left + (clientRect.right - clientRect.left)/2;

	startItem = GetItemOutsideIndex(hOwner, &startPoint);
	currentItem = startItem;

	LVITEM lvi;
		
	if (0 != (0x8000 & GetAsyncKeyState(VK_SHIFT)))
	{
		INT selectionMark = (INT)SendMessage(hOwner, LVM_GETSELECTIONMARK, 0, 0L); 
		if (-1 != selectionMark)
		{
			startItem = selectionMark;
			INT count = (INT)SendMessage(hOwner, LVM_GETITEMCOUNT, 0, 0L);
			INT iFrom = (currentItem < 0 ) ? 0 : ((currentItem < count) ? currentItem : (count - 1));
			INT iTo = (selectionMark > iFrom) ? selectionMark : iFrom;
			if (selectionMark < iTo) iFrom = selectionMark;

			lvi.stateMask = LVIS_SELECTED;
			lvi.state = 0;
			SendMessage(hOwner, LVM_SETITEMSTATE, (WPARAM)-1, (LPARAM)&lvi);
			
			lvi.state = LVIS_SELECTED;
			for (;iFrom <= iTo;iFrom++)
			{
				SendMessage(hOwner, LVM_SETITEMSTATE, (WPARAM)iFrom, (LPARAM)&lvi);
			}
		}
	}
	else
	{	
		INT itemCount = (INT)SendMessage(hOwner, LVM_GETITEMCOUNT, 0, 0L);

		if (itemCount > 0)
		{
			lvi.stateMask = LVIS_FOCUSED;
			if (0 == (0x8000 & GetAsyncKeyState(VK_CONTROL)))
			{
				INT selectedCount = (INT)SendMessage(hOwner, LVM_GETSELECTEDCOUNT, 0, 0L);
				if (selectedCount > 0)
					lvi.stateMask |= LVIS_SELECTED;
			}

			lvi.state = 0;
			SendMessage(hOwner, LVM_SETITEMSTATE, (WPARAM)-1, (LPARAM)&lvi);
			
			
			INT selectionMark = (startItem < 0 ) ? 0 : ((startItem < itemCount) ? startItem : (itemCount - 1));
			SendMessage(hOwner, LVM_SETSELECTIONMARK, (WPARAM)0, (LPARAM)selectionMark); 
		}
	}

	if (-1 != currentItem)
	{
		lvi.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
		lvi.state = LVIS_SELECTED | LVIS_FOCUSED;
		SendMessage(hOwner, LVM_SETITEMSTATE, (WPARAM)currentItem, (LPARAM)&lvi);
	}

	RECT rcItem;
	rcItem.left = LVIR_BOUNDS;
	if (!SendMessage(hOwner, LVM_GETITEMRECT, (WPARAM)0, (LPARAM)&rcItem))
		itemHeight = 0;
	else
		itemHeight = rcItem.bottom - rcItem.top;

	HWND hHeader = (HWND)SendMessage(hOwner, LVM_GETHEADER, 0, 0L);
	if (NULL != hHeader && 0 != (WS_VISIBLE & GetWindowLongPtr(hHeader, GWL_STYLE)))
	{
		RECT rcHeader;
		if (GetWindowRect(hHeader, &rcHeader))
		{
			MapWindowPoints(HWND_DESKTOP, hOwner, (POINT*)&rcHeader, 2);
			if (rcHeader.top < 0) OffsetRect(&rcHeader, 0, -rcHeader.top);
			headerHeight = rcHeader.bottom - rcHeader.top;
		}
	}
	clientRect.top += headerHeight;

	RECT rcScroll;
	GetVScrollRect(hOwner, &rcScroll);
	if (!IsRectEmpty(&rcScroll))
		rgnScrollbar = CreateRectRgnIndirect(&rcScroll);
		
	focusVisible = ( 0 == (UISF_HIDEFOCUS & SendMessage(hOwner, WM_QUERYUISTATE, 0, 0L))); 
}


ListViewSelectionHook::~ListViewSelectionHook()
{
	bDestroying = TRUE;
	if (NULL != hOwner)
	{
		if (hOwner == GetCapture())
			ReleaseCapture();
	}

	if (NULL != hbmpBk)
	{
		DeleteObject(hbmpBk);
	}

	if (NULL != rgnInvalid)
		DeleteObject(rgnInvalid);

	if (NULL != rgnScrollbar)
		DeleteObject(rgnScrollbar);
}

MouseHook *ListViewSelectionHook::CreateHook(HWND hList)
{
	KillTimer(hList, 42);
	KillTimer(hList, 43);

	POINT pt;
	GetCursorPos(&pt);
	MapWindowPoints(HWND_DESKTOP, hList, &pt, 1);

	ListViewSelectionHook *pmh = new ListViewSelectionHook(hList, &pt);
	if (NULL == pmh->hook)
	{
		pmh->Release();
		pmh = NULL;
	}
	else 
		SetCapture(hList);
	return pmh;
}

void ListViewSelectionHook::UpdateSelection(LONG y, BOOL bInvertMode, BOOL *startTimer)
{
	POINT pt;
	pt.x = startPoint.x;
	pt.y = y;
	
	if (NULL != startTimer)
	{
		*startTimer = FALSE;
	}

	LVHITTESTINFO ht;
	ZeroMemory(&ht, sizeof(LVHITTESTINFO));
	ht.pt = pt;
	
	INT itemCount = (INT)SendMessage(hOwner, LVM_GETITEMCOUNT, 0, 0L);
	
	INT item;
	if (pt.y  < clientRect.top) 
	{
		INT topItem = (INT)SendMessage(hOwner, LVM_GETTOPINDEX, 0, 0L);
		item = (topItem >= 0) ? topItem - 1 : -1;
		if (topItem > 0)
			ht.flags = LVHT_ABOVE;
	}
	else if (pt.y >= clientRect.bottom)
	{
		ht.pt.y = clientRect.bottom - 1;
		item = (INT)SendMessage(hOwner, LVM_HITTEST, 0, (LPARAM)&ht);
		if (-1 == item) item = itemCount;
		if (item < itemCount)
			ht.flags |= LVHT_BELOW;
	}
	else
	{
		item = (INT)SendMessage(hOwner, LVM_HITTEST, 0, (LPARAM)&ht);
		if (-1 == item)
		{
			item = itemCount;
		}
		ht.flags &= ~(LVHT_ABOVE | LVHT_BELOW);
	}
	
	if (0 != ((LVHT_ABOVE | LVHT_BELOW) & ht.flags))
	{
		if (!timerActivated)
		{			
			if (NULL == startTimer)
				SetTimer(hOwner, SCROLLTIMER_ID, SCROLLTIMER_DELAY, NULL);
			else
				*startTimer = TRUE;
			timerActivated = TRUE;
		}
	}

	if (item == currentItem)
		return;

	LVITEM lvi;
	lvi.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
	lvi.state = 0;
	INT iFrom, iTo;

	if (currentItem > item)
	{
		iFrom = currentItem;
		iTo = (item >= 0) ? item : 0;
		if (startItem > item) iTo--;
		
		for (iFrom; iFrom > iTo; iFrom--)
		{
			lvi.state = (iFrom > startItem) ? 0 : LVIS_SELECTED;
			SendMessage(hOwner, LVM_SETITEMSTATE, (WPARAM)iFrom, (LPARAM)&lvi);
		}
	}
	else 
	{
		iFrom = (currentItem >= 0) ? currentItem : 0;
		iTo = item; 

		if (startItem < item) iTo++;
		
		for (; iFrom < iTo; iFrom++)
		{
			lvi.state = (iFrom < startItem) ? 0 : LVIS_SELECTED;
			SendMessage(hOwner, LVM_SETITEMSTATE, (WPARAM)iFrom, (LPARAM)&lvi);
		}
	}

	currentItem = item;

	if (item < 0) item = 0;
	if (item >= itemCount) item = itemCount - 1;
	if (-1 != item)
	{
		lvi.stateMask = LVIS_FOCUSED;
		lvi.state = LVIS_FOCUSED;
		SendMessage(hOwner, LVM_SETITEMSTATE, (WPARAM)item, (LPARAM)&lvi);
	}
}

void ListViewSelectionHook::OnMouseMove(UINT uFlags, POINTS pts)
{
	BOOL startTimer;
	UpdateSelection(pts.y, (0 != (MK_CONTROL & uFlags)), &startTimer);
	if (startTimer)
		SetTimer(hOwner, SCROLLTIMER_ID, SCROLLTIMER_DELAY, NULL);
}

void ListViewSelectionHook::OnScrollTimer(UINT_PTR idEvent)
{
	POINT pt;
	KillTimer(hOwner, idEvent);
	timerActivated = FALSE;

	DWORD windowStyle = GetWindowStyle(hOwner);

	if (hOwner != GetFocus() || 0 == (WS_VSCROLL & windowStyle))
	{
		FinishScroll();
		Release();
		return;
	}

	SCROLLINFO si;
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
	if (!GetScrollInfo(hOwner, SB_VERT, &si))
		return;

	GetCursorPos(&pt);
	MapWindowPoints(HWND_DESKTOP, hOwner, &pt, 1);

	if ((clientRect.top <= pt.y && pt.y <= clientRect.bottom) ||
		(clientRect.top > pt.y && si.nPos <= si.nMin) ||
		(clientRect.bottom < pt.y && ((si.nPos + si.nPage) >= (UINT)si.nMax)))
	{
		return;
	}

	INT k = ((pt.y < clientRect.top) ? (clientRect.top - pt.y) : (pt.y - clientRect.bottom)) / (itemHeight*2);
	
	if (k < 1) k = 1;
	if (k > 10) k = 10;
	
	if (clientRect.top > pt.y && si.nPos < (si.nMin + k))
		k = si.nPos - si.nMin;
	if (clientRect.bottom < pt.y && ((si.nPos + si.nPage + k) > (UINT)si.nMax))
		k = si.nMax - (si.nPos + si.nPage);
	
	if (0 == k)
		return;

	INT height = itemHeight*k;
	
	if (clientRect.top > pt.y) height= -height;

	
	if (0 != (WS_VISIBLE & windowStyle))
		SetWindowLongPtr(hOwner, GWL_STYLE, windowStyle & ~WS_VISIBLE);
	
	SendMessage(hOwner, LVM_SCROLL, 0, (LPARAM)height);
	BOOL startTimer;
	INT timerDelay = SCROLLTIMER_DELAY;
	UpdateSelection(pt.y, (0 != (0x8000 & GetAsyncKeyState(VK_CONTROL))), &startTimer);
					
	if (0 != (WS_VISIBLE & windowStyle))
	{
		
		if (NULL == rgnInvalid)
			rgnInvalid = CreateRectRgn(0,0,0,0);
		else
			SetRectRgn(rgnInvalid, 0,0,0,0);

		SetWindowLongPtr(hOwner, GWL_STYLE, windowStyle | WS_VISIBLE);
				
		BOOL smoothScroll = ((pt.y < clientRect.top && ((clientRect.top - pt.y) < itemHeight)) ||
							(pt.y > clientRect.bottom && ((pt.y - clientRect.bottom) < itemHeight)));
		if (smoothScroll)
		{
			SmoothScroll(-height);
			timerDelay = 0;
		}
		else
		{
			RECT rcInvalid;
			INT reminder = (clientRect.bottom - clientRect.top)%itemHeight;
			if (height > 0)
			{
				INT invalidTop = clientRect.bottom - reminder;
				if (startItem > currentItem || (startItem < currentItem && focusVisible))
				{
					invalidTop -= itemHeight;
				}
				SetRect(&rcInvalid, clientRect.left, invalidTop, clientRect.right, clientRect.bottom);
				RedrawWindow(hOwner, &rcInvalid, NULL, 
						RDW_INVALIDATE | RDW_NOERASE | RDW_NOCHILDREN );
			}

			RECT rcScroll;
			SetRect(&rcScroll, clientRect.left, clientRect.top, clientRect.right, clientRect.bottom);
			if (ERROR == ScrollWindowEx(hOwner, 0, -height, &rcScroll, &rcScroll, NULL, &rcInvalid, 0))
				SetRect(&rcInvalid, rcScroll.left, rcScroll.top, rcScroll.right, rcScroll.bottom);
			
			if (!IsRectEmpty(&rcInvalid))
			{	
				SetRectRgn(rgnInvalid, rcInvalid.left, rcInvalid.top - reminder, rcInvalid.right, rcInvalid.bottom);
			}
		}
		
		if (NULL != rgnScrollbar)
			CombineRgn(rgnInvalid, rgnInvalid, rgnScrollbar, RGN_OR);
		

		if(NULL != rgnInvalid)
		{
			RedrawWindow(hOwner, NULL, rgnInvalid,
				RDW_INVALIDATE | RDW_FRAME | RDW_NOCHILDREN);
		}
	}

	if (startTimer)
	{
		SetTimer(hOwner, SCROLLTIMER_ID, timerDelay, NULL);
	}
	
}

static void dump_region(char *p, HRGN hrgn)
{
    DWORD i, size;
    RGNDATA *data = NULL;
    RECT *rect;

    if (!hrgn) {
        aTRACE_FMT( "%s null region\n", p );
        return;
    }
    if (!(size = GetRegionData( hrgn, 0, NULL ))) {
        return;
    }
    if (!(data = (RGNDATA*)HeapAlloc( GetProcessHeap(), 0, size ))) return;
    GetRegionData( hrgn, size, data );
    aTRACE_FMT("%s %ld rects:", p, data->rdh.nCount );
    for (i = 0, rect = (RECT *)data->Buffer; i<20 && i < data->rdh.nCount; i++, rect++)
	{
		aTRACE_FMT(" {%d, %d, %d, %d}", rect->left, rect->top, rect->right, rect->bottom);
	}
	aTRACE_FMT("\n");
    HeapFree( GetProcessHeap(), 0, data );
}

BOOL ListViewSelectionHook::SmoothScroll(INT dy)
{
	HDC hdc = GetDCEx(hOwner, NULL, DCX_CACHE | DCX_CLIPSIBLINGS | DCX_CLIPCHILDREN | DCX_NORESETATTRS);
	if (NULL == hdc)
		return FALSE;
	
	HBITMAP hbmpBkOld;

	INT reminder = (clientRect.bottom - clientRect.top)%itemHeight;

	RECT rcScroll;
	CopyRect(&rcScroll, &clientRect);
	
	HDC hdcBk = CreateCompatibleDC(hdc);
	if (NULL != hbmpBk)
	{
		BITMAP bm;
		if (!GetObject(hbmpBk, sizeof(BITMAP), &bm) || 
			bm.bmWidth < (rcScroll.right - rcScroll.left) ||
			abs(bm.bmHeight) < (abs(dy) + reminder))
		{
			DeleteObject(hbmpBk);
		}
	}
	if (NULL == hbmpBk)
	{
		hbmpBk = CreateCompatibleBitmap(hdc, rcScroll.right - rcScroll.left, itemHeight + reminder);
	}

	hbmpBkOld = (HBITMAP)SelectObject(hdcBk, hbmpBk);
	INT bkTop, destTop, destHeight, step;
	POINT ptOrig;

	if (dy < 0)
	{
		SetViewportOrgEx(hdcBk, rcScroll.left, -(rcScroll.bottom + dy - reminder), &ptOrig);
		SendMessage(hOwner, WM_PRINTCLIENT, (WPARAM)hdcBk, (LPARAM)PRF_CLIENT | PRF_ERASEBKGND);
		SetViewportOrgEx(hdcBk, ptOrig.x, ptOrig.y, NULL);
		step = 1;
		bkTop = 0;
		destTop = rcScroll.bottom -1;
	}
	else
	{
		SetViewportOrgEx(hdcBk, rcScroll.left, -rcScroll.top, &ptOrig);
		SendMessage(hOwner, WM_PRINTCLIENT, (WPARAM)hdcBk, (LPARAM)PRF_CLIENT | PRF_ERASEBKGND);
		SetViewportOrgEx(hdcBk, ptOrig.x, ptOrig.y, NULL);
		step = -1;
		bkTop = dy - 1;
		destTop = rcScroll.top;
	}

	if (dy < 0 && 0 != reminder)
	{
		BitBlt(hdc, rcScroll.left, rcScroll.bottom - reminder, rcScroll.right, reminder, hdcBk, 0, 0, SRCCOPY);
		bkTop += reminder;
	}
	
	destHeight = abs(step);
	HRGN rgnUpdate = CreateRectRgn(0, 0, 0, 0);

	for (int c = abs(dy); c != 0; c--)
	{
		ScrollDC(hdc, 0, -step, &rcScroll, &rcScroll, rgnUpdate, NULL);
		BitBlt(hdc, rcScroll.left, destTop, rcScroll.right, destHeight, hdcBk, 0, bkTop, SRCCOPY);
		bkTop += step;		
		if (1 != c) 
			SleepEx(14, TRUE);
	}
	DeleteObject(rgnUpdate);

	SelectObject(hdcBk, hbmpBkOld);
	DeleteDC(hdcBk);
	ReleaseDC(hOwner, hdc);

	return TRUE;

	//ScrollWindowEx(hOwner, 0, dy, NULL, NULL, NULL, NULL, SW_SMOOTHSCROLL | HIWORD(0));
	//return TRUE;
}

void ListViewSelectionHook::FinishScroll()
{
	RECT ri;
	ri.left = LVIR_BOUNDS;
	if (-1 != currentItem && 
		SendMessage(hOwner, LVM_GETITEMRECT, (WPARAM)currentItem, (LPARAM)&ri) && 
		(ri.top < clientRect.top || ri.bottom > clientRect.bottom))
	{
		DWORD windowStyle = GetWindowStyle(hOwner);
		if (0 != (WS_VISIBLE & windowStyle))
			SetWindowLongPtr(hOwner, GWL_STYLE, windowStyle & ~WS_VISIBLE);
			
		SendMessage(hOwner, LVM_ENSUREVISIBLE, currentItem, FALSE);
			
		if (0 != (WS_VISIBLE & windowStyle))
		{
			SetWindowLongPtr(hOwner, GWL_STYLE, windowStyle);

			LONG oldTop = ri.top;
			ri.left = LVIR_BOUNDS;
			SendMessage(hOwner, LVM_GETITEMRECT, (WPARAM)currentItem, (LPARAM)&ri);
				
			RECT rcScroll;
			CopyRect(&rcScroll, &clientRect);
			if (ri.top != oldTop)
			{
				if (NULL == rgnInvalid)
					rgnInvalid = CreateRectRgn(0, 0, 0, 0);
				ScrollWindowEx(hOwner, 0, (ri.top - oldTop), &rcScroll, &rcScroll, rgnInvalid, NULL, 0);


				if (NULL != rgnScrollbar)
					CombineRgn(rgnInvalid, rgnInvalid, rgnScrollbar, RGN_OR);
				
				if(NULL != rgnInvalid)
				{
					RedrawWindow(hOwner, NULL, rgnInvalid,
						RDW_INVALIDATE | RDW_FRAME | RDW_NOCHILDREN);
				}
			}
		}
	}
	
}

LRESULT ListViewSelectionHook::HookProc(int nCode, MSG *pMsg)
{
	if (pMsg->hwnd != hOwner)
		return CallNext(nCode, pMsg);

	switch(pMsg->message)
	{
		case WM_RBUTTONUP:
			FinishScroll();
			Release();
			SendMessage(pMsg->hwnd, WM_CONTEXTMENU, (WPARAM)pMsg->hwnd, MAKELPARAM(pMsg->pt.x, pMsg->pt.y));
			return 1;
		case WM_LBUTTONUP:
			FinishScroll();
			Release();
			return 1;
		case WM_MOUSEMOVE:
			if (0 == ((MK_LBUTTON | MK_RBUTTON) & pMsg->wParam))
			{
				LRESULT r = CallNext(nCode, pMsg);
				FinishScroll();
				Release();
				return r;
			}
			OnMouseMove((UINT)pMsg->wParam, MAKEPOINTS(pMsg->lParam));
			return 1;
		case WM_TIMER:
			switch(pMsg->wParam)
			{
				case SCROLLTIMER_ID:
					OnScrollTimer((UINT_PTR)pMsg->wParam);
					return 1;
			}
			break;
		case WM_KILLFOCUS:
		case WM_CAPTURECHANGED:
			if (!bDestroying && NULL != hOwner && 
				hOwner == GetCapture() && hOwner != (HWND)pMsg->lParam)
			{
				FinishScroll();
				Release();
			}
			break;
		case WM_KEYDOWN:
		case WM_KEYUP:
			// eat key messages
			return 1;
	}
	return CallNext(nCode, pMsg);
}