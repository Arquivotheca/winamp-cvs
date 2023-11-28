#ifndef _NULLSOFT_WINAMP_DATAVIEW_FILTERED_OBJECT_LIST_VIEW_FILTER_HANDLER_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_FILTERED_OBJECT_LIST_VIEW_FILTER_HANDLER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_viewfilterevent.h"
#include "./filteredObjectList.h"


class FilteredObjectListViewFilterHandler : public ifc_viewfilterevent
{

protected:
	FilteredObjectListViewFilterHandler(FilteredObjectList *objectList);
	~FilteredObjectListViewFilterHandler();

public:
	static HRESULT CreateInstance(FilteredObjectList *objectList, 
								  FilteredObjectListViewFilterHandler **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_viewfilterevent */
	void FilterEvent_BeginUpdate(ifc_viewfilter *instance);
	void FilterEvent_EndUpdate(ifc_viewfilter *instance);
	void FilterEvent_BlockAll(ifc_viewfilter *instance);
	void FilterEvent_ActionChanged(ifc_viewfilter *instance, const size_t *objectIndex, size_t count, ViewFilterAction action);
	
protected:
	size_t ref;
	FilteredObjectList *objectList;

protected:
	RECVS_DISPATCH;
};


#endif //_NULLSOFT_WINAMP_DATAVIEW_FILTERED_OBJECT_LIST_VIEW_FILTER_HANDLER_HEADER