#include "main.h"
#include "./groupFilter.h"
#include "./hashTypes.h"


#include <strsafe.h>


typedef struct _SortObjectsParam
{
	ifc_groupprovider *groupProvider;
	ifc_dataobjectlist *objectList;
	LCID localeId;
} SortObjectsParam;

GroupFilter::GroupFilter(GroupFilterProvider *_provider)
	: ref(1), name(NULL), previous(NULL), next(NULL), nextGroup(NULL),
	  provider(_provider), groupList(NULL),
	  indexMap(NULL), indexMapSize(0), indexSortMap(NULL), objectList(NULL), 
	  groupSelectionMap(NULL), groupSelectionCount(0),
	  lengthValueId(VALUEID_UNKNOWN), sizeValueId(VALUEID_UNKNOWN),
	  summaryGroup(NULL), selectionLock(0), bypassModeEnabled(TRUE)
{
	char buffer[256];
	const char *groupName;
	ifc_groupprovider *groupProvider;

	provider->AddRef();
	groupName = NULL;

	if (SUCCEEDED(provider->GetGroupProvider(&groupProvider)))
	{
		ifc_dataobject *summary;

		groupName = groupProvider->GetName();
		
		if (SUCCEEDED(groupProvider->CreateSummaryGroup(&summary)))
		{
			if (FAILED(GroupFilterObject::CreateInstance(summary, &summaryGroup)))
				summaryGroup = NULL;

			summary->Release();
		}

		groupProvider->Release();
	}

	

	if (FALSE != IS_STRING_EMPTY(groupName))
		groupName = "Group";
	
	
	StringCchPrintfA(buffer, ARRAYSIZE(buffer), "%sFilter", groupName);
	name = AnsiString_Duplicate(buffer);
	
}

GroupFilter::~GroupFilter()
{
	size_t index;
	
	index = filterEventHandlerList.size();
	while(index--)
	{
		filterEventHandlerList[index]->Release();
	}
	filterEventHandlerList.clear();

	index = groupFilterEventHandlerList.size();
	while(index--)
	{
		groupFilterEventHandlerList[index]->Release();
	}
	groupFilterEventHandlerList.clear();

	SafeRelease(nextGroup);
	SafeRelease(next);
	
	if (NULL != previous)
	{
		ifc_viewfilter *filter;
		if (SUCCEEDED(previous->QueryInterface(IFC_ViewFilter, (void**)&filter)))
		{
			filter->UnregisterEventHandler(this);
			filter->Release();
		}
		previous->Release();
	}
	
	SafeRelease(groupList);
	SafeRelease(provider);

	free(indexMap);
	free(indexSortMap);
	free(groupSelectionMap);

	SafeRelease(objectList);
	SafeRelease(summaryGroup);

	AnsiString_Free(name);
}

HRESULT GroupFilter::CreateInstance(ifc_groupprovider *groupProvider, GroupFilter **instance)
{
	GroupFilter *self;
	GroupFilterProvider *provider;

	if (NULL == instance)
		return E_POINTER;

	*instance = NULL;
	
	if (FAILED(GroupFilterProvider::CreateInstance(groupProvider, &provider)))
		return E_INVALIDARG;

	self = new (std::nothrow) GroupFilter(provider);
	if (NULL == self)
		return E_OUTOFMEMORY;

	*instance = self;
	return S_OK;
}


size_t GroupFilter::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t GroupFilter::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int GroupFilter::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) 
		return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_ViewFilter))
		*object = static_cast<ifc_viewfilter*>(this);
	else if (IsEqualIID(interface_guid, IFC_DoublyLinkedNode))
		*object = static_cast<ifc_doublylinkednode*>(this);
	else if (IsEqualIID(interface_guid, IFC_ViewGroupFilter))
		*object = static_cast<ifc_viewgroupfilter*>(this);
	else if (IsEqualIID(interface_guid, IFC_ViewFilterEvent))
		*object = static_cast<ifc_viewfilterevent*>(this);
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

const char *GroupFilter::GetName()
{	
	return name;
}

HRESULT GroupFilter::Bind(ifc_dataprovider *_provider)
{	
	HRESULT hr;
	ifc_groupprovider *groupProvider;

	if (NULL == groupList)
	{
		hr = FilteredObjectList::CreateInstance(&groupList);
		if (FAILED(hr))
			return hr;
	}

	hr = provider->GetGroupProvider(&groupProvider);
	if (SUCCEEDED(hr))
	{
		hr = groupProvider->Bind(_provider);
		groupProvider->Release();
	}

	lengthValueId = VALUEID_UNKNOWN;
	sizeValueId = VALUEID_UNKNOWN;

	if (SUCCEEDED(hr))
	{
		const char *valueNames[] = {"Length", "Size"};
		size_t valueIds[ARRAYSIZE(valueNames)];
		if (SUCCEEDED(_provider->ResolveNames(valueNames, ARRAYSIZE(valueNames), valueIds)))
		{
			lengthValueId = valueIds[0];
			sizeValueId = valueIds[1];
		}

	}
	return hr;

}

static int __cdecl
GroupFilter_SortObjectsCb(void *context, const void *elem1, const void *elem2)
{
	SortObjectsParam *param;
	param = (SortObjectsParam*)context;

	return param->groupProvider->CompareObjects(param->localeId, 
												param->objectList->GetItem(*(size_t*)elem1), 
												param->objectList->GetItem(*(size_t*)elem2)) - 2;
}

