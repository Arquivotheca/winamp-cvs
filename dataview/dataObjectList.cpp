#include "main.h"
#include "./dataObjectList.h"
#include "./dataObjectEnum.h"

DataObjectList::DataObjectList()
	: ref(1), updatesLock(0)
{
}

DataObjectList::~DataObjectList()
{
	size_t index;

	index = eventHandlerList.size();
	while(index--)
	{
		eventHandlerList[index]->Release();
	}
	eventHandlerList.clear();

	index = objectList.size();
	while(index--)
	{
		objectList[index]->Release();
	}
}

HRESULT DataObjectList::CreateInstance(DataObjectList **instance)
{
	if (NULL == instance) 
		return E_POINTER;

	*instance = NULL;

	*instance = new (std::nothrow) DataObjectList();
	if (NULL == *instance)
		return E_OUTOFMEMORY;

	return S_OK;
}


size_t DataObjectList::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t DataObjectList::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int DataObjectList::QueryInterface(GUID interface_guid, void **object)
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

size_t DataObjectList::GetCount()
{
	return objectList.size();
}

ifc_dataobject *DataObjectList::GetItem(size_t index)
{
	if (index >= objectList.size())
		return NULL;

	return objectList[index];
}

HRESULT DataObjectList::Enumerate(ifc_dataobjectenum **enumerator)
{
	return DataObjectEnum::CreateInstance(objectList.begin(), objectList.size(), 
									  (DataObjectEnum**)enumerator);
}

size_t DataObjectList::Find(LCID localeId, ifc_dataobject *object)
{
	if (NULL != object)
	{
		size_t index, count;
		ifc_dataobject *object2;
				
		count = objectList.size();

		for (index = 0; index < count; index++)
		{
			object2 = objectList[index];
			if (object == object2 || 
				S_OK == object->IsEqual(localeId, object2))
			{
				return index;
			}
		}
	}

	return (size_t)-1;
}

HRESULT DataObjectList::RegisterEventHandler(ifc_dataobjectlistevent *eventHandler)
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

HRESULT DataObjectList::UnregisterEventHandler(ifc_dataobjectlistevent *eventHandler)
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

void DataObjectList::Clear()
{
	if (0 == objectList.size())
		return;
	
	objectList.clear();
	Notify_ObjectsRemovedAll();
}

void DataObjectList::Add(ifc_dataobject **objects, size_t count)
{
	size_t index, startIndex;
	ifc_dataobject *object;

	if (NULL == objects)
		return;

	objectList.reserve(objectList.size() + count);
	
	startIndex = objectList.size();

	for (index = 0; index < count; index++)
	{
		object = objects[index];
		if (NULL != object)
		{
			object->AddRef();
			objectList.push_back(object);
		}
	}

	if (startIndex < objectList.size())
		Notify_ObjectsAdded(objectList.begin() + startIndex, objectList.size() - startIndex, startIndex);
}

BOOL DataObjectList::Remove(size_t index)
{
	ifc_dataobject *object;

	if (index >= objectList.size())
		return FALSE;

	object = objectList[index];
	objectList.eraseindex(index);
	Notify_ObjectsRemoved(&object, 1, index);
	object->Release();

	return TRUE;
}

ifc_dataobject **DataObjectList::GetBegin()
{
	return objectList.begin();
}

void DataObjectList::Reserve(size_t size)
{
	objectList.reserve(size);
}

void DataObjectList::StartUpdate()
{
	if (1 == InterlockedIncrement(&updatesLock))
		Notify_UpdateStarted();
}

void DataObjectList::FinishUpdate()
{
	if (0 != updatesLock && 
		0 == InterlockedDecrement(&updatesLock))
	{
		Notify_UpdateFinished();
	}
}

void DataObjectList::NotifyChange(size_t startIndex, size_t count)
{
	if ((startIndex + count) >= objectList.size())
	{
		if (startIndex >= objectList.size())
			return;

		count = objectList.size() - startIndex;
	}

	Notify_ObjectsChanged(objectList.begin() + startIndex, count, startIndex);
}


void DataObjectList::Notify_ObjectsAdded(ifc_dataobject **added, size_t count, size_t startIndex)
{
	size_t index, max;

	max = eventHandlerList.size();
	for(index = 0; index < max; index++)
	{
		eventHandlerList[index]->ObjectListEvent_Added(this, added, count, startIndex);
	}
}

void DataObjectList::Notify_ObjectsRemoved(ifc_dataobject **removed, size_t count, size_t startIndex)
{
	size_t index, max;

	max = eventHandlerList.size();
	for(index = 0; index < max; index++)
	{
		eventHandlerList[index]->ObjectListEvent_Removed(this, removed, count, startIndex);
	}
}

void DataObjectList::Notify_ObjectsRemovedAll()
{
	size_t index, max;

	max = eventHandlerList.size();
	for(index = 0; index < max; index++)
	{
		eventHandlerList[index]->ObjectListEvent_RemovedAll(this);
	}
}

void DataObjectList::Notify_ObjectsChanged(ifc_dataobject **changed, size_t count, size_t startIndex)
{
	size_t index, max;

	max = eventHandlerList.size();
	for(index = 0; index < max; index++)
	{
		eventHandlerList[index]->ObjectListEvent_Changed(this, changed, count, startIndex);
	}
}

void DataObjectList::Notify_UpdateStarted()
{
	size_t index, max;

	max = eventHandlerList.size();
	for(index = 0; index < max; index++)
	{
		eventHandlerList[index]->ObjectListEvent_UpdateStarted(this);
	}
}

void DataObjectList::Notify_UpdateFinished()
{
	size_t index, max;

	max = eventHandlerList.size();
	for(index = 0; index < max; index++)
	{
		eventHandlerList[index]->ObjectListEvent_UpdateFinished(this);
	}
}


#define CBCLASS DataObjectList
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
CB(API_GETCOUNT, GetCount)
CB(API_GETITEM, GetItem)
CB(API_ENUMERATE, Enumerate)
CB(API_FIND, Find)
CB(API_REGISTEREVENTHANDLER, RegisterEventHandler)
CB(API_UNREGISTEREVENTHANDLER, UnregisterEventHandler)
END_DISPATCH;
#undef CBCLASS