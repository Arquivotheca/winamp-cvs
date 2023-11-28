#include "./main.h"
#include "./plugin.h"
#include "./listInsertMark.h"
#include "./itemView.h"
#include "./skinWindow.h"
#include "../nu/vector.h"

#include <windows.h>
#include <commctrl.h>


static LARGE_INTEGER frequency;

typedef struct __OBJECTTIMERREC
{
	UINT_PTR timerId;
	ListViewInsertMark *pObject;
} OBJECTTIMERREC;

typedef Vector<OBJECTTIMERREC> TIMERLIST;
static TIMERLIST *timerList;


static void CALLBACK UninitializeInsertMark(void)
{
	if (NULL != timerList)
	{
		size_t index = timerList->size();
		while(index--)
			KillTimer(NULL, timerList->at(index).timerId);
	}
	delete timerList;
}

static void InitializeInsertMark(void)
{	
	if (NULL == timerList)
		timerList = new TIMERLIST();
	QueryPerformanceFrequency(&frequency);
	Plugin_RegisterUnloadCallback(UninitializeInsertMark);
}

ListViewInsertMark::ListViewInsertMark(HWND hList) :
	hwnd(NULL), iAfter(-2), activeTop(-1), activeHeight(0), timerId(0)
{
	if (NULL == timerList)
	{
		InitializeInsertMark();
		if (0 == timerList)  return;
	}

	hwnd = hList;

	GetClientRect(hwnd, &clientRect);
	HWND hHeader = (HWND)SendMessage(hwnd, LVM_GETHEADER, 0, 0L);
	if (NULL != hHeader)
	{
		RECT rcHeader;
		if (GetWindowRect(hHeader, &rcHeader))
		{
			MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&rcHeader, 2);
			clientRect.top = rcHeader.bottom;
		}
		INT count = (INT)SendMessage(hHeader, HDM_GETITEMCOUNT, 0, 0L);
		if (count == 0)
		{
			clientRect.right = clientRect.left;
		}
		else
		{
			INT index = (INT)SendMessage(hHeader, HDM_ORDERTOINDEX, (WPARAM)(count- 1), 0L);
			if (SendMessage(hHeader, HDM_GETITEMRECT, (WPARAM)(index), (LPARAM)&rcHeader))
			{
				if(rcHeader.right < clientRect.right)
					clientRect.right = rcHeader.right;
			}
		}
	}

	iCount = (INT)SendMessage(hwnd, LVM_GETITEMCOUNT, 0, 0L);

	INT itemHeight;
	RECT rc;
	rc.left = LVIR_BOUNDS;
	if (SendMessage(hwnd, LVM_GETITEMRECT, (WPARAM)0, (LPARAM)&rc))
		itemHeight = (rc.bottom - rc.top);
	else
		itemHeight = 0;
	
	markHeight = (itemHeight > 0) ? ((3 * itemHeight) / 4) : 4;
	if (markHeight < 4)
		markHeight = 4;
	

	rgbBk = (COLORREF)SendMessage(hwnd, LVM_GETBKCOLOR, 0, 0L);

	DropboxView *pView = DropBox_GetItemView(hwnd);
	if (NULL != pView && pView->GetSkinned())
		rgbSelBk = GetSkinColor(COLOR_HIGHLIGHT);
	else
		rgbSelBk = GetSystemColor(COLOR_HIGHLIGHT);
	
	hdc = GetDCEx(hwnd, NULL, DCX_CACHE | DCX_CLIPCHILDREN | DCX_CLIPSIBLINGS);
	
//	markHeight = 48;
	
	injectedCount = (markHeight > 0 && itemHeight > 0) ? ((markHeight/itemHeight) + ((markHeight%itemHeight) ? 1 : 0)) : 0;
	if (injectedCount && !SendMessage(hwnd, LVM_SETITEMCOUNT, (WPARAM)(iCount + injectedCount), 
				(LPARAM)(LVSICF_NOSCROLL)))
	{
		injectedCount = 0;
		UpdateWindow(hwnd);
	}
	
	
}

