#include "main.h"
#include "./filteredObjectList.h"
#include "./filteredObjectEnum.h"

typedef nu::PtrList<ifc_dataobject> ObjectList;

FilteredObjectList::FilteredObjectList()
	: ref(1), objectList(NULL)
{
	if (FAILED(DataObjectList::CreateInstance(&objectList)))
		objectList = NULL;
	else
	{
		if (SUCCEEDED(objectList->RegisterEventHandler(this)) &&
			2 == ref)
		{
			ref--;
		}

	}

}

FilteredObjectList::~FilteredObjectList()
{
	size_t index;

	index = eventHandlerList.size();
	while(index--)
	{
		eventHandlerList[index]->Release();
	}
	eventHandlerList.clear();

	if (NULL != objectList)
	{
		objectList->UnregisterEventHandler(this);
		objectList->Release();
	}
}

HRESULT FilteredObjectList::CreateInstance(FilteredObjectList **instance)
{
	if (NULL == instance) 
		return E_POINTER;

	*instance = NULL;

	*instance = new (std::nothrow) FilteredObjectList();
	if (NULL == *instance)
		return E_OUTOFMEMORY;

	return S_OK;
}


size_t FilteredObjectList::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t FilteredObjectList::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int FilteredObjectList::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_DataObjectList))
		*object = static_cast<ifc_dataobjectlist*>(this);
	else
	{
		*object = NULL;
		return E_NOINTERFACE;
	}

	if (NULL == *object)
		return E_UNEXPECTED;

	AddRef();
	return S_OK;
}

size_t FilteredObjectList::GetCount()
{
	return indexList.size();
}

ifc_dataobject *FilteredObjectList::GetItem(size_t index)
{
	if (index >= indexList.size())
		return NULL;
	
	index = indexList[index];
	return objectList->GetItem(index);
}

HRESULT FilteredObjectList::Enumerate(ifc_dataobjectenum **enumerator)
{
	return FilteredObjectEnum::CreateInstance(objectList->GetBegin(), indexList.begin(), indexList.size(), 
											  (FilteredObjectEnum**)enumerator);
}

size_t FilteredObjectList::Find(LCID localeId, ifc_dataobject *object)
{
	if (NULL != object)
	{
		size_t index, count;
		ifc_dataobject *object2;
				
		count = indexList.size();

		for (index = 0; index < count; index++)
		{
			object2 = objectList->GetItem(indexList[index]);
			if (object == object2 || 
				S_OK == object->IsEqual(localeId, object2))
			{
				return index;
			}
		}
	}

	return (size_t)-1;
}

HRESULT FilteredObjectList::RegisterEventHandler(ifc_dataobjectlistevent *eventHandler)
{
	size_t index;

	if (NULL == eventHandler)
		return E_INVALIDARG;

	index = eventHandlerList.size();
	while(index--)
	{
		if (eventHandler == eventHandlerList[index])
			return S_FALSE;
	}
	
	eventHandler->AddRef();
	eventHandlerList.push_back(eventHandler);
	
	return S_OK;
}

HRESULT FilteredObjectList::UnregisterEventHandler(ifc_dataobjectlistevent *eventHandler)
{
	size_t index;

	if (NULL == eventHandler)
		return E_INVALIDARG;

	index = eventHandlerList.size();
	while(index--)
	{
		if (eventHandler == eventHandlerList[index])
		{
			eventHandlerList.eraseindex(index);
			eventHandler->Release();
			return S_OK;
		}
	}
	
	return S_FALSE;
}

void FilteredObjectList::Objects_Clear()
{
	objectList->Clear();
}

size_t FilteredObjectList::Objects_GetCount()
{
	return objectList->GetCount();
}

size_t FilteredObjectList::Objects_GetRealIndex(size_t index)
{
	if (index >= indexList.size())
		return (size_t)-1;
	
	return indexList[index];
}

ifc_dataobject *FilteredObjectList::Objects_Get(size_t index)
{
	return objectList->GetItem(index);
}

void FilteredObjectList::Objects_Add(ifc_dataobject **objects, size_t count)
{
	objectList->Add(objects, count);
}

BOOL FilteredObjectList::Objects_Subtract(size_t index)
{
	return objectList->Remove(index);
}

