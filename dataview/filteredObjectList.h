#ifndef _NULLSOFT_WINAMP_DATAVIEW_FILTERED_OBJECT_LIST_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_FILTERED_OBJECT_LIST_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_dataobject.h"
#include "./ifc_dataobjectlist.h"

#include "./dataObjectList.h"
#include "../nu/ptrlist.h"
#include "../nu/vector.h"

#include <bfc/multipatch.h>

#define MPIID_FOL_DATAOBJECTLIST		10
#define MPIID_FOL_DATAOBJECTLISTEVENT	20

class FilteredObjectList : public MultiPatch<MPIID_FOL_DATAOBJECTLIST, ifc_dataobjectlist>,
						   public MultiPatch<MPIID_FOL_DATAOBJECTLISTEVENT, ifc_dataobjectlistevent>
					
{
	
protected:
	FilteredObjectList();
	~FilteredObjectList();

public:
	static HRESULT CreateInstance(FilteredObjectList **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_dataobjectlist */
	size_t GetCount();
	ifc_dataobject *GetItem(size_t index);
	HRESULT Enumerate(ifc_dataobjectenum **enumerator);
	size_t Find(LCID localeId, ifc_dataobject *object);
	HRESULT RegisterEventHandler(ifc_dataobjectlistevent *eventHandler);
	HRESULT UnregisterEventHandler(ifc_dataobjectlistevent *eventHandler);

public:
	// direct objects access
	void Objects_Clear();
	size_t Objects_GetCount();
	size_t Objects_GetRealIndex(size_t index);
	ifc_dataobject *Objects_Get(size_t index);
	void Objects_Add(ifc_dataobject **objects, size_t count);
	BOOL Objects_Subtract(size_t index);
	ifc_dataobject **Objects_GetBegin();
	HRESULT Objects_GetList(ifc_dataobjectlist **objectList);

	// filter access
	HRESULT Filter_AllowAll();
	HRESULT Filter_BlockAll();
	size_t Filter_Allow(const size_t *objects, size_t count);
	size_t Filter_Block(const size_t *objects, size_t count);
	HRESULT Filter_IsAllowed(size_t objectIndex);
	size_t Filter_NotifyChange(const size_t *objectIndex, size_t count);

	void StartUpdate();
	void FinishUpdate();

protected:
	void Notify_ObjectsAdded(ifc_dataobject **added, size_t count, size_t startIndex);
	void Notify_ObjectsRemoved(ifc_dataobject **removed, size_t count, size_t startIndex);
	void Notify_ObjectsRemovedAll();
	void Notify_ObjectsChanged(ifc_dataobject **changed, size_t count, size_t startIndex);
	void Notify_UpdateStarted();
	void Notify_UpdateFinished();

	/* ifc_dataobjectlistevent */
	void ObjectListEvent_Added(ifc_dataobjectlist *list, ifc_dataobject **added, size_t count, size_t startIndex);
	void ObjectListEvent_Removed(ifc_dataobjectlist *list, ifc_dataobject **removed, size_t count, size_t startIndex);
	void ObjectListEvent_RemovedAll(ifc_dataobjectlist *list);
	void ObjectListEvent_Changed(ifc_dataobjectlist *list, ifc_dataobject **changed, size_t count, size_t startIndex);
	void ObjectListEvent_UpdateStarted(ifc_dataobjectlist *list);
	void ObjectListEvent_UpdateFinished(ifc_dataobjectlist *list);

protected:
	typedef nu::PtrList<ifc_dataobjectlistevent> EventHandlerList;
	typedef Vector<size_t> IndexList;

protected:
	size_t ref;
	DataObjectList *objectList;
	IndexList filterList;
	IndexList indexList;
	EventHandlerList eventHandlerList;
	

protected:
	RECVS_MULTIPATCH;
};

#endif //_NULLSOFT_WINAMP_DATAVIEW_FILTERED_OBJECT_LIST_HEADER