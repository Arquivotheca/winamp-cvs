#include "main.h"
#include "./listDragScroll.h"
#include "./plugin.h"
#include "../nu/vector.h"
#include "../nu/trace.h"
#include <windows.h>
#include <commctrl.h>


typedef struct __OBJECTTIMERREC
{
	UINT_PTR timerId;
	ListViewDragScroll *pObject;
} OBJECTTIMERREC;

typedef Vector<OBJECTTIMERREC> TIMERLIST;

static TIMERLIST *timerList;


static INT systemScrollZone = 0;
static INT systemScrollDelay = 0;
static INT systemScrollInterval = 0;

typedef enum __SCROLLZONE
{
	SCROLLZONE_NONE = 0x0000,
	SCROLLZONE_TOP = 0x0001,
	SCROLLZONE_BOTTOM = 0x0002,
	SCROLLZONE_MASK = 0x000F,
	SCROLLZONE_INIT = 0x8000,
} SCROLLZONE;


static void CALLBACK UninitializeDragScroll(void)
{
	if (NULL != timerList)
	{
		size_t index = timerList->size();
		while(index--)
			KillTimer(NULL, timerList->at(index).timerId);
	}
	delete timerList;
}

static void InitializeDragScroll()
{	
	if (NULL == timerList)
		timerList = new TIMERLIST();

	systemScrollZone = GetProfileInt(TEXT("windows"), TEXT("DragScrollInset"), DD_DEFSCROLLINSET);
	systemScrollDelay = GetProfileInt(TEXT("windows"), TEXT("DragScrollDelay"),DD_DEFSCROLLDELAY);
	systemScrollInterval = GetProfileInt(TEXT("windows"), TEXT("DragScrollInterval"), DD_DEFSCROLLINTERVAL);

	Plugin_RegisterUnloadCallback(UninitializeDragScroll);
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

ListViewDragScroll::ListViewDragScroll(HWND hList) 
	: hwnd(NULL), timerId(0), headerHeight(0), itemHeight(0), rgnInvalid(NULL), rgnScrollbar(NULL)
{
	if (NULL == timerList)
	{
		InitializeDragScroll();
		if (0 == timerList)  return;
	}
	hwnd = hList;

	if (0 == (LVS_NOCOLUMNHEADER & GetWindowLongPtr(hwnd, GWL_STYLE)))
	{
		HWND hHeader = (HWND)SendMessage(hwnd, LVM_GETHEADER, 0, 0L);
		if (NULL != hHeader && 0 != (WS_VISIBLE & GetWindowLongPtr(hHeader, GWL_STYLE)))
		{
			RECT rc;
			GetWindowRect(hHeader, &rc);
			MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&rc, 2);
			headerHeight = rc.bottom;
		}
	}
	RECT rcScroll;
	GetVScrollRect(hwnd, &rcScroll);
	if (!IsRectEmpty(&rcScroll))
		rgnScrollbar = CreateRectRgnIndirect(&rcScroll);

	RECT rcItem;
	rcItem.left = LVIR_BOUNDS;
	if (SendMessage(hwnd, LVM_GETITEMRECT, (WPARAM)0, (LPARAM)&rcItem))
		itemHeight = rcItem.bottom - rcItem.top;
	

}

ListViewDragScroll::~ListViewDragScroll()
{
	Reset();

	size_t index = timerList->size();
	while(index--)
	{
		if (timerList->at(index).pObject == this)
			timerList->eraseAt(index);
	}

	if (NULL != rgnInvalid)
		DeleteObject(rgnInvalid);

	if (NULL != rgnScrollbar)
		DeleteObject(rgnScrollbar);
}

UINT_PTR ListViewDragScroll::SetTimer(UINT_PTR idEvent, UINT uElapse)
{
	UINT_PTR id = ::SetTimer(NULL, idEvent, uElapse, OnScrollTimerReal);
	size_t index = timerList->size();
	while(index--)
	{
		if (timerList->at(index).timerId == id)
		{
			timerList->at(index).pObject = this;
			return id;
		}
	}

	if (id != 0)
	{
		OBJECTTIMERREC rec;
		rec.pObject = this;
		rec.timerId = id;
		timerList->push_back(rec);
	}
	return id;
}

BOOL ListViewDragScroll::KillTimer(UINT_PTR idEvent)
{
	BOOL bResult = ::KillTimer(NULL, idEvent);
	size_t index = timerList->size();
	while(index--)
	{
		if (timerList->at(index).timerId == idEvent)
		{
			timerList->eraseAt(index);
			break;
		}
	}
	return bResult;
}

void ListViewDragScroll::Reset()
{
	if (0 != timerId)
	{
		KillTimer(timerId);
		timerId = 0;
	}
	activeScrollZone = SCROLLZONE_NONE;
}


BOOL ListViewDragScroll::IsScrolling()
{
	return (SCROLLZONE_NONE == (SCROLLZONE_MASK & activeScrollZone));
}