ifc_dataobject **FilteredObjectList::Objects_GetBegin()
{
	return objectList->GetBegin();
}

HRESULT FilteredObjectList::Objects_GetList(ifc_dataobjectlist **_objectList)
{
	if (NULL == _objectList)
		return E_POINTER;

	*_objectList = objectList;
	
	if (NULL == objectList)
		return E_UNEXPECTED;

	objectList->AddRef();
	
	return S_OK;
}

HRESULT FilteredObjectList::Filter_AllowAll()
{
	size_t index, max, startIndex;
	ObjectList allowed;

	max = filterList.size();
	startIndex = indexList.size();

	if (max == startIndex)
		return S_FALSE;
	
	indexList.reserve(max);
	
	allowed.reserve(max - startIndex);
	
	for (index = 0; index < max; index++)
	{
		if (0 == filterList[index])
		{
			allowed.push_back(objectList->GetItem(index));
			
			indexList.push_back(index);
			filterList[index] = indexList.size();
		}
	}

	if (indexList.size() == startIndex)
		return S_FALSE;

	Notify_ObjectsAdded(allowed.begin(), allowed.size(), startIndex);
	return S_OK;
}

HRESULT FilteredObjectList::Filter_BlockAll()
{
	size_t count;
	
	count = indexList.size();
	if (0 == count)
		return S_FALSE;

	memset(filterList.begin(), 0, filterList.size() * sizeof(size_t));
	indexList.clear();

	Notify_ObjectsRemovedAll();
	return S_OK;
}

size_t FilteredObjectList::Filter_Allow(const size_t *objects, size_t objectsCount)
{
	size_t objectIndex, filterSize, startIndex;
	nu::PtrList<ifc_dataobject> allowedList;

	allowedList.reserve(objectsCount);

	startIndex = indexList.size();
	indexList.reserve(startIndex + objectsCount);

	filterSize = filterList.size();
	

	for(size_t i = 0; i < objectsCount; i++)
	{
		objectIndex = objects[i];

		if (objectIndex >= filterSize)
			continue;
	 
		if (0 == filterList[objectIndex])
		{
			indexList.push_back(objectIndex);
			filterList[objectIndex] = indexList.size();
			allowedList.push_back(objectList->GetItem(objectIndex));
		}
	}

	Notify_ObjectsAdded(allowedList.begin(), allowedList.size(), startIndex);

	return allowedList.size();
}

static int
FilteredObjectList_SortBlockCb(void *context, const void *element1, const void *element2)
{
	const size_t *orderList;

	orderList = (const size_t*)context;
	return orderList[*(size_t*)element1] - orderList[*(size_t*)element2];
}

size_t FilteredObjectList::Filter_Block(const size_t *objects, size_t objectsCount)
{
	size_t objectIndex;
	IndexList sortedList;
	nu::PtrList<ifc_dataobject> blockedList;
	size_t cursor, cursorVal, eraseIndex, patchLimit;
	
	sortedList.reserve(objectsCount);
	for (size_t i = 0; i < objectsCount; i++)
	{
		objectIndex = objects[i];
		if (objectIndex >= filterList.size())
			continue;
	 
		if (0 != filterList[objectIndex])
			sortedList.push_back(objectIndex);
	}

	if(sortedList.size() == indexList.size())
	{
		Filter_BlockAll();
		return sortedList.size();
	}

	if (0 == sortedList.size())
		return 0;

	qsort_s(sortedList.begin(), sortedList.size(), sizeof(size_t), FilteredObjectList_SortBlockCb, filterList.begin());
	
	cursor = sortedList.size() - 1;
	patchLimit = indexList.size();
	
	for(;;)
	{
		cursorVal = sortedList[cursor];
		blockedList.push_back(objectList->GetItem(cursorVal));
		eraseIndex = filterList[cursorVal] - 1;
		indexList.eraseAt(eraseIndex);
		filterList[cursorVal] = 0;
				
		if (0 == cursor || eraseIndex != filterList[sortedList[cursor - 1]])
		{
			//aTRACE_FMT("block notify objects removed: count(%u), start(%u)\r\n", blockedList.size(), eraseIndex);

			// patch fiterList
			patchLimit -= blockedList.size();
			for (size_t i = eraseIndex; i < patchLimit; i++)
			{
				size_t test  = indexList[i];
				filterList[indexList[i]] = (i - cursor) + 1;
			}
			
			Notify_ObjectsRemoved(blockedList.begin(), blockedList.size(), eraseIndex);

			if (0 == cursor)
				break;

			patchLimit = eraseIndex;
			blockedList.clear();
			
		}

		cursor--;
	}

	return sortedList.size();
}