HRESULT GroupFilter::Init(ifc_dataobjectlist *_objectList)
{	
	HRESULT hr;
	LCID localeId;
	ifc_groupprovider *groupProvider;
	size_t index, bufferCount, objectListSize, objectId;
	ifc_dataobject *group, *object, *object2;
	GroupFilterObject *filterObject, **buffer;
		
	SafeRelease(objectList);
	
	objectList = _objectList;
	if (NULL != objectList)
	{
		objectList->AddRef();
		objectListSize = objectList->GetCount();
	}
	else
		objectListSize = 0;

	if (NULL != indexMap)
	{
		free(indexMap);
		indexMap = NULL;
	}

	if (NULL != indexSortMap)
	{
		free(indexSortMap);
		indexSortMap = NULL;
	}

	if (NULL != groupSelectionMap)
	{
		free(groupSelectionMap);
		groupSelectionMap = NULL;
	}

	groupSelectionCount = 0;

	indexMapSize = 0;
	
	if (NULL == groupList)
		return E_UNEXPECTED;
	
	Selection_Lock();
	groupList->Objects_Clear();
	Selection_Unlock();
			
	if (0 == objectListSize)
		return S_OK;
		
	hr = provider->GetGroupProvider(&groupProvider);
	if (FAILED(hr))
		return hr;

	localeId = Plugin_GetUserLocaleId();

	bufferCount = 0;
	buffer = (GroupFilterObject**)malloc(sizeof(GroupFilterObject*) * objectListSize);

	indexMap = (size_t*)malloc(sizeof(size_t) * objectListSize);
	indexSortMap = (size_t*)malloc(sizeof(size_t) * objectListSize);
	
	if (NULL == buffer || NULL == indexMap || NULL == indexSortMap)
	{
		free(indexMap);
		free(indexSortMap);
		free(buffer);
		hr = E_OUTOFMEMORY;
	}
	else
	{
		SortObjectsParam param;

		indexMapSize = objectListSize;

		for (index = 0; index < indexMapSize; index++)
			indexSortMap[index] = index;
				
		param.groupProvider = groupProvider;
		param.objectList = objectList;
		param.localeId = localeId;
		
		qsort_s(indexSortMap, indexMapSize, sizeof(size_t), GroupFilter_SortObjectsCb, &param);
		
		for (index = 0; index < indexMapSize; index++)
		{
			objectId = indexSortMap[index];
			object = objectList->GetItem(objectId);
			if (SUCCEEDED(groupProvider->CreateGroup(localeId, object, &group)))
			{
				if (SUCCEEDED(GroupFilterObject::CreateInstance(group, &filterObject)))
				{
					buffer[bufferCount] = filterObject;
					indexMap[objectId] = bufferCount;

					filterObject->SearchIndex_Set(index);

					for (++index; index < indexMapSize; index++)
					{
						object2 = objectList->GetItem(indexSortMap[index]);
						if (COBJ_EQUAL != groupProvider->CompareObjects(localeId, object, object2))
						{
							index--;
							break;
						}

						indexMap[indexSortMap[index]] = bufferCount;

					}
					bufferCount++;
				}
				group->Release();
			}
		}
		
		Selection_Lock();
		if (0 != bufferCount)
		{
			groupList->Objects_Add((ifc_dataobject**)buffer, bufferCount);
			while(bufferCount--)
			{
				buffer[bufferCount]->Release();
			}
		}
		Selection_Unlock();

		groupSelectionMap = (BOOL*)malloc(sizeof(BOOL) * groupList->Objects_GetCount());
		if (NULL != groupSelectionMap)
			memset(groupSelectionMap, 0, sizeof(BOOL) * groupList->Objects_GetCount());

		free(buffer);
	}

	groupProvider->Release();

	return hr;
}

HRESULT GroupFilter::Update()
{
	size_t index, groupId;
	GroupFilterObject *group;
	
	if (NULL != summaryGroup)
		summaryGroup->Reset();

	groupId = groupList->GetCount();
	while(groupId--)
	{
		group = (GroupFilterObject*)groupList->Objects_Get(groupId);
		group->Reset();
	}
		
	index = objectList->GetCount();
	while(index--)
	{
		groupId = indexMap[index];
		group = (GroupFilterObject*)groupList->Objects_Get(groupId);
		if (S_OK == groupList->Filter_IsAllowed(groupId))
		{
			Group_AddObjectInfo(group, index);
		}
	}

	if (NULL != summaryGroup)
	{
		groupId = groupList->GetCount();
		while(groupId--)
		{			
			group = (GroupFilterObject*)groupList->Objects_Get(groupId);
			summaryGroup->AddGroup(group);
		}
	}

	return S_OK;
}

HRESULT GroupFilter::IsAllowed(size_t objectIndex)
{
	size_t groupId;

	if (objectIndex >= indexMapSize)
		return E_INVALIDARG;
		
	groupId = indexMap[objectIndex];
	if (S_OK == groupList->Filter_IsAllowed(groupId) && 
		S_OK == Group_IsSelected(groupId))
	{
		return S_OK;
	}

	return S_FALSE;
}