void ListViewDragScroll::SetScroll(INT scrollZone)
{
	if (scrollZone == (SCROLLZONE_MASK & activeScrollZone))
		return;
	if (SCROLLZONE_NONE != activeScrollZone)
		Reset();
	
	activeScrollZone = scrollZone | SCROLLZONE_INIT;
	timerId = SetTimer(0, systemScrollDelay);
}

void ListViewDragScroll::OnScrollTimer(UINT_PTR idEvent)
{
	if (SCROLLZONE_INIT & activeScrollZone)
	{		
		KillTimer(idEvent);
		activeScrollZone &= ~SCROLLZONE_INIT;
		timerId = SetTimer(timerId, systemScrollInterval);
	}

	if (CanScroll(activeScrollZone))
	{
		
		UINT scrollDir = (SCROLLZONE_TOP == activeScrollZone) ? SB_LINEUP : SB_LINEDOWN;

		DWORD windowStyle = GetWindowStyle(hwnd);
		if (0 != (WS_VISIBLE & windowStyle))
			SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle & ~WS_VISIBLE);

		SendMessage(hwnd, WM_VSCROLL, MAKEWPARAM(scrollDir, 0), NULL);
		SendMessage(hwnd, WM_VSCROLL, MAKEWPARAM(SB_ENDSCROLL, 0), NULL);

		if (0 != (WS_VISIBLE & windowStyle))
		{
			RECT rcScroll, rcInvalid;
			INT dy = (SB_LINEUP == scrollDir) ? itemHeight : -itemHeight;

			GetClientRect(hwnd, &rcScroll);
			rcScroll.top += headerHeight;

			SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle);
			ScrollWindowEx(hwnd, 0, dy, &rcScroll, &rcScroll, NULL, NULL, 0);
		
			CopyRect(&rcInvalid, &rcScroll);
			
			if (SB_LINEUP == scrollDir)
			{
				rcInvalid.bottom = rcInvalid.top + itemHeight;
			}
			else
			{
				INT reminder = (rcInvalid.bottom - rcInvalid.top)%itemHeight;
				rcInvalid.top = rcInvalid.bottom - itemHeight - reminder;
			}

			if (NULL == rgnInvalid)
				rgnInvalid = CreateRectRgnIndirect(&rcInvalid);
			else
				SetRectRgn(rgnInvalid, rcInvalid.left, rcInvalid.top, rcInvalid.right, rcInvalid.bottom);

			if (NULL != rgnScrollbar)
				CombineRgn(rgnInvalid, rgnInvalid, rgnScrollbar, RGN_OR);

			RedrawWindow(hwnd, NULL, rgnInvalid, 
				RDW_INVALIDATE | RDW_FRAME | RDW_NOCHILDREN | RDW_ERASE | RDW_UPDATENOW);
			
		}
	}
	else 
		Reset();
}

BOOL ListViewDragScroll::CanScroll(INT scrollZone)
{
	DWORD windowStyle = GetWindowStyle(hwnd);
	if (0 == (WS_VSCROLL & windowStyle))
		return FALSE;

	SCROLLINFO si;
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_RANGE | SIF_POS | SIF_PAGE;
	if (!GetScrollInfo(hwnd, SB_VERT, &si))
		return FALSE;

	switch(SCROLLZONE_MASK & scrollZone)
	{
		case SCROLLZONE_TOP:
			return (si.nPos > si.nMin);
		case SCROLLZONE_BOTTOM:
		{
			INT test = si.nPos + si.nPage;
			if (si.nMax == test)
			{
				INT iLast = (INT)SendMessage(hwnd, LVM_GETITEMCOUNT, 0, 0L);
				if (iLast < 1)
					return FALSE;
				
				iLast--;
				
				RECT rc, rcItem;
				rcItem.left = LVIR_BOUNDS;
				if (GetClientRect(hwnd, &rc) && 
					SendMessage(hwnd, LVM_GETITEMRECT, iLast, (LPARAM)&rcItem))
				{					
					return (rcItem.bottom > rc.bottom);
				}
				return FALSE;

			}
			return (si.nMax > test);
		}
	}
	return FALSE;
}

BOOL ListViewDragScroll::Scroll(POINT pt)
{
	RECT rc;
	if (!GetClientRect(hwnd, &rc))
		return FALSE;

	MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&pt, 1);
	rc.top  += headerHeight;
	 
	INT zone = itemHeight/2 + itemHeight%2;

	if (pt.y >= rc.top && pt.y <= (rc.top + zone) && CanScroll(SCROLLZONE_TOP))
	{
		SetScroll(SCROLLZONE_TOP);
		return TRUE;
	}
	if (pt.y >= (rc.bottom - zone) && pt.y <= rc.bottom && CanScroll(SCROLLZONE_BOTTOM))
	{
		SetScroll(SCROLLZONE_BOTTOM);
		return TRUE;
	}
	Reset();
	return FALSE;
}

void CALLBACK ListViewDragScroll::OnScrollTimerReal(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	
	size_t index = timerList->size();
	while(index--)
	{
		if (timerList->at(index).timerId == idEvent)
		{
			if (NULL != timerList->at(index).pObject)
				timerList->at(index).pObject->OnScrollTimer(idEvent);
			return;
		}
	}
	::KillTimer(NULL, idEvent);
	
}