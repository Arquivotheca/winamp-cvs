#ifndef NULLSOFT_DROPBOX_PLUGIN_LISTVIEW_DRAGSCROLL_HEADER
#define NULLSOFT_DROPBOX_PLUGIN_LISTVIEW_DRAGSCROLL_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

class ListViewDragScroll
{
public:
	ListViewDragScroll(HWND hList);
	~ListViewDragScroll();

public:
	BOOL Scroll(POINT pt);
	void Reset();
	BOOL IsScrolling();
	HWND GetHWND() { return hwnd; }
	
protected:
	void SetScroll(INT scrollZone);
	BOOL CanScroll(INT scrollZone);
	void OnScrollTimer(UINT_PTR idEvent);

private:
	static void CALLBACK OnScrollTimerReal(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
	UINT_PTR SetTimer(UINT_PTR idEvent, UINT uElapse);
	BOOL KillTimer(UINT_PTR idEvent);

private:
	HWND hwnd;
	UINT_PTR timerId;
	INT activeScrollZone;
	INT headerHeight;
	INT itemHeight;
	HRGN rgnScrollbar;
	HRGN rgnInvalid;
};


#endif // NULLSOFT_DROPBOX_PLUGIN_LISTVIEW_DRAGSCROLL_HEADER