#include "main.h"
#include "./headerCallback.h"
#include "./dropWindow.h"
#include "./document.h"
#include "./itemView.h"
#include "./toolbar.h"
#include "./toolbarCallback.h"
#include "./meterbar.h"
#include "./meterbarCallback.h"
#include "./resource.h"

#include "./playlistDropTarget.h"

#include "./skinWindow.h"

#define UPDATEMETERBAR_TIMER		89
#define UPDATEMETERBAR_DELAY		100

class HeaderToolbarCallback : public ToolbarCallback
{
public:
	HeaderToolbarCallback(HWND hHost) : hwnd(hHost) {}
	virtual ~HeaderToolbarCallback() {}

public:
	void OnDestroy(Toolbar *instance)
	{
		instance->RegisterCallback(NULL);
		delete(this);
	}

	void Invalidate(const RECT *prcInvalid)
	{
		InvalidateRect(hwnd, prcInvalid, FALSE);
	}

	INT TrackMouseLeave(Toolbar *instance)
	{
		TRACKMOUSEEVENT tm;
		tm.cbSize = sizeof(TRACKMOUSEEVENT);
		tm.dwFlags = TME_LEAVE;
		tm.hwndTrack = hwnd;
		return (::TrackMouseEvent(&tm)) ? ToolbarCallback::Success : ToolbarCallback::Failed;
	}

	void CancelTrackMouseLeave(Toolbar *instance)
	{
		TRACKMOUSEEVENT tm;
		tm.cbSize = sizeof(TRACKMOUSEEVENT);
		tm.dwFlags = TME_LEAVE | TME_CANCEL;
		tm.hwndTrack = hwnd;
		::TrackMouseEvent(&tm);
	}
	void ShowTip(LPCTSTR pszText, const RECT *prcBounds)
	{
		DropboxHeader_ShowTip(hwnd, pszText, prcBounds);
	}

	void OnCommand(Toolbar *instance, UINT commandId)
	{
		SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(commandId, 0), 0L);
	}
	HMENU ResolveMenuId(INT menuId)
	{
		HWND hParent = GetParent(hwnd);
		return (NULL != hParent) ? DropboxWindow_GetMenu(hParent, menuId) : NULL;
	}
	
	void ReleaseMenu(INT menuId, HMENU hMenu)
	{
		HWND hParent = GetParent(hwnd);
		if (NULL != hParent) 
			DropboxWindow_ReleaseMenu(hParent, menuId, hMenu);
	}

	INT TrackPopupMenuEx(HMENU hMenu, UINT menuFlags, INT x, INT y, LPTPMPARAMS lptpm)
	{
		POINT pt = {x, y};
		MapWindowPoints(hwnd, HWND_DESKTOP, &pt, 1);

		DropboxHeader_PopTip(hwnd);

		DWORD windowStyle = GetWindowStyle(hwnd);
		INT result = TrackPopup(hMenu, menuFlags, pt.x, pt.y, hwnd, (0 == (DBS_SKINWINDOW & windowStyle)));

		return result;
	}

protected:
	HWND hwnd;
};

class HeaderMeterbarCallback : public MeterbarCallback
{
public:
	HeaderMeterbarCallback(HWND hHost) : hwnd(hHost) {}
	virtual ~HeaderMeterbarCallback() {}

public:
	void OnDestroy(Meterbar *instance)
	{
		instance->RegisterCallback(NULL);
		delete(this);
	}
	void Invalidate(const RECT *prcInvalid)
	{
		InvalidateRect(hwnd, prcInvalid, FALSE);
	}

	void ShowTip(LPCTSTR pszText, const RECT *prcBounds)
	{
		DropboxHeader_ShowTip(hwnd, pszText, prcBounds);
	}

	Document *GetDocument() 
	{ 
		HWND hParent = GetParent(hwnd);
		return (NULL != hParent) ? DropboxWindow_GetActiveDocument(hParent) : NULL; 
	}

	DropboxView *GetView()  
	{ 
		HWND hParent = GetParent(hwnd);
		HWND hView = (NULL != hParent) ? DropboxWindow_GetActiveView(hParent) : NULL;
		return  (NULL != hView) ? DropBox_GetItemView(hView) : NULL;
	}