HRESULT GroupFilter::RegisterFilterEventHandler(ifc_viewfilterevent *eventHandler)
{
	size_t index;

	if (NULL == eventHandler)
		return E_INVALIDARG;

	index = filterEventHandlerList.size();
	while(index--)
	{
		if (eventHandler == filterEventHandlerList[index])
			return S_FALSE;
	}
	
	eventHandler->AddRef();
	filterEventHandlerList.push_back(eventHandler);
	
	return S_OK;
}

HRESULT GroupFilter::UnregisterFilterEventHandler(ifc_viewfilterevent *eventHandler)
{
	size_t index;

	if (NULL == eventHandler)
		return E_INVALIDARG;

	index = filterEventHandlerList.size();
	while(index--)
	{
		if (eventHandler == filterEventHandlerList[index])
		{
			filterEventHandlerList.eraseindex(index);
			eventHandler->Release();
			return S_OK;
		}
	}
	
	return S_FALSE;
}

HRESULT GroupFilter::SetPrevious(ifc_doublylinkednode *node)
{
	ifc_viewfilter *filter;

	if (NULL != previous)
	{
		if (SUCCEEDED(previous->QueryInterface(IFC_ViewFilter, (void**)&filter)))
		{
			filter->UnregisterEventHandler(this);
			filter->Release();
		}

		previous->Release();
	}
	
	previous = node;
	
	if (NULL != previous)
	{
		previous->AddRef();

		if (SUCCEEDED(previous->QueryInterface(IFC_ViewFilter, (void**)&filter)))
		{
			filter->RegisterEventHandler(this);
			filter->Release();
		}
	}
	
	return S_OK;
}

HRESULT GroupFilter::SetNext(ifc_doublylinkednode *node)
{
	
	SafeRelease(nextGroup);
	SafeRelease(next);
	
	next = node;
	
	if (NULL != next)
	{
		next->AddRef();
				
		if (FAILED(next->QueryInterface(IFC_ViewGroupFilter, (void**)&nextGroup)))
			nextGroup = NULL;
	}
	
	if (NULL != provider)
	{
		ifc_groupprovider *nextProvider;

		if (NULL == nextGroup ||
			FAILED(nextGroup->GetGroupProvider(&nextProvider)))
		{
			nextProvider = NULL;
		}
		
		provider->SetNextFilter(nextProvider);
		SafeRelease(nextProvider);
	}

	return S_OK;
}

HRESULT GroupFilter::GetPrevious(ifc_doublylinkednode **node)
{
	if (NULL == node)
		return E_POINTER;

	if (NULL == previous)
	{
		*node = NULL;
		return S_FALSE;
	}
	
	previous->AddRef();
	*node = previous;
	return S_OK;
}

HRESULT GroupFilter::GetNext(ifc_doublylinkednode **node)
{
	if (NULL == node)
		return E_POINTER;

	if (NULL == next)
	{
		*node = NULL;
		return S_FALSE;
	}
	
	next->AddRef();
	*node = next;

	return S_OK;
}


HRESULT GroupFilter::GetProvider(ifc_dataprovider **_provider)
{
	if (NULL == _provider)
		return E_POINTER;

	if (NULL == provider)
	{
		*_provider = NULL;
		return S_FALSE;
	}

	*_provider = provider;
	provider->AddRef();
	
	return S_OK;
}

HRESULT GroupFilter::GetObjects(ifc_dataobjectlist **list)
{
	if (NULL == list)
		return E_POINTER;

	if (NULL == groupList)
	{
		*list = NULL;
		return S_FALSE;
	}

	*list = groupList;
	groupList->AddRef();
	
	return S_OK;
}

HRESULT GroupFilter::GetGroupProvider(ifc_groupprovider **groupProvider)
{
	if (NULL == provider)
	{
		if (NULL != groupProvider)
			*groupProvider = NULL;
		return S_FALSE;
	}

	return provider->GetGroupProvider(groupProvider);
}

HRESULT GroupFilter::GetSummaryObject(ifc_dataobject **object)
{
	if (NULL == object)
		return E_POINTER;
	
	*object = summaryGroup;
	if (NULL == summaryGroup)
		return S_FALSE;

	summaryGroup->AddRef();
	return S_OK;
}

HRESULT GroupFilter::UpdateSelection(ifc_viewselection *selection, ifc_viewselection *appended, ifc_viewselection *removed)
{
	size_t previousFiltersCount;
	ifc_viewfilter **previousFilters;

	if (S_OK == Selection_IsLocked())
		return S_FALSE;
	
	if (FALSE != bypassModeEnabled)
	{
		if (0 == selection->GetCount())
		{
			Selection_RemoveAll();
		}
		else
		{			
			Selection_Update(removed, ViewFilterAction_Block);
			Selection_Update(appended, ViewFilterAction_Allow);
		}
		return S_OK;
	}

	previousFiltersCount = GetPreviousFilters(NULL, 0);
	if (0 != previousFiltersCount)
	{
		previousFilters = (ifc_viewfilter**)malloc(previousFiltersCount * sizeof(ifc_viewfilter*));
		if (NULL == previousFilters)
			return E_OUTOFMEMORY;

		previousFiltersCount = GetPreviousFilters(previousFilters, previousFiltersCount);
	}
	else
		previousFilters = NULL;
	
	Notify_FilterBeginUpdate();
	
	if (0 == selection->GetCount())
	{
		Notify_FilterBlockAll();
		Selection_RemoveAll();
	}
	else
	{			
		Selection_UpdateAndNotify(removed, ViewFilterAction_Block, previousFilters, previousFiltersCount);
		Selection_UpdateAndNotify(appended, ViewFilterAction_Allow, previousFilters, previousFiltersCount);
		
	}
	
	while(previousFiltersCount--)
		previousFilters[previousFiltersCount]->Release();

	free(previousFilters);

	Notify_FilterEndUpdate();
	return S_OK;
}