ListViewInsertMark::~ListViewInsertMark(void)
{
	Remove(TRUE);

	if (0 != timerId)
	{
		KillTimer(timerId);
		timerId = 0;
	}

	size_t index = timerList->size();
	while(index--)
	{
		if (timerList->at(index).pObject == this)
			timerList->eraseAt(index);
	}

	if (NULL != hdc)
	{
		ReleaseDC(hwnd, hdc);
		hdc = NULL;
	}

	if (injectedCount > 0)
	{
		iCount = (INT)SendMessage(hwnd, LVM_GETITEMCOUNT, 0, 0L);
		SendMessage(hwnd, LVM_SETITEMCOUNT, (WPARAM)(iCount - injectedCount),
					(LPARAM)(LVSICF_NOSCROLL));
	}


//	UpdateWindow(hwnd);
}


UINT_PTR ListViewInsertMark::SetTimer(UINT_PTR idEvent, UINT uElapse)
{
	UINT_PTR id = ::SetTimer(NULL, idEvent, uElapse, OnAnimationTimerReal);
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

BOOL ListViewInsertMark::KillTimer(UINT_PTR idEvent)
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

BOOL ListViewInsertMark::UpdatePosition(POINT pt)
{
	RECT ri;
	LVHITTESTINFO ht;
	INT iItem;

	if (pt.y <= clientRect.top) 
	{
		iItem = (INT)SendMessage(hwnd, LVM_GETTOPINDEX, 0, 0L);
		if (-1 != iItem) iItem--;
		if (iItem == iAfter)
			return FALSE;
	}
	else if (pt.y >= clientRect.bottom)
	{
		pt.x = clientRect.left + (clientRect.right - clientRect.left)/2;
		pt.y = clientRect.bottom - 1;
		ht.pt = pt;	
		iItem = (INT)SendMessage(hwnd, LVM_HITTEST, 0, (LPARAM)&ht);
		if (-1 == iItem)
		{
			iItem = (iCount - 1);
			if (iItem == iAfter)
				return FALSE;
		}
	}
	else
	{
		pt.x = clientRect.left + (clientRect.right - clientRect.left)/2;
		if (activeTop > -1 && activeTop < pt.y)
		{
			if (pt.y >= (activeTop + markHeight))
				pt.y -= markHeight;
			else
				pt.y -= (pt.y - activeTop);
		}
		ht.pt = pt;
		iItem = (INT)SendMessage(hwnd, LVM_HITTEST, 0, (LPARAM)&ht);
		if (iItem >= iCount) iItem = -1;
		if (-1 == iItem)
		{
			iItem = (iCount - 1);
			if (iItem == iAfter || -1 == iItem)
				return FALSE;
		}
	}

	ri.left = LVIR_BOUNDS;
	if (iItem < 0)
	{
		if (iCount > 0 && SendMessage(hwnd, LVM_GETITEMRECT, (WPARAM)0, (LPARAM)&ri))
			activeTop = ri.top;
		else
			activeTop = 0;
	}
	else
	{
		SendMessage(hwnd, LVM_GETITEMRECT, (WPARAM)iItem, (LPARAM)&ri);
		activeTop = ri.bottom;
		//if (iItem < (iCount - 1))
		{
			ri.bottom = ri.top + (ri.bottom - ri.top)/2;
			if (PtInRect(&ri, pt))
			{
				activeTop = ri.top;
				iItem--;
			}
		}
	}

	if (iItem == iAfter)
		return FALSE;
	
	iAfter = iItem;
	return TRUE;
}

void ListViewInsertMark::Display(POINT pt, INT animationMs)
{	
	LONG oldTop = activeTop;
	if (!UpdatePosition(pt))
		return;

	if (0 != timerId)
	{
		KillTimer(timerId);
		timerId = 0;
	}

	if (NULL == hdc)
		return;

	if (oldTop > -1 && activeHeight < markHeight)
	{
		LONG tempTop = activeTop;
		activeTop = oldTop;
		INT distance = markHeight - activeHeight; 
		PerformScroll(distance * scrollDirection);
			
		DrawEx(activeTop, distance, rgbActive);

		activeHeight = markHeight;
		
		activeTop = tempTop;
	}

	
	scrollDirection = 1;
	if (oldTop > -1)
	{			
		if (activeTop < oldTop)
		{
			SetRect(&scrollRect, clientRect.left, activeTop, clientRect.right, oldTop + activeHeight - 1);
			SetRect(&scrollClipRect, clientRect.left, activeTop, clientRect.right, oldTop + activeHeight);
		}
		else if (activeTop > oldTop)
		{
			SetRect(&scrollRect, clientRect.left, oldTop + 1, clientRect.right, activeTop + activeHeight);
			SetRect(&scrollClipRect, clientRect.left, oldTop, clientRect.right, activeTop + activeHeight);
			scrollDirection = -1;
		}

	}
	else
	{
		UpdateWindow(hwnd);
		SetRect(&scrollRect, clientRect.left, activeTop, clientRect.right, clientRect.bottom);
		SetRect(&scrollClipRect, clientRect.left, activeTop, clientRect.right, clientRect.bottom);
	}

	if (iAfter >= 0 && iAfter < (iCount - 1))
	{
		if (0 != (LVIS_SELECTED & SendMessage(hwnd, LVM_GETITEMSTATE, (WPARAM)iAfter, (LPARAM)(LVIS_SELECTED))) && 
			0 != (LVIS_SELECTED & SendMessage(hwnd, LVM_GETITEMSTATE, (WPARAM)(iAfter + 1), (LPARAM)(LVIS_SELECTED))))
		{
			rgbActive = rgbSelBk;
		}
		else 
			rgbActive = rgbBk;
	}
	else 
		rgbActive = rgbBk;

	//if (scrollDirection > 0 )
	//	rgbActive = 0x0000FF;//(activeTop > 0) ? rgbBk : rgbSelBk;
	//else
	//	rgbActive = 0xFF0000;

	activeHeight = 0;
	if ( 0 == animationMs || !StartAnimatedScroll(animationMs)) 
	{
		PerformScroll(markHeight * scrollDirection);
		activeHeight = markHeight;
		Draw();
	}

}


BOOL ListViewInsertMark::StartAnimatedScroll(INT animationMs)
{	

	if (scrollDirection > 1) scrollDirection = 1;
	else if (scrollDirection < -1) scrollDirection = -1;

	if (activeHeight >= markHeight)
		return TRUE;
			
	if (!QueryPerformanceCounter(&scrollStarted))
		return FALSE;

	scrollAnimation = animationMs;
	scrollCounter = 0;
	timerId = SetTimer(0, 0);
	return (0 != timerId);
}

void ListViewInsertMark::OnAnimationTimer(UINT_PTR idEvent)
{	
	KillTimer(idEvent);
	if (timerId != idEvent)
		KillTimer(timerId);
	timerId = 0;
	
	LARGE_INTEGER current;
	UINT elapsedMs;

	QueryPerformanceCounter(&current);
	elapsedMs = (UINT)((current.QuadPart - scrollStarted.QuadPart) * 1000 / frequency.QuadPart);
	
	INT distance = (markHeight - activeHeight);
	if (distance <= 0)
		return;

	INT step = 1;
	if (elapsedMs >= scrollAnimation)
		step = distance;		
	else
	{
		INT t = (scrollAnimation - elapsedMs) * scrollCounter;
		INT m;
		if (elapsedMs > 0)
		 m = (t / elapsedMs) + ((t%elapsedMs) ? 1 : 0);
		else 
		m = t;
		
		step = (m > 0) ? (distance/m) : 1;
	}
	if (step < 1) step = 1;


	PerformScroll(step * scrollDirection);
	
	INT top;

	if (scrollDirection < 0)
		top = activeTop + (markHeight - activeHeight) - step;
	else 
		top = activeTop + activeHeight;

	DrawEx(top, step, rgbActive);

	activeHeight += step;
	scrollCounter++;

	distance = (markHeight - activeHeight);
	if (distance <= 0)
		return;

	QueryPerformanceCounter(&current);
	elapsedMs = (UINT)((current.QuadPart - scrollStarted.QuadPart) * 1000 / frequency.QuadPart);


	INT delay = (scrollAnimation > elapsedMs) ? ((scrollAnimation - elapsedMs)/distance) : 0;
	if (delay > 30) 
	{
		delay = 30;
	}

    timerId = SetTimer(0, delay);
}

BOOL ListViewInsertMark::Remove(BOOL bRedraw)
{
	if (0 != timerId)
	{
		KillTimer(timerId);
		timerId = 0;
	}

	if (activeTop > -1)
	{				
		if (activeHeight < markHeight)
		{
			INT distance = markHeight - activeHeight; 
			PerformScroll(distance * scrollDirection);
			activeHeight = markHeight;
		}

		SetRect(&scrollRect, clientRect.left, activeTop + 1, clientRect.right, clientRect.bottom + activeHeight);
		SetRect(&scrollClipRect, clientRect.left,  activeTop, clientRect.right, clientRect.bottom);
		
		if (activeHeight > 0 && bRedraw)
			PerformScroll(-activeHeight);
		
		activeTop = -1;
		activeHeight = 0;
		iAfter = -2;
		return TRUE;
	}
	return FALSE;
}

BOOL ListViewInsertMark::IsDisplayed()
{
	return (-1 != activeTop);
}

void ListViewInsertMark::PerformScroll(INT dy)
{
	if (NULL != hdc && 0 != dy)
	{
		if (scrollClipRect.bottom > clientRect.bottom) scrollClipRect.bottom = clientRect.bottom;
		if (scrollClipRect.top < clientRect.top) scrollClipRect.top = clientRect.top;

		BOOL invalidate = FALSE;
		if (scrollRect.bottom > clientRect.bottom)
		{
			scrollRect.bottom = clientRect.bottom;
			if (dy < 0) invalidate = TRUE;
		}
		if (scrollRect.top < clientRect.top)
		{
			scrollRect.top = clientRect.top;
			if (dy > 0) invalidate = TRUE;
		}
		

		ScrollDC(hdc, 0, dy, &scrollRect, &scrollClipRect, NULL, NULL);
		if (invalidate)
		{
			RECT ri;
			if (dy < 0)
				SetRect(&ri, clientRect.left, clientRect.bottom + dy,  clientRect.right, clientRect.bottom);
			else 
				SetRect(&ri, clientRect.left, clientRect.top,  clientRect.right, clientRect.top + dy);
			
			if (scrollClipRect.bottom < ri.bottom) ri.bottom = scrollClipRect.bottom;
			if (scrollClipRect.top > ri.top) ri.top = scrollClipRect.top;
			if (ri.top < ri.bottom)
				RedrawWindow(hwnd, &ri, NULL, RDW_INVALIDATE | RDW_NOERASE | RDW_NOCHILDREN);

		}
	}
}

void ListViewInsertMark::Draw()
{
	if (activeTop > -1 && activeHeight > 0)
		DrawEx(activeTop, activeHeight, rgbActive);
}

void ListViewInsertMark::DrawEx(LONG top, INT height, COLORREF rgb)
{
	if (NULL == hdc)
		return;
	if (top >= clientRect.bottom || (top + height) <= clientRect.top)
		return;
	
	RECT block;
	SetRect(&block, clientRect.left, top, clientRect.right, top + height);
				
	COLORREF rgbOld = SetBkColor(hdc, rgb);
	ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &block, NULL, 0, NULL);
	if (rgb != rgbOld) SetBkColor(hdc, rgbOld);
	
}

void ListViewInsertMark::AdjustItemRect(RECT *prcItem)
{
	if (-1 != activeTop && activeTop <= prcItem->top)
	{
		INT oy = (prcItem->top >= (activeTop + activeHeight)) ? activeHeight : (prcItem->top - activeTop);
		OffsetRect(prcItem, 0, oy);
	}
}

void CALLBACK ListViewInsertMark::OnAnimationTimerReal(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{	
	size_t index = timerList->size();
	while(index--)
	{
		if (timerList->at(index).timerId == idEvent)
		{
			if (NULL != timerList->at(index).pObject)
				timerList->at(index).pObject->OnAnimationTimer(idEvent);
			return;
		}
	}
	::KillTimer(NULL, idEvent);
}