	void MetricsInvalid()
	{
		SetTimer(hwnd, UPDATEMETERBAR_TIMER, UPDATEMETERBAR_DELAY, UpdateMeterbarTimerElapsed);
	}
private:
	static void CALLBACK UpdateMeterbarTimerElapsed(HWND hwnd, UINT uMsg, UINT_PTR eventId, DWORD dwTime)
	{
		KillTimer(hwnd, eventId);
		DropboxHeader_UpdateMetrics(hwnd);
	}
protected:
	HWND hwnd;
};

class HeaderPlDropCallback : public PlaylistDropTargetCallback
{
public:
	HeaderPlDropCallback(HWND hHost) : hwnd(hHost), overlaySet(FALSE) {}
	~HeaderPlDropCallback() 
	{
	}

public:
	void OnDestroy(PlaylistDropTarget *instance)
	{
		instance->RegisterCallback(NULL);
		delete(this);
	}
	BOOL DragEnter(PlaylistDropTarget *instance, POINT pt, UINT keyState)
	{
		if (!GetDropAllowed())
			return FALSE;
		
		return TRUE;
	}
	BOOL DragOver(PlaylistDropTarget *instance, POINT pt, UINT keyState)
	{
		MapWindowPoints(HWND_DESKTOP, hwnd, &pt, 1);
		
		if (!GetDropAllowed() || 
			DBHP_MENU != DropboxHeader_HitTest(hwnd, pt.x, pt.y))
		{
			if (overlaySet)
			{
				DropboxHeader_RemoveMenuOverlay(hwnd);
				overlaySet = FALSE;
			}
			return FALSE;
		}
		
		if (!overlaySet)
		{
			OVERLAYINFO oi;
			ZeroMemory(&oi, sizeof(OVERLAYINFO));
			oi.hInstance = plugin.hDllInstance;
			oi.pszImage = MAKEINTRESOURCE(IDR_OPENPLAYLIST_IMAGE);
			overlaySet = DropboxHeader_SetMenuOverlay(hwnd, &oi);		
		}
		return TRUE;
	}

	void DragLeave(PlaylistDropTarget *instance)
	{
		if (overlaySet)
		{
			DropboxHeader_RemoveMenuOverlay(hwnd);
			overlaySet = FALSE;
		}
	}

	BOOL Drop(PlaylistDropTarget *instance, POINT pt, UINT keyState)
	{
		if (overlaySet)
		{
			DropboxHeader_RemoveMenuOverlay(hwnd);
			overlaySet = FALSE;
		}

		if (!GetDropAllowed())
			return FALSE;

		MapWindowPoints(HWND_DESKTOP, hwnd, &pt, 1);
		if (DBHP_MENU != DropboxHeader_HitTest(hwnd, pt.x, pt.y))
			return FALSE;

		return TRUE;
	}

protected:
	BOOL GetDropAllowed()
	{
		HWND hParent = GetParent(hwnd);
		Document *document = (NULL != hParent) ? DropboxWindow_GetActiveDocument(hParent) : NULL; 
		return (NULL != document && !document->QueryAsyncOpInfo(NULL));
	}
protected:
	HWND hwnd;
	BOOL overlaySet;
};

BOOL DropboxHeader_RegisterMeterbarCallback(HWND hwnd, Meterbar *instance)
{
	if (NULL == instance)
		return FALSE;
	HeaderMeterbarCallback *callback = new HeaderMeterbarCallback(hwnd);
	if (NULL == callback)
		return FALSE;
	instance->RegisterCallback(callback);
	return TRUE;
}

BOOL DropboxHeader_RegisterToolbarCallback(HWND hwnd, Toolbar *instance)
{
	if (NULL == instance)
		return FALSE;
	HeaderToolbarCallback *callback = new HeaderToolbarCallback(hwnd);
	if (NULL == callback)
		return FALSE;
	instance->RegisterCallback(callback);
	return TRUE;
}

BOOL DropboxHeader_RegisterPlDropCallback(HWND hwnd, PlaylistDropTarget *instance)
{
	if (NULL == instance)
		return FALSE;
	HeaderPlDropCallback *callback = new HeaderPlDropCallback(hwnd);
	if (NULL == callback)
		return FALSE;
	instance->RegisterCallback(callback);
	return TRUE;
}

