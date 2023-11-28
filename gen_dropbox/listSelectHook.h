#ifndef NULLSOFT_DROPBOX_PLUGIN_LISTSELECT_HOOK_HEADER
#define NULLSOFT_DROPBOX_PLUGIN_LISTSELECT_HOOK_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./mouseHook.h"

class ListViewSelectionHook : public MouseHook
{

protected:
	ListViewSelectionHook(HWND hList, POINT *ppt);
	virtual ~ListViewSelectionHook();

public:
	static MouseHook *CreateHook(HWND hList);

protected:
	virtual LRESULT HookProc(int nCode, MSG *pMsg);
	void OnMouseMove(UINT uFlags, POINTS pts);
	void OnScrollTimer(UINT_PTR idEvent);
	void UpdateSelection(LONG y, BOOL bInvertMode, BOOL *startTimer);
	void FinishScroll();
	BOOL SmoothScroll(INT dy);
private:
	POINT startPoint;
	INT startItem;
	INT currentItem;
	BOOL timerActivated;
	BOOL bDestroying;
	INT itemHeight;
	INT headerHeight;
	RECT clientRect;
	BOOL focusVisible;
	HBITMAP hbmpBk;
	HRGN rgnScrollbar;
	HRGN rgnInvalid;
	
};

#endif // NULLSOFT_DROPBOX_PLUGIN_LISTSELECT_HOOK_HEADER
