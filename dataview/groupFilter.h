#ifndef _NULLSOFT_WINAMP_DATAVIEW_GROUP_FILTER_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_GROUP_FILTER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_viewfilter.h"
#include "./ifc_viewgroupfilter.h"
#include "./ifc_doublylinkednode.h"
#include "./ifc_viewfilterevent.h"
#include "./ifc_groupprovider.h"
#include "./filteredObjectList.h"
#include "./groupFilterObject.h"
#include "./groupFilterProvider.h"

#include "../nu/ptrlist.h"
#include "../nu/vector.h"

#include <bfc/multipatch.h>

#define MPIID_GF_VIEWFILTER			10
#define MPIID_GF_DOUBLYLINKEDNODE	20
#define MPIID_GF_VIEWGROUPFILTER	30
#define MPIID_GF_VIEWFILTEREVENT	40


class GroupFilter :	public MultiPatch<MPIID_GF_VIEWFILTER, ifc_viewfilter>,
					public MultiPatch<MPIID_GF_DOUBLYLINKEDNODE, ifc_doublylinkednode>,
					public MultiPatch<MPIID_GF_VIEWGROUPFILTER, ifc_viewgroupfilter>,
					public MultiPatch<MPIID_GF_VIEWFILTEREVENT, ifc_viewfilterevent>
{

protected:


protected:
	GroupFilter(GroupFilterProvider *provider);
	~GroupFilter();

public:
	static HRESULT CreateInstance(ifc_groupprovider *groupProvider, GroupFilter **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_viewfilter */
	const char *GetName();
	HRESULT Bind(ifc_dataprovider *provider);
	HRESULT Init(ifc_dataobjectlist *objectList); 
	HRESULT IsAllowed(size_t objectIndex);
	HRESULT Update();
	HRESULT RegisterFilterEventHandler(ifc_viewfilterevent *eventHandler);
	HRESULT UnregisterFilterEventHandler(ifc_viewfilterevent *eventHandler);
	
	/* ifc_doublylinkednode */
	HRESULT SetPrevious(ifc_doublylinkednode *node);
	HRESULT SetNext(ifc_doublylinkednode *node);
	HRESULT GetPrevious(ifc_doublylinkednode **node);
	HRESULT GetNext(ifc_doublylinkednode **node);

	/* ifc_viewgroupfilter */
	HRESULT GetProvider(ifc_dataprovider **provider);
	HRESULT GetObjects(ifc_dataobjectlist **list);
	HRESULT GetGroupProvider(ifc_groupprovider **groupProvider);
	HRESULT GetSummaryObject(ifc_dataobject **object);
	HRESULT UpdateSelection(ifc_viewselection *selection, ifc_viewselection *appended, ifc_viewselection *removed);
	size_t GetGroupId(size_t objectIndex, BOOL ignoreUnknownGroup);
	HRESULT EnableBypass(BOOL enable);
	HRESULT IsBypassEnabled();
	HRESULT RegisterGroupFilterEventHandler(ifc_viewgroupfilterevent *eventHandler);
	HRESULT UnregisterGroupFilterEventHandler(ifc_viewgroupfilterevent *eventHandler);

	/* ifc_viewfilterevent */
	void FilterEvent_BeginUpdate(ifc_viewfilter *instance);
	void FilterEvent_EndUpdate(ifc_viewfilter *instance);
	void FilterEvent_BlockAll(ifc_viewfilter *instance);
	void FilterEvent_ActionChanged(ifc_viewfilter *instance, const size_t *objectIndex, size_t count, ViewFilterAction action);

public:
	size_t GetPreviousFilters(ifc_viewfilter **buffer, size_t bufferSize);
	HRESULT IsAllowedByFilters(size_t objectIndex, ifc_viewfilter **filters, size_t filtersCount);

protected:
	HRESULT Group_AddObjectInfo(GroupFilterObject *group, size_t objectIndex);
	HRESULT Group_RemoveObjectInfo(GroupFilterObject *group, size_t objectIndex);
	HRESULT Group_IsSelected(size_t groupId);

	void Selection_Lock();
	void Selection_Unlock();
	HRESULT Selection_IsLocked();
	HRESULT Selection_RemoveAll();
	HRESULT Selection_AddGroup(size_t groupId);
	HRESULT Selection_RemoveGroup(size_t groupId);
	HRESULT Selection_Update(ifc_viewselection *selection, ViewFilterAction action);
	HRESULT Selection_UpdateAndNotify(ifc_viewselection *selection, ViewFilterAction action, ifc_viewfilter **previousFilters, size_t previousFiltersCount);
	HRESULT Selection_IsEmpty();

	void Notify_FilterBeginUpdate();
	void Notify_FilterEndUpdate();
	void Notify_FilterBlockAll();
	void Notify_FilterActionChanged(const size_t *objectIndex, size_t count, ViewFilterAction action);

	void Notify_BypassModeChanged();

protected:
	typedef nu::PtrList<ifc_viewfilterevent> FilterEventHandlerList;
	typedef nu::PtrList<ifc_viewgroupfilterevent> GroupFilterEventHandlerList;
	typedef Vector<BOOL> BoolList;
	typedef Vector<size_t> IndexList;
	
protected:
	size_t ref;
	char *name;
	GroupFilterProvider *provider;
	ifc_doublylinkednode *previous;
	ifc_doublylinkednode *next;
	ifc_viewgroupfilter *nextGroup;
	FilterEventHandlerList filterEventHandlerList;
	GroupFilterEventHandlerList groupFilterEventHandlerList;
	FilteredObjectList *groupList;
	size_t *indexMap, *indexSortMap, indexMapSize;
	size_t lengthValueId, sizeValueId;
	ifc_dataobjectlist *objectList;
	BOOL *groupSelectionMap;
	size_t groupSelectionCount;
	GroupFilterObject *summaryGroup;
	size_t selectionLock;
	BOOL bypassModeEnabled;

protected:
	RECVS_MULTIPATCH;
};


#endif //_NULLSOFT_WINAMP_DATAVIEW_GROUP_FILTER_HEADER