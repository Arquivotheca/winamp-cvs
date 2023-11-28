#ifndef NULLSOFT_DROPBOX_PLUGIN_LISTVIEW_SPACER_HEADER
#define NULLSOFT_DROPBOX_PLUGIN_LISTVIEW_SPACER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>


class ListViewInsertMark 
{
public:
	ListViewInsertMark(HWND hwndList);
	~ListViewInsertMark(void);

public:
	void Display(POINT pt, INT animationMs);
	BOOL Remove(BOOL bRedraw);
	void AdjustItemRect(RECT *prcItem);
	void Draw();

	HWND GetHwnd() { return hwnd; }
	INT GetPosition() { return (iAfter + 1); }

	BOOL IsDisplayed();


protected:
	BOOL UpdatePosition(POINT pt);
	void PerformScroll(INT dy);
	BOOL StartAnimatedScroll(INT animationMs);
	void OnAnimationTimer(UINT_PTR idEvent);
	void DrawEx(LONG top, INT height, COLORREF rgb);


private:
	static void CALLBACK OnAnimationTimerReal(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
	UINT_PTR SetTimer(UINT_PTR idEvent, UINT uElapse);
	BOOL KillTimer(UINT_PTR idEvent);

protected:
	HWND hwnd;
	HDC hdc;
	INT iAfter;
	INT iCount;
	INT injectedCount;
	RECT clientRect;
    LONG activeTop;
	INT activeHeight;
	INT markHeight;
	COLORREF rgbBk;
	COLORREF rgbSelBk;
	COLORREF rgbActive;
	
	UINT_PTR timerId;
	UINT scrollAnimation;
	RECT scrollRect;
	RECT scrollClipRect;
	INT	scrollCounter;
	INT scrollDirection;
	LARGE_INTEGER scrollStarted;
};



#endif //NULLSOFT_DROPBOX_PLUGIN_LISTVIEW_SPACER_HEADER