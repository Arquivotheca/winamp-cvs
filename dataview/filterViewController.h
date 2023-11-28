#ifndef _NULLSOFT_WINAMP_DATAVIEW_FILTER_VIEW_CONTROLLER_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_FILTER_VIEW_CONTROLLER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_viewcontroller.h"
#include "./ifc_viewcontentsevent.h"
#include "./ifc_viewfilter.h"

#include <bfc/multipatch.h>

#define MPIID_FC_VIEWCONTROLLER			10
#define MPIID_FC_VIEWCONTENTSEVENT		20

class FilterViewController : public MultiPatch<MPIID_FC_VIEWCONTROLLER, ifc_viewcontroller>,
							 public MultiPatch<MPIID_FC_VIEWCONTENTSEVENT, ifc_viewcontentsevent>
{

protected:
	FilterViewController(ifc_viewfilter *filter, ifc_viewcontroller *controller);
	~FilterViewController();

public:
	static HRESULT CreateInstance(ifc_viewfilter *filter, 
								  ifc_viewcontroller *controller, 
								  FilterViewController **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_viewcontroller */
	void FreeString(char *string);
	HRESULT GetPrimaryColumn(ifc_viewcontents *contents, char **columnName);
	HRESULT GetDefaultSort(ifc_viewcontents *contents, char **columnName, BOOL *ascendingOrder);
	HRESULT GetDefaultView(ifc_viewcontents *contents, char **viewName);
	HRESULT GetDefaultColumns(ifc_viewwindow *window, char **columns);

	/* ifc_viewcontentsevent */
	void ContentsEvent_ObjectListChanged(ifc_viewcontents *contents, ifc_dataobjectlist *newObjects, ifc_dataobjectlist *prevObjects);
	void ContentsEvent_ObjectsAdded(ifc_viewcontents *contents, ifc_dataobjectlist *list, ifc_dataobject **added, size_t count, size_t startIndex);
	void ContentsEvent_ObjectsRemoved(ifc_viewcontents *contents, ifc_dataobjectlist *list, ifc_dataobject **removed, size_t count, size_t startIndex);
	void ContentsEvent_ObjectsRemovedAll(ifc_viewcontents *contents, ifc_dataobjectlist *list);
	void ContentsEvent_ObjectsChanged(ifc_viewcontents *contents, ifc_dataobjectlist *list, ifc_dataobject **changed, size_t count, size_t startIndex);
	void ContentsEvent_ObjectsUpdateStarted(ifc_viewcontents *contents,  ifc_dataobjectlist *list);
	void ContentsEvent_ObjectsUpdateFinished(ifc_viewcontents *contents, ifc_dataobjectlist *list);
	void ContentsEvent_SelectionChanged(ifc_viewcontents *contents, ifc_viewselection *selection, ifc_viewselection *appended, ifc_viewselection *removed, ifc_viewselectionevent::Reason reason);
	void ContentsEvent_ColumnsChanged(ifc_viewcontents *contents);

public:
	HRESULT GetGroupColumn(ifc_viewcontents *contents, ifc_viewcolumn **column);
	HRESULT GetViewController(ifc_viewcontroller **controller);
	

protected:
	size_t ref;
	ifc_viewcontroller *controller;
	ifc_viewfilter *filter;
	
protected:
	RECVS_MULTIPATCH;
};


#endif //_NULLSOFT_WINAMP_DATAVIEW_FILTER_VIEW_CONTROLLER_HEADER