HRESULT GroupFilter::Selection_Update(ifc_viewselection *selection, ViewFilterAction action)
{
	size_t groupId;
	ifc_viewselectionenum *enumerator;
	IndexRange range;
	
	if (NULL == selection)
		return E_INVALIDARG;

	if (0 == selection->GetCount())
		return S_OK;
	
	if (SUCCEEDED(selection->Enumerate(&enumerator)))
	{
		while(S_OK == enumerator->Next(&range, 1, NULL))
		{
			for(;range.first <= range.last; range.first++)
			{
				groupId = groupList->Objects_GetRealIndex(range.first);
				
				if (ViewFilterAction_Allow == action)
					Selection_AddGroup(groupId);
				else
					Selection_RemoveGroup(groupId);
			}
		}
		enumerator->Release();
	}
	
	return S_OK;
}

HRESULT GroupFilter::Selection_UpdateAndNotify(ifc_viewselection *selection, ViewFilterAction action, ifc_viewfilter **previousFilters, size_t previousFiltersCount)
{
	size_t index, groupId, objectIndex;
	GroupFilterObject *group;
	ifc_viewselectionenum *enumerator;
	IndexRange range;
	
	if (NULL == selection)
		return E_INVALIDARG;

	if (0 == selection->GetCount())
		return S_OK;
	
	if (SUCCEEDED(selection->Enumerate(&enumerator)))
	{
		IndexList actionList;

		while(S_OK == enumerator->Next(&range, 1, NULL))
		{
			for(;range.first <= range.last; range.first++)
			{
				groupId = groupList->Objects_GetRealIndex(range.first);
				group = (GroupFilterObject*)groupList->Objects_Get(groupId);
				if (NULL == group)
				{
					if (range.first >= groupList->Objects_GetCount())
						break;

					continue;
				}

				if (ViewFilterAction_Allow == action)
					Selection_AddGroup(groupId);
				else
					Selection_RemoveGroup(groupId);

				index = group->SearchIndex_Get();
				if ((size_t)-1 != index)
				{
					objectIndex = indexSortMap[index];
					do
					{						
						if (S_OK == IsAllowedByFilters(objectIndex, previousFilters, previousFiltersCount))
						{	
							actionList.push_back(objectIndex);
						}
						
						index++;
						if (index >= indexMapSize)
							break;

						objectIndex = indexSortMap[index];

					}while(groupId == indexMap[objectIndex]);
				}
			}
		}
		enumerator->Release();

		if (0 != actionList.size())
			Notify_FilterActionChanged(actionList.begin(), actionList.size(), action);

	}
	
	return S_OK;
}

size_t GroupFilter::GetGroupId(size_t objectIndex, BOOL ignoreUnknownGroup)
{
	size_t groupId;

	if (objectIndex >= indexMapSize)
		return (size_t)-1;
	
	groupId = indexMap[objectIndex];
	if (FALSE != ignoreUnknownGroup)
	{
		GroupFilterObject *group;
		group = (GroupFilterObject*)groupList->Objects_Get(groupId);
		if (NULL == group || S_OK == group->IsUnknown())
			return (size_t)-1;
	}

	return groupId;
}

HRESULT GroupFilter::EnableBypass(BOOL enableBypass)
{
	IndexList actionList;
	size_t groupId, groupCount, groupIndex;
	size_t index, objectIndex;
	GroupFilterObject *group;
	ViewFilterAction filterAction;

	if (bypassModeEnabled == enableBypass)
		return S_FALSE;
	
	if (FALSE != enableBypass)
		filterAction = ViewFilterAction_Allow;
	else
		filterAction = ViewFilterAction_Block;

	if (0 != groupList->GetCount())
	{
		size_t previousFiltersCount;
		ifc_viewfilter **previousFilters;

		bypassModeEnabled = FALSE; // to allow Group_IsSelected()

		previousFiltersCount = GetPreviousFilters(NULL, 0);
		if (0 != previousFiltersCount)
		{
			previousFilters = (ifc_viewfilter**)malloc(previousFiltersCount * sizeof(ifc_viewfilter*));
			if (NULL == previousFilters)
				return E_OUTOFMEMORY;

			previousFiltersCount = GetPreviousFilters(previousFilters, previousFiltersCount);
		}
		else
			previousFilters = NULL;
		
		

		groupCount = groupList->Objects_GetCount();
		for (groupIndex = 0; groupIndex < groupCount; groupIndex++)
		{
			groupId = groupList->Objects_GetRealIndex(groupIndex);
			group = (GroupFilterObject*)groupList->Objects_Get(groupId);
			if (NULL == group)
				continue;
			
			if (S_OK == groupList->Filter_IsAllowed(groupId) &&
				S_OK != Group_IsSelected(groupId))
			{

				index = group->SearchIndex_Get();
				if ((size_t)-1 != index)
				{
					objectIndex = indexSortMap[index];
					do
					{						
						if (S_OK == IsAllowedByFilters(objectIndex, previousFilters, previousFiltersCount))
						{	
							actionList.push_back(objectIndex);
						}
						
						index++;
						if (index >= indexMapSize)
							break;

						objectIndex = indexSortMap[index];

					}while(groupId == indexMap[objectIndex]);
				}
			}
		}

		while(previousFiltersCount--)
			previousFilters[previousFiltersCount]->Release();

		free(previousFilters);
	}

	bypassModeEnabled = enableBypass;

	Notify_FilterBeginUpdate();
	
	Notify_BypassModeChanged();
		
	if (0 != actionList.size())
	{		
		Notify_FilterActionChanged(actionList.begin(), actionList.size(), filterAction);
	}

	Notify_FilterEndUpdate();
	return S_OK;
}