HRESULT FilteredObjectList::Filter_IsAllowed(size_t objectIndex)
{
	if (objectIndex >= filterList.size())
		return E_INVALIDARG;

	return (0 != filterList[objectIndex]) ? S_OK : S_FALSE;
}

size_t FilteredObjectList::Filter_NotifyChange(const size_t *objectIndex, size_t count)
{
	size_t index, changed;

	changed = 0;

	while(count--)
	{
		index = *objectIndex;
		if (index >= filterList.size())
			continue;
	 
		if (0 == filterList[index])
			return S_FALSE;
	
		Notify_ObjectsChanged(objectList->GetBegin() + index, 1, filterList[index] - 1);
		changed++;
	}

	return changed;
}

void FilteredObjectList::StartUpdate()
{
	objectList->StartUpdate();
}

void FilteredObjectList::FinishUpdate()
{
	objectList->FinishUpdate();
}

void FilteredObjectList::Notify_ObjectsAdded(ifc_dataobject **added, size_t count, size_t startIndex)
{
	size_t index, max;

	max = eventHandlerList.size();
	for(index = 0; index < max; index++)
	{
		eventHandlerList[index]->ObjectListEvent_Added(this, added, count, startIndex);
	}
}

void FilteredObjectList::Notify_ObjectsRemoved(ifc_dataobject **removed, size_t count, size_t startIndex)
{
	size_t index, max;

	max = eventHandlerList.size();
	for(index = 0; index < max; index++)
	{
		eventHandlerList[index]->ObjectListEvent_Removed(this, removed, count, startIndex);
	}
}

void FilteredObjectList::Notify_ObjectsRemovedAll()
{
	size_t index, max;

	max = eventHandlerList.size();
	for(index = 0; index < max; index++)
	{
		eventHandlerList[index]->ObjectListEvent_RemovedAll(this);
	}
}

void FilteredObjectList::Notify_ObjectsChanged(ifc_dataobject **changed, size_t count, size_t startIndex)
{
	size_t index, max;

	max = eventHandlerList.size();
	for(index = 0; index < max; index++)
	{
		eventHandlerList[index]->ObjectListEvent_Changed(this, changed, count, startIndex);
	}
}

void FilteredObjectList::Notify_UpdateStarted()
{
	size_t index, max;

	max = eventHandlerList.size();
	for(index = 0; index < max; index++)
	{
		eventHandlerList[index]->ObjectListEvent_UpdateStarted(this);
	}
}

void FilteredObjectList::Notify_UpdateFinished()
{
	size_t index, max;

	max = eventHandlerList.size();
	for(index = 0; index < max; index++)
	{
		eventHandlerList[index]->ObjectListEvent_UpdateFinished(this);
	}
}

void FilteredObjectList::ObjectListEvent_Added(ifc_dataobjectlist *list, ifc_dataobject **added, size_t count, size_t startIndex)
{
	size_t index, indexOffset;

	filterList.reserve(filterList.size() + count);
	indexList.reserve(indexList.size() + count);
	
	indexOffset = indexList.size();
	
	for (index = 0; index < count; index++)
	{		
		indexList.push_back(startIndex + index);
		filterList.push_back(indexOffset + index + 1);
	}

	Notify_ObjectsAdded(added, count, indexOffset);
}

void FilteredObjectList::ObjectListEvent_Removed(ifc_dataobjectlist *list, ifc_dataobject **removed, size_t count, size_t startIndex)
{
	size_t index, listSize;
	ifc_dataobject *object;

	for (;startIndex < count; startIndex++)
	{
		
		if (0 != filterList[startIndex])
		{
			object = removed[startIndex];

			listSize = indexList.size();
			index = filterList[startIndex];

			for (;index < listSize; index++)
			{
				filterList[indexList[index]] = index;
			}

			index = filterList[startIndex] - 1;
			indexList.eraseAt(index);
		}
		else
			object = NULL;

		filterList.eraseAt(startIndex);

		if (NULL != object)
			Notify_ObjectsRemoved(&object, 1, index);
	}
}

