#ifndef _NULLSOFT_WINAMP_DATAVIEW_VIEW_CONTENTS_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_VIEW_CONTENTS_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "./ifc_viewcontents.h"
#include "./ifc_viewcontroller.h"
#include "./ifc_dataprovider.h"
#include "./ifc_dataobjectlist.h"
#include "./ifc_dataobjectlistevent.h"

#include "./viewColumn.h"
#include "./selection.h"
#include "../nu/ptrlist.h"

#include <bfc/multipatch.h>

#define MPIID_CONTENTS_VIEWCONTENTS			10
#define MPIID_CONTENTS_DATAOBJECTLISTEVENT	20
#define MPIID_CONTENTS_VIEWSELECTIONEVENT	30
#define MPIID_CONTENTS_DATAPROVIDEREVENT	40

class ViewContents : public MultiPatch<MPIID_CONTENTS_VIEWCONTENTS, ifc_viewcontents>,
					 public MultiPatch<MPIID_CONTENTS_DATAOBJECTLISTEVENT, ifc_dataobjectlistevent>,
					 public MultiPatch<MPIID_CONTENTS_VIEWSELECTIONEVENT, ifc_viewselectionevent>,
					 public MultiPatch<MPIID_CONTENTS_DATAPROVIDEREVENT, ifc_dataproviderevent>
{

protected:
	ViewContents(const char *name, ifc_dataprovider *provider, ifc_viewconfig *config, ifc_viewcontroller *controller, ifc_viewfilter *filter);
	~ViewContents();

public:
	static HRESULT CreateInstance(const char *name, 
								  ifc_dataprovider *provider, 
								  ifc_viewconfig *config,
								  ifc_viewcontroller *controller,
								  ifc_viewfilter *filter,
								  ViewContents **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_viewcontents */
	const char *GetName();
	HRESULT GetProvider(ifc_dataprovider **provider);
	HRESULT GetConfig(ifc_viewconfig **config);
	HRESULT GetController(ifc_viewcontroller **controller);
	HRESULT GetSelection(ifc_dataobjectenum **enumerator);
	
	HRESULT EnumerateColumns(ifc_viewcolumnenum **enumerator);
	HRESULT FindColumn(const char *name, ifc_viewcolumn **column);
	HRESULT GetPrimaryColumn(ifc_viewcolumn **column);
	HRESULT GetSortColumn(ifc_viewcolumn **column);
	HRESULT SetSortColumn(const char *columnName);
	HRESULT GetSortOrder(SortOrder *order);
	HRESULT SetSortOrder(SortOrder order);

	HRESULT GetObjects(ifc_dataobjectlist **list);
	HRESULT GetSelectionTracker(ifc_viewselection **tracker);
	HRESULT GetFilter(ifc_viewfilter **filter);

	HRESULT RegisterEventHandler(ifc_viewcontentsevent *eventHandler);
	HRESULT UnregisterEventHandler(ifc_viewcontentsevent *eventHandler);


public:
	HRESULT AttachObjects(ifc_dataobjectlist *list);
	HRESULT Destroy();
	

protected:
	/* ifc_viewselectionevent */
	void SelectionEvent_Changed(ifc_viewselection *instance, ifc_viewselection *appended, ifc_viewselection *removed, Reason reason);

	/* ifc_dataobjectlistevent */
	void ObjectListEvent_Added(ifc_dataobjectlist *list, ifc_dataobject **added, size_t count, size_t startIndex);
	void ObjectListEvent_Removed(ifc_dataobjectlist *list, ifc_dataobject **removed, size_t count, size_t startIndex);
	void ObjectListEvent_RemovedAll(ifc_dataobjectlist *list);
	void ObjectListEvent_Changed(ifc_dataobjectlist *list, ifc_dataobject **changed, size_t count, size_t startIndex);
	void ObjectListEvent_UpdateStarted(ifc_dataobjectlist *list);
	void ObjectListEvent_UpdateFinished(ifc_dataobjectlist *list);

	/* ifc_dataproviderevent */
	void DataProviderEvent_FieldsChanged(ifc_dataprovider *instance);
	void DataProviderEvent_ObjectsChanged(ifc_dataprovider *instance, ifc_dataobject **objects, size_t count);


private:
	HRESULT Bind();
	void Columns_RemoveAll();
	
	void Notify_ObjectListChanged(ifc_dataobjectlist *newObjects, ifc_dataobjectlist *prevObjects);
	void Notify_ObjectsAdded(ifc_dataobjectlist *list, ifc_dataobject **added, size_t count, size_t startIndex);
	void Notify_ObjectsRemoved(ifc_dataobjectlist *list, ifc_dataobject **removed, size_t count, size_t startIndex);
	void Notify_ObjectsRemovedAll(ifc_dataobjectlist *list);
	void Notify_ObjectsChanged(ifc_dataobjectlist *list, ifc_dataobject **changed, size_t count, size_t startIndex);
	void Notify_ObjectsUpdateStarted(ifc_dataobjectlist *list);
	void Notify_ObjectsUpdateFinished(ifc_dataobjectlist *list);
	void Notify_SelectionChanged(ifc_viewselection *selection, ifc_viewselection *appended, ifc_viewselection *removed, Reason reason);
	void Notify_ColumnsChanged();

public:
	typedef nu::PtrList<ifc_viewcolumn> ColumnList;
	typedef nu::PtrList<ifc_viewcontentsevent> EventHandlerList;

protected:
	size_t ref;
	char *name;
	ifc_dataprovider *provider;
	ifc_viewconfig *config;
	ifc_viewcontroller *controller;
	ifc_viewcolumn *primaryColumn;
	ifc_viewcolumn *sortColumn;
	SortOrder sortOrder;
	ColumnList columnList;
	ifc_dataobjectlist *objectList;
	Selection *selection;
	ifc_viewfilter *filter;
	EventHandlerList eventHandlerList;
	
/*
		take data provider
		bind all registered columns and create them -> if nothing binded - fail to create
		
		store primary column;

		ifc_controller->RegisterViewSettigs(ifc_view *view, default columns layout);
		

		// per contents
		   primary column (title)
		   view name (icon);
		   sort (album,a);
		
		// per view
		   zoom level
		   show horz scroll (yes/no)
		   show summary (yes/no)
		   columns(album:50pt,h|artist:50pt)
*/

protected:
	RECVS_MULTIPATCH;
};

#endif //_NULLSOFT_WINAMP_DATAVIEW_VIEW_CONTENTS_HEADER