HRESULT GroupFilter::IsBypassEnabled()
{
	return (FALSE != bypassModeEnabled) ? S_OK : S_FALSE;
}
HRESULT GroupFilter::RegisterGroupFilterEventHandler(ifc_viewgroupfilterevent *eventHandler)
{
	size_t index;

	if (NULL == eventHandler)
		return E_INVALIDARG;

	index = groupFilterEventHandlerList.size();
	while(index--)
	{
		if (eventHandler == groupFilterEventHandlerList[index])
			return S_FALSE;
	}
	
	eventHandler->AddRef();
	groupFilterEventHandlerList.push_back(eventHandler);
	
	return S_OK;
}

HRESULT GroupFilter::UnregisterGroupFilterEventHandler(ifc_viewgroupfilterevent *eventHandler)
{
	size_t index;

	if (NULL == eventHandler)
		return E_INVALIDARG;

	index = groupFilterEventHandlerList.size();
	while(index--)
	{
		if (eventHandler == groupFilterEventHandlerList[index])
		{
			groupFilterEventHandlerList.eraseindex(index);
			eventHandler->Release();
			return S_OK;
		}
	}
	
	return S_FALSE;
}

void GroupFilter::FilterEvent_BeginUpdate(ifc_viewfilter *instance)
{
	groupList->StartUpdate();
	Notify_FilterBeginUpdate();
}

void GroupFilter::FilterEvent_EndUpdate(ifc_viewfilter *instance)
{
	groupList->FinishUpdate();
	Notify_FilterEndUpdate();
}

void GroupFilter::FilterEvent_BlockAll(ifc_viewfilter *instance)
{
	Selection_Lock();
	if (S_OK == groupList->Filter_BlockAll())
	{
		SafeRelease(summaryGroup);

		Selection_RemoveAll();
		Notify_FilterBlockAll();
	}
	Selection_Unlock();
}

void GroupFilter::FilterEvent_ActionChanged(ifc_viewfilter *instance, const size_t *objects, size_t count, ViewFilterAction action)
{
	size_t groupId, objectIndex;
	GroupFilterObject *group;
	int hashResult;
	khint_t key, hashSize;

	IndexList actionList;
	actionList.reserve(count);

	khash_t(sizet_set) *filterHash, *notifyHash;
	filterHash = kh_init(sizet_set);
	notifyHash = kh_init(sizet_set);
		
	switch(action)
	{
		case ViewFilterAction_Allow:
			
			if (S_OK == Selection_IsEmpty())
				EnableBypass(TRUE);
			
			while(count--)
			{
				objectIndex = objects[count];
				groupId = indexMap[objectIndex];
				group = (GroupFilterObject*)groupList->Objects_Get(groupId);
			
				if (S_OK != groupList->Filter_IsAllowed(groupId))
				{	
					kh_put(sizet_set, filterHash, groupId, &hashResult);
					if (0 != hashResult)
					{
						group->Reset();
						if (NULL != summaryGroup)
							summaryGroup->AddGroup(group);
					}
				}
				else
					hashResult = 0;

				Group_AddObjectInfo(group, objectIndex);
				
				if (0 == hashResult)
					kh_put(sizet_set, notifyHash, groupId, &hashResult);
				
				if (S_OK == Group_IsSelected(groupId))
					actionList.push_back(objectIndex);
			}
			
			break;

		case ViewFilterAction_Block:

			while(count--)
			{
				objectIndex = objects[count];
				groupId = indexMap[objectIndex];
				group = (GroupFilterObject*)groupList->Objects_Get(groupId);

				if (S_OK == groupList->Filter_IsAllowed(groupId) &&
					0 != group->Objects_GetCount())
					/*kh_end(filterHash) != kh_get(sizet_set, filterHash, groupId))*/
				{
					
					HRESULT groupSelected;

					groupSelected = Group_IsSelected(groupId);
					Group_RemoveObjectInfo(group, objectIndex);

					if (0 == group->Objects_GetCount())
					{
						Selection_RemoveGroup(groupId);
						kh_put(sizet_set, filterHash, groupId, &hashResult);
						if (NULL != summaryGroup)
							summaryGroup->SubtractGroup(group);
					}
					else
					{	
						kh_put(sizet_set, notifyHash, groupId, &hashResult);
					}

					if (S_OK == groupSelected)
						actionList.push_back(objectIndex);
				}
			}
			break;
	}

	

	hashSize = kh_size(filterHash);
	if (kh_size(notifyHash) > hashSize)
		hashSize = kh_size(notifyHash);

	if (0 != hashSize)
	{
		size_t tempCount;
		size_t *tempList = (size_t*)malloc(sizeof(size_t) * hashSize);
		if (NULL != tempList)
		{
					
			Selection_Lock();

			hashSize = kh_size(filterHash);
			if (0 != hashSize)
			{
				tempCount = 0;
				for (key = kh_begin(filterHash); key != kh_end(filterHash); ++key)
				{
					if (kh_exist(filterHash, key))
						tempList[tempCount++] = kh_key(filterHash, key);
				}

				if (0 != tempCount)
				{
					if (ViewFilterAction_Allow == action)
						groupList->Filter_Allow(tempList, tempCount);
					else
						groupList->Filter_Block(tempList, tempCount);
				}
			}

			hashSize = kh_size(notifyHash);
			if (0 != hashSize)
			{
				tempCount = 0;
				for (key = kh_begin(notifyHash); key != kh_end(notifyHash); ++key)
				{
					if (kh_exist(notifyHash, key))
						tempList[tempCount++] = kh_key(notifyHash, key);
				}

				if (0 != tempCount)
				{
					groupList->Filter_NotifyChange(tempList, tempCount);
				}
			}

			Selection_Unlock();
		}
		free(tempList);
		
	}

	if (ViewFilterAction_Block == action && 
		S_OK == Selection_IsEmpty())
	{
		EnableBypass(TRUE);
	}

	if (0 != actionList.size())
		Notify_FilterActionChanged(actionList.begin(), actionList.size(), action);

	kh_destroy(sizet_set, filterHash);
	kh_destroy(sizet_set, notifyHash);
}

