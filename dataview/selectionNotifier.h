#ifndef _NULLSOFT_WINAMP_DATAVIEW_VIEW_SELECTION_NOTIFIER_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_VIEW_SELECTION_NOTIFIER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "./ifc_viewselectionevent.h"
#include "../nu/ptrlist.h"

class Selection;

class SelectionNotifier
{
public:
	SelectionNotifier();
	~SelectionNotifier();

	HRESULT RegisterHandler(ifc_viewselectionevent *eventHandler);
	HRESULT UnregisterHandler(ifc_viewselectionevent *eventHandler);
	size_t GetHandlerCount();

	Selection *GetAppendedTracker();
	Selection *GetRemovedTracker();
	
	void Notify(Selection *instance, ifc_viewselectionevent::Reason reason);
	void ResetTrackers();
	HRESULT IsTrackersEmpty();

protected:
	typedef nu::PtrList<ifc_viewselectionevent> HandlerList;

protected:
	Selection *appended;
	Selection *removed;
	HandlerList handlers;
};


#endif //_NULLSOFT_WINAMP_DATAVIEW_VIEW_SELECTION_NOTIFIER_HEADER