void FilteredObjectList::ObjectListEvent_RemovedAll(ifc_dataobjectlist *list)
{
	indexList.clear();
	filterList.clear();
	Notify_ObjectsRemovedAll();
}

void FilteredObjectList::ObjectListEvent_Changed(ifc_dataobjectlist *list, ifc_dataobject **changed, size_t count, size_t startIndex)
{
	size_t index;
	ifc_dataobject *object;

	for (;startIndex < count; startIndex++)
	{
		if (0 != filterList[startIndex])
		{
			object = changed[startIndex];
			index = filterList[startIndex];
			Notify_ObjectsChanged(&object, 1, index);

		}
	}
}

void FilteredObjectList::ObjectListEvent_UpdateStarted(ifc_dataobjectlist *list)
{
	Notify_UpdateStarted();
}

void FilteredObjectList::ObjectListEvent_UpdateFinished(ifc_dataobjectlist *list)
{
	Notify_UpdateFinished();
}

#define CBCLASS FilteredObjectList
START_MULTIPATCH;
	START_PATCH(MPIID_FOL_DATAOBJECTLIST)
		M_CB(MPIID_FOL_DATAOBJECTLIST, ifc_dataobjectlist, ADDREF, AddRef);
		M_CB(MPIID_FOL_DATAOBJECTLIST, ifc_dataobjectlist, RELEASE, Release);
		M_CB(MPIID_FOL_DATAOBJECTLIST, ifc_dataobjectlist, QUERYINTERFACE, QueryInterface);
		M_CB(MPIID_FOL_DATAOBJECTLIST, ifc_dataobjectlist, API_GETCOUNT, GetCount)
		M_CB(MPIID_FOL_DATAOBJECTLIST, ifc_dataobjectlist, API_GETITEM, GetItem)
		M_CB(MPIID_FOL_DATAOBJECTLIST, ifc_dataobjectlist, API_ENUMERATE, Enumerate)
		M_CB(MPIID_FOL_DATAOBJECTLIST, ifc_dataobjectlist, API_FIND, Find)
		M_CB(MPIID_FOL_DATAOBJECTLIST, ifc_dataobjectlist, API_REGISTEREVENTHANDLER, RegisterEventHandler)
		M_CB(MPIID_FOL_DATAOBJECTLIST, ifc_dataobjectlist, API_UNREGISTEREVENTHANDLER, UnregisterEventHandler)
	NEXT_PATCH(MPIID_FOL_DATAOBJECTLISTEVENT)
		M_CB(MPIID_FOL_DATAOBJECTLISTEVENT, ifc_dataobjectlistevent, ADDREF, AddRef);
		M_CB(MPIID_FOL_DATAOBJECTLISTEVENT, ifc_dataobjectlistevent, RELEASE, Release);
		M_CB(MPIID_FOL_DATAOBJECTLISTEVENT, ifc_dataobjectlistevent, QUERYINTERFACE, QueryInterface);
		M_VCB(MPIID_FOL_DATAOBJECTLISTEVENT, ifc_dataobjectlistevent, API_OBJECTLISTEVENT_ADDDED, ObjectListEvent_Added);
		M_VCB(MPIID_FOL_DATAOBJECTLISTEVENT, ifc_dataobjectlistevent, API_OBJECTLISTEVENT_REMOVED, ObjectListEvent_Removed);
		M_VCB(MPIID_FOL_DATAOBJECTLISTEVENT, ifc_dataobjectlistevent, API_OBJECTLISTEVENT_REMOVEDALL, ObjectListEvent_RemovedAll);
		M_VCB(MPIID_FOL_DATAOBJECTLISTEVENT, ifc_dataobjectlistevent, API_OBJECTLISTEVENT_UPDATESTARTED, ObjectListEvent_UpdateStarted);
		M_VCB(MPIID_FOL_DATAOBJECTLISTEVENT, ifc_dataobjectlistevent, API_OBJECTLISTEVENT_UPDATEFINISHED, ObjectListEvent_UpdateFinished);
	END_PATCH
END_MULTIPATCH;