size_t GroupFilter::GetPreviousFilters(ifc_viewfilter **buffer, size_t bufferSize)
{
	size_t count;
	ifc_doublylinkednode *node, *previous;

	count = 0;
	
	node = this;
	node->AddRef();
	
		

	if (NULL == buffer && 0 == bufferSize)
	{
		while(S_OK == node->GetPrevious(&previous))
		{
			node->Release();
			node = previous;
			count++;
		}
	}
	else if (NULL != buffer)
	{
		while(bufferSize-- &&
			  S_OK == node->GetPrevious(&previous))
		{
			node->Release();
			node = previous;

			if (SUCCEEDED(node->QueryInterface(IFC_ViewFilter, (void**)&buffer[count])))
				count++;
		}
	}

	node->Release();

	return count;
}

HRESULT GroupFilter::IsAllowedByFilters(size_t objectIndex, ifc_viewfilter **filters, size_t filtersCount)
{
	HRESULT hr;
	size_t index;
	
	for (index = 0; index < filtersCount; index++)
	{
		hr = filters[index]->IsAllowed(objectIndex);
		if (S_OK != hr)
			return hr;
	}

	return S_OK;
}

HRESULT GroupFilter::Group_AddObjectInfo(GroupFilterObject *group, size_t objectIndex)
{
	LCID localeId;
	DataValue dataValue;
	ifc_dataobject *object;
	unsigned __int64 i64val;
	
	object = objectList->GetItem(objectIndex);

	if (NULL == group || NULL == object)
		return E_INVALIDARG;

	localeId = Plugin_GetUserLocaleId();

	DATAVALUE_INIT_INT32(&dataValue, 0);
	if(SUCCEEDED(object->GetValue(localeId, lengthValueId, &dataValue)))
	{
		i64val = DATAVALUE_GET_INT32(&dataValue);
		group->Length_Add(i64val);
		if (NULL != summaryGroup)
			summaryGroup->Length_Add(i64val);
	}
	
	DATAVALUE_CLEAR(&dataValue);

	DATAVALUE_INIT_INT64(&dataValue, 0);
	if(SUCCEEDED(object->GetValue(localeId, sizeValueId, &dataValue)))
	{
		i64val = DATAVALUE_GET_INT64(&dataValue);
		group->Size_Add(i64val);
		if (NULL != summaryGroup)
			summaryGroup->Size_Add(i64val);
	}

	DATAVALUE_CLEAR(&dataValue);

	group->Add(object);

	 
	if (NULL != nextGroup)
	{
		size_t groupId;
		groupId = nextGroup->GetGroupId(objectIndex, TRUE);
		if ((size_t)-1 != groupId)
		{
			group->NextFilters_Add(groupId);
			if (NULL != summaryGroup)
				summaryGroup->NextFilters_Add(groupId);
		}
	}
	

	if (NULL != summaryGroup)
		summaryGroup->Add(object);
		
	return S_OK;
}

