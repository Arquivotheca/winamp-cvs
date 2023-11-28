#include "main.h"
#include "./selectionNotifier.h"
#include "./selection.h"


SelectionNotifier::SelectionNotifier()
{
	Selection::CreateInstance(&appended);
	Selection::CreateInstance(&removed);
}

SelectionNotifier::~SelectionNotifier()
{
	appended->Release();
	removed->Release();
}

HRESULT SelectionNotifier::RegisterHandler(ifc_viewselectionevent *eventHandler)
{
	size_t index;

	if (NULL == eventHandler)
		return E_INVALIDARG;

	index = handlers.size();
	while(index--)
	{
		if (eventHandler == handlers[index])
			return S_FALSE;
	}
	
	eventHandler->AddRef();
	handlers.push_back(eventHandler);
	
	return S_OK;
}

HRESULT SelectionNotifier::UnregisterHandler(ifc_viewselectionevent *eventHandler)
{
	size_t index;

	if (NULL == eventHandler)
		return E_INVALIDARG;

	index = handlers.size();
	while(index--)
	{
		if (eventHandler == handlers[index])
		{
			handlers.eraseindex(index);
			eventHandler->Release();
			return S_OK;
		}
	}
	
	return S_FALSE;
}

size_t SelectionNotifier::GetHandlerCount()
{
	return handlers.size();
}

Selection *SelectionNotifier::GetAppendedTracker()
{
	return appended;
}

Selection *SelectionNotifier::GetRemovedTracker()
{
	return removed;
}

void SelectionNotifier::Notify(Selection *instance, ifc_viewselectionevent::Reason reason)
{
	size_t index, count;
	Selection *appendedTracker, *removedTracker;

	appendedTracker = GetAppendedTracker();
	removedTracker = GetRemovedTracker();

	if (0 == appendedTracker->GetListSize() && 
		0 == removedTracker->GetListSize())
	{
		return;
	}

	count = handlers.size();
	for (index = 0; index < count; index++)
	{
		handlers[index]->SelectionEvent_Changed(instance, appendedTracker, removedTracker, reason);
	}	
}

void SelectionNotifier::ResetTrackers()
{
	appended->RemoveAll();
	removed->RemoveAll();
}

HRESULT SelectionNotifier::IsTrackersEmpty()
{
	Selection *tracker;

	tracker = GetAppendedTracker();
	if (0 == tracker->GetListSize())
	{
		tracker = GetRemovedTracker();
		if (0 == tracker->GetListSize())
			return S_OK;
	}

	return S_FALSE;
}