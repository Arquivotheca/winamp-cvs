#ifndef _NULLSOFT_WINAMP_DATAVIEW_DATA_OBJECT_LIST_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_DATA_OBJECT_LIST_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_dataobject.h"
#include "./ifc_dataobjectlist.h"

#include "../nu/ptrlist.h"

class DataObjectList : public ifc_dataobjectlist
{
	
protected:
	DataObjectList();
	~DataObjectList();

public:
	static HRESULT CreateInstance(DataObjectList **instance);

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
	void Clear();
	void Add(ifc_dataobject **objects, size_t count);
	BOOL Remove(size_t index);
	ifc_dataobject **GetBegin();
	void Reserve(size_t size);

	void StartUpdate();
	void FinishUpdate();

	void NotifyChange(size_t startIndex, size_t count);

protected:
	void Notify_ObjectsAdded(ifc_dataobject **added, size_t count, size_t startIndex);
	void Notify_ObjectsRemoved(ifc_dataobject **removed, size_t count, size_t startIndex);
	void Notify_ObjectsRemovedAll();
	void Notify_ObjectsChanged(ifc_dataobject **changed, size_t count, size_t startIndex);
	void Notify_UpdateStarted();
	void Notify_UpdateFinished();

protected:
	typedef nu::PtrList<ifc_dataobjectlistevent> EventHandlerList;
	typedef nu::PtrList<ifc_dataobject> ObjectList;
	
protected:
	size_t ref;
	size_t updatesLock;
	ObjectList objectList;
	EventHandlerList eventHandlerList;

protected:
	RECVS_DISPATCH;
};

#endif //_NULLSOFT_WINAMP_DATAVIEW_DATA_OBJECT_LIST_HEADER