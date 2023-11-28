#include "main.h"
#include "./groupFilterProvider.h"
#include "./groupFilterObject.h"

typedef struct ValueInfo
{
	const char *name;
	size_t id;

} ValueInfo;

static const ValueInfo valueInfoMap[] =
{
	/*
	*	!!! ATTENTION: LIST MUST BE SORTED !!!
	*/ 
	{ "Group",				GroupFilterObject::ValueId_Group },
	{ "Length",				GroupFilterObject::ValueId_Length },
	{ "NextFilterCount",	GroupFilterObject::ValueId_NextFilterCount },
	{ "Size",				GroupFilterObject::ValueId_Size },
	{ "TrackCount",			GroupFilterObject::ValueId_TrackCount },
};


static int
GroupFilterProvider_FindValueInfoCb(const void *target, const void *element)
{
	ValueInfo *valueInfo;
	valueInfo = (ValueInfo*)element;

	return CompareStringA(CSTR_INVARIANT, NORM_IGNORECASE, (const char*)target, -1, valueInfo->name, -1) - 2;
}

GroupFilterProvider::GroupFilterProvider(ifc_groupprovider *_groupProvider)
	: ref(1), groupProvider(_groupProvider), objectList(NULL), nextGroupProvider(NULL)
{
	if (NULL != groupProvider)
		groupProvider->AddRef();
}

GroupFilterProvider::~GroupFilterProvider()
{
	size_t index;
	
	index = eventHandlerList.size();
	while(index--)
	{
		eventHandlerList[index]->Release();
	}
	eventHandlerList.clear();

	SafeRelease(groupProvider);
	SafeRelease(objectList);
	SafeRelease(nextGroupProvider);
}

HRESULT GroupFilterProvider::CreateInstance(ifc_groupprovider *groupProvider, GroupFilterProvider **instance)
{
	if (NULL == instance) 
		return E_POINTER;

	*instance = NULL;

	if (NULL == groupProvider)
		return E_INVALIDARG;

	*instance = new (std::nothrow) GroupFilterProvider(groupProvider);
	if (NULL == *instance)
		return E_OUTOFMEMORY;

	return S_OK;
}

size_t GroupFilterProvider::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t GroupFilterProvider::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int GroupFilterProvider::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) 
		return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_DataProvider))
		*object = static_cast<ifc_dataprovider*>(this);
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

size_t GroupFilterProvider::ResolveName(const char *name)
{
	ValueInfo *valueInfo;

	if (NULL == name)
		return VALUEID_UNKNOWN;

	valueInfo = (ValueInfo*)bsearch(name, valueInfoMap, ARRAYSIZE(valueInfoMap), 
							sizeof(ValueInfo), GroupFilterProvider_FindValueInfoCb);

	if (NULL == valueInfo)
		return VALUEID_UNKNOWN;


	if (GroupFilterObject::ValueId_NextFilterCount == valueInfo->id)
	{
		if (NULL == nextGroupProvider)
			return VALUEID_UNKNOWN;
	}

	return valueInfo->id;
}

HRESULT GroupFilterProvider::ResolveNames(const char **names, size_t count, size_t *valueIds)
{	
	size_t index, valueId, unresolved;

	if (NULL == names || NULL == valueIds)
		return E_POINTER;

	unresolved = 0;

	for (index = 0; index < count; index++)
	{
		valueId = ResolveName(names[index]);
		if (VALUEID_UNKNOWN == valueId)
		{
			if (FAILED(groupProvider->ResolveNames(&names[index], 1, &valueId)) ||
				VALUEID_UNKNOWN == valueId)
			{
				valueId = VALUEID_UNKNOWN;
				unresolved++;
			}
		}

		valueIds[index] = valueId;
	}

	return (0 != unresolved) ? VALUE_E_UNKNOWNNAME : S_OK;
}

HRESULT GroupFilterProvider::SetObjects(ifc_dataobjectlist *_objectList)
{
	SafeRelease(objectList);
		
	objectList = _objectList;
	if (NULL != objectList)
		objectList->AddRef();

	return S_OK;
}


HRESULT GroupFilterProvider::Enumerate(ifc_dataobjectenum **enumerator)
{
	if (NULL == objectList)
		return E_FAIL;

	return objectList->Enumerate(enumerator);
}

HRESULT GroupFilterProvider::GetColumnDisplayName(const char *name, wchar_t *buffer, size_t bufferSize)
{
	if (0 == ColumnInfo_CompareNames(name, -1, "NextFilterCount", -1))
	{
		if (NULL == nextGroupProvider)
			return S_OK;

		return nextGroupProvider->GetCounterText(buffer, bufferSize);
	}
	return E_NOTIMPL;
}

HRESULT GroupFilterProvider::RegisterEventHandler(ifc_dataproviderevent *eventHandler)
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

HRESULT GroupFilterProvider::UnregisterEventHandler(ifc_dataproviderevent *eventHandler)
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

HRESULT GroupFilterProvider::GetGroupProvider(ifc_groupprovider **_groupProvider)
{
	if (NULL == _groupProvider)
		return E_POINTER;

	*_groupProvider = groupProvider;
	if (NULL != groupProvider)
		groupProvider->AddRef();

	return S_OK;
}

HRESULT GroupFilterProvider::SetNextFilter(ifc_groupprovider *groupProvider)
{
	if (groupProvider == nextGroupProvider)
		return S_OK;

	SafeRelease(nextGroupProvider);
		
	nextGroupProvider = groupProvider;
	if (NULL != nextGroupProvider)
		nextGroupProvider->AddRef();

	Notify_FieldsChanged();

	return S_OK;
}

void GroupFilterProvider::Notify_FieldsChanged()
{
	size_t index, count;

	count = eventHandlerList.size();
	for (index = 0; index < count; index++)
	{
		eventHandlerList[index]->DataProviderEvent_FieldsChanged(this);
	}
}

void GroupFilterProvider::Notify_ObjectsChanged(ifc_dataobject **objects, size_t count)
{
	size_t index, size;

	size = eventHandlerList.size();
	for (index = 0; index < size; index++)
	{
		eventHandlerList[index]->DataProviderEvent_ObjectsChanged(this, objects, count);
	}
}


#define CBCLASS GroupFilterProvider
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
CB(API_RESOLVENAMES, ResolveNames)
CB(API_ENUMERATE, Enumerate)
CB(API_GETCOLUMNDISPLAYNAME, GetColumnDisplayName)
CB(API_REGISTEREVENTHANDLER, RegisterEventHandler)
CB(API_UNREGISTEREVENTHANDLER, UnregisterEventHandler)
END_DISPATCH;
#undef CBCLASS