HRESULT GroupFilter::Group_RemoveObjectInfo(GroupFilterObject *group, size_t objectIndex)
{
	LCID localeId;
	DataValue dataValue;
	ifc_dataobject *object;
	unsigned __int64 i64val;

	object = objectList->GetItem(objectIndex);

	if (NULL == group || NULL == object)
		return E_INVALIDARG;

	localeId = Plugin_GetUserLocaleId();

	DATAVALUE_INIT_INT32(&dataValue, 0);
	if(SUCCEEDED(object->GetValue(localeId, lengthValueId, &dataValue)))
	{
		i64val = DATAVALUE_GET_INT32(&dataValue);
		group->Length_Subtract(i64val);
		if (NULL != summaryGroup)
			summaryGroup->Length_Subtract(i64val);
	}

	
	DATAVALUE_CLEAR(&dataValue);

	DATAVALUE_INIT_INT64(&dataValue, 0);
	if(SUCCEEDED(object->GetValue(localeId, sizeValueId, &dataValue)))
	{
		i64val = DATAVALUE_GET_INT64(&dataValue);
		group->Size_Subtract(i64val);
		if (NULL != summaryGroup)
			summaryGroup->Size_Subtract(i64val);
	}
	
	DATAVALUE_CLEAR(&dataValue);

	group->Subtract(object);

	if (NULL != nextGroup)
	{
		size_t groupId;
		groupId = nextGroup->GetGroupId(objectIndex, TRUE);
		if ((size_t)-1 != groupId)
		{
			group->NextFilters_Subtract(groupId);
			if (NULL != summaryGroup)
				summaryGroup->NextFilters_Subtract(groupId);
		}
	}

	if (NULL != summaryGroup)
		summaryGroup->Subtract(object);

	return S_OK;
}

HRESULT GroupFilter::Group_IsSelected(size_t groupId)
{
	return (FALSE != bypassModeEnabled || FALSE != groupSelectionMap[groupId]) ? S_OK : S_FALSE;
}

HRESULT GroupFilter::Selection_AddGroup(size_t groupId)
{
	if (FALSE != groupSelectionMap[groupId])
		return S_FALSE;

	groupSelectionMap[groupId] = TRUE;
	groupSelectionCount++;
	
	return S_OK;
}

HRESULT GroupFilter::Selection_RemoveGroup(size_t groupId)
{
	if (FALSE == groupSelectionMap[groupId])
		return S_FALSE;

	groupSelectionMap[groupId] = FALSE;
	if (0 != groupSelectionCount)
		groupSelectionCount--;
	
	return S_OK;
}

HRESULT GroupFilter::Selection_RemoveAll()
{
	if (0 == groupSelectionCount)
		return S_FALSE;

	memset(groupSelectionMap, 0, sizeof(BOOL) * groupList->Objects_GetCount());
	groupSelectionCount = 0;
	
	return S_OK;
}

HRESULT GroupFilter::Selection_IsEmpty()
{
	return (FALSE == bypassModeEnabled && 0 == groupSelectionCount) ? S_OK : S_FALSE;
}

void GroupFilter::Selection_Lock()
{
	selectionLock++;
}
void GroupFilter::Selection_Unlock()
{
	if (0 != selectionLock)
		selectionLock--;
}

HRESULT GroupFilter::Selection_IsLocked()
{
	return (0 != selectionLock) ? S_OK : S_FALSE;
}

void GroupFilter::Notify_FilterBeginUpdate()
{
	size_t index, count;
	
	count = filterEventHandlerList.size();
	for(index = 0; index < count; index++)
	{
		filterEventHandlerList[index]->FilterEvent_BeginUpdate(this);
	}
}

void GroupFilter::Notify_FilterEndUpdate()
{
	size_t index, count;
	
	count = filterEventHandlerList.size();
	for(index = 0; index < count; index++)
	{
		filterEventHandlerList[index]->FilterEvent_EndUpdate(this);
	}
}

void GroupFilter::Notify_FilterBlockAll()
{
	size_t index, count;
	
	count = filterEventHandlerList.size();
	for(index = 0; index < count; index++)
	{
		filterEventHandlerList[index]->FilterEvent_BlockAll(this);
	}
}

void GroupFilter::Notify_FilterActionChanged(const size_t *objectIndex, size_t count, ViewFilterAction action)
{
	size_t index, handlerCount;

	handlerCount = filterEventHandlerList.size();
	for(index = 0; index < handlerCount; index++)
	{
		filterEventHandlerList[index]->FilterEvent_ActionChanged(this, objectIndex, count, action);
	}
}

void GroupFilter::Notify_BypassModeChanged()
{
	size_t index, handlerCount;

	handlerCount = groupFilterEventHandlerList.size();
	for(index = 0; index < handlerCount; index++)
	{
		groupFilterEventHandlerList[index]->GroupFilterEvent_BypassModeChanged(this, bypassModeEnabled);
	}
}

#define CBCLASS GroupFilter
START_MULTIPATCH;
	START_PATCH(MPIID_GF_VIEWFILTER)
		M_CB(MPIID_GF_VIEWFILTER, ifc_viewfilter, ADDREF, AddRef);
		M_CB(MPIID_GF_VIEWFILTER, ifc_viewfilter, RELEASE, Release);
		M_CB(MPIID_GF_VIEWFILTER, ifc_viewfilter, QUERYINTERFACE, QueryInterface);
		M_CB(MPIID_GF_VIEWFILTER, ifc_viewfilter, API_GETNAME, GetName);
		M_CB(MPIID_GF_VIEWFILTER, ifc_viewfilter, API_BIND, Bind);
		M_CB(MPIID_GF_VIEWFILTER, ifc_viewfilter, API_INIT, Init);
		M_CB(MPIID_GF_VIEWFILTER, ifc_viewfilter, API_ISALLOWED, IsAllowed);
		M_CB(MPIID_GF_VIEWFILTER, ifc_viewfilter, API_UPDATE, Update);
		M_CB(MPIID_GF_VIEWFILTER, ifc_viewfilter, ifc_viewfilter::API_REGISTEREVENTHANDLER, RegisterFilterEventHandler);
		M_CB(MPIID_GF_VIEWFILTER, ifc_viewfilter, ifc_viewfilter::API_UNREGISTEREVENTHANDLER, UnregisterFilterEventHandler);
	NEXT_PATCH(MPIID_GF_DOUBLYLINKEDNODE)
		M_CB(MPIID_GF_DOUBLYLINKEDNODE, ifc_doublylinkednode, ADDREF, AddRef);
		M_CB(MPIID_GF_DOUBLYLINKEDNODE, ifc_doublylinkednode, RELEASE, Release);
		M_CB(MPIID_GF_DOUBLYLINKEDNODE, ifc_doublylinkednode, QUERYINTERFACE, QueryInterface);
		M_CB(MPIID_GF_DOUBLYLINKEDNODE, ifc_doublylinkednode, API_SETPREVIOUS, SetPrevious);
		M_CB(MPIID_GF_DOUBLYLINKEDNODE, ifc_doublylinkednode, API_SETNEXT, SetNext);
		M_CB(MPIID_GF_DOUBLYLINKEDNODE, ifc_doublylinkednode, API_GETPREVIOUS, GetPrevious);
		M_CB(MPIID_GF_DOUBLYLINKEDNODE, ifc_doublylinkednode, API_GETNEXT, GetNext);
	NEXT_PATCH(MPIID_GF_VIEWGROUPFILTER)
		M_CB(MPIID_GF_VIEWGROUPFILTER, ifc_viewgroupfilter, ADDREF, AddRef);
		M_CB(MPIID_GF_VIEWGROUPFILTER, ifc_viewgroupfilter, RELEASE, Release);
		M_CB(MPIID_GF_VIEWGROUPFILTER, ifc_viewgroupfilter, QUERYINTERFACE, QueryInterface);
		M_CB(MPIID_GF_VIEWGROUPFILTER, ifc_viewgroupfilter, API_GETPROVIDER, GetProvider);
		M_CB(MPIID_GF_VIEWGROUPFILTER, ifc_viewgroupfilter, API_GETOBJECTS, GetObjects);
		M_CB(MPIID_GF_VIEWGROUPFILTER, ifc_viewgroupfilter, API_GETGROUPPROVIDER, GetGroupProvider);
		M_CB(MPIID_GF_VIEWGROUPFILTER, ifc_viewgroupfilter, API_GETSUMMARYOBJECT, GetSummaryObject);
		M_CB(MPIID_GF_VIEWGROUPFILTER, ifc_viewgroupfilter, API_UPDATESELECTION, UpdateSelection);
		M_CB(MPIID_GF_VIEWGROUPFILTER, ifc_viewgroupfilter, API_GETGROUPID, GetGroupId);
		M_CB(MPIID_GF_VIEWGROUPFILTER, ifc_viewgroupfilter, API_ENABLEBYPASS, EnableBypass);
		M_CB(MPIID_GF_VIEWGROUPFILTER, ifc_viewgroupfilter, API_ISBYPASSENABLED, IsBypassEnabled);
		M_CB(MPIID_GF_VIEWGROUPFILTER, ifc_viewgroupfilter, ifc_viewgroupfilter::API_REGISTEREVENTHANDLER, RegisterGroupFilterEventHandler);
		M_CB(MPIID_GF_VIEWGROUPFILTER, ifc_viewgroupfilter, ifc_viewgroupfilter::API_UNREGISTEREVENTHANDLER, UnregisterGroupFilterEventHandler);
	NEXT_PATCH(MPIID_GF_VIEWFILTEREVENT)
		M_CB(MPIID_GF_VIEWFILTEREVENT, ifc_viewfilterevent, ADDREF, AddRef);
		M_CB(MPIID_GF_VIEWFILTEREVENT, ifc_viewfilterevent, RELEASE, Release);
		M_CB(MPIID_GF_VIEWFILTEREVENT, ifc_viewfilterevent, QUERYINTERFACE, QueryInterface);
		M_VCB(MPIID_GF_VIEWFILTEREVENT, ifc_viewfilterevent, API_FILTEREVENT_BEGINUPDATE, FilterEvent_BeginUpdate);
		M_VCB(MPIID_GF_VIEWFILTEREVENT, ifc_viewfilterevent, API_FILTEREVENT_ENDUPDATE, FilterEvent_EndUpdate);
		M_VCB(MPIID_GF_VIEWFILTEREVENT, ifc_viewfilterevent, API_FILTEREVENT_BLOCKALL, FilterEvent_BlockAll);
		M_VCB(MPIID_GF_VIEWFILTEREVENT, ifc_viewfilterevent, API_FILTEREVENT_ACTIONCHANGED, FilterEvent_ActionChanged);
	END_PATCH
END_MULTIPATCH;
