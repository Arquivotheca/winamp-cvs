#include "main.h"
#include "./groupFilterObject.h"

GroupFilterObject::GroupFilterObject(ifc_dataobject *_group)
	: ref(1), groupData(_group), groupObject(NULL), objectCount(0), 
	  length(0), size(0), searchIndex((size_t)-1), nextFilters(NULL)
{
	if (NULL != groupData)
	{
		groupData->AddRef();
		if (FAILED(groupData->QueryInterface(IFC_GroupObject, (void**)&groupObject)))
			groupObject = NULL;
	}
	nextFilters = kh_init(sizet_map);
}

GroupFilterObject::~GroupFilterObject()
{
	SafeRelease(groupData);
	SafeRelease(groupObject);

	if (NULL != nextFilters)
		kh_destroy(sizet_map, nextFilters);
}

HRESULT GroupFilterObject::CreateInstance(ifc_dataobject *groupData, GroupFilterObject **instance)
{
	if (NULL == instance)
		return E_POINTER;

	if (NULL == groupData)
	{
		*instance = NULL;
		return E_INVALIDARG;
	}

	*instance = new (std::nothrow) GroupFilterObject(groupData);
	if (NULL == *instance)
		return E_OUTOFMEMORY;

	return S_OK;
}

size_t GroupFilterObject::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t GroupFilterObject::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int GroupFilterObject::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) 
		return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_DataObject))
		*object = static_cast<ifc_dataobject*>(this);
	else if (IsEqualIID(interface_guid, IFC_GroupObject))
		*object = static_cast<ifc_groupobject*>(this);
	else if (IsEqualIID(interface_guid, IFC_SummaryGroupObject))
		*object = static_cast<ifc_summarygroupobject*>(this);
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


HRESULT GroupFilterObject::GetValue(LCID localeId, size_t valueId, DataValue *value)
{
	if (NULL == value)
		return E_POINTER;

	switch((ValueId)valueId)
	{
		case ValueId_Group:
			DATAVALUE_SET_DISPATCHABLE_RETURN_ERR(value, groupData, VALUE_E_BADTYPE);
			return S_OK;
		case ValueId_NextFilterCount:
			DATAVALUE_SET_INT_RETURN_ERR(value, NextFilters_GetCount(), VALUE_E_BADTYPE);
			return S_OK;
		case ValueId_TrackCount:
			DATAVALUE_SET_INT_RETURN_ERR(value, Objects_GetCount(), VALUE_E_BADTYPE);
			return S_OK;
		case ValueId_Size:
			DATAVALUE_SET_XINT64_RETURN_ERR(value, Size_Get(), VALUE_E_BADTYPE);
			return S_OK;
		case ValueId_Length:
			DATAVALUE_SET_XINT64_RETURN_ERR(value, Length_Get(), VALUE_E_BADTYPE);
			return S_OK;
	}

	if (NULL == groupData)
		return VALUE_E_NOTFOUND;

	return groupData->GetValue(localeId, valueId, value);
}

HRESULT GroupFilterObject::IsEqual(LCID localeId, ifc_dataobject *object2)
{
	DataValue value;
	int result;

	if (NULL == object2)
		return E_INVALIDARG;

	DATAVALUE_INIT_DISPATCHABLE(&value, groupData);

	result = object2->Compare(localeId, ValueId_Group, &value);

	DATAVALUE_CLEAR(&value);

	return (COBJ_EQUAL == result) ? S_OK : S_FALSE;
}

int GroupFilterObject::Compare(LCID localeId, size_t valueId, DataValue *value)
{
	if (NULL == value)
		return COBJ_ERROR;

	switch((ValueId)valueId)
	{
		case ValueId_Group:
			DATAVALUE_CHECK_TYPE_RETURN_ERR(value, DT_DISPATCHABLE, COBJ_ERROR);
			return COBJ_COMPARE(groupData, value->dispatchVal);
		case ValueId_NextFilterCount:
			DATAVALUE_CHECK_TYPE_RETURN_ERR(value, DT_SIZE, COBJ_ERROR);
			return COBJ_COMPARE(NextFilters_GetCount(), value->sizeVal);
		case ValueId_TrackCount:
			DATAVALUE_CHECK_TYPE_RETURN_ERR(value, DT_SIZE, COBJ_ERROR);
			return COBJ_COMPARE(Objects_GetCount(), value->sizeVal);
		case ValueId_Size:
			DATAVALUE_CHECK_TYPE_RETURN_ERR(value, DT_UINT64, COBJ_ERROR);
			return COBJ_COMPARE(Size_Get(), value->ui64Val);
		case ValueId_Length:
			DATAVALUE_CHECK_TYPE_RETURN_ERR(value, DT_UINT64, COBJ_ERROR);
			return COBJ_COMPARE(Length_Get(), value->ui64Val);
	}

	if (NULL == groupData)
		return COBJ_ERROR;

	return groupData->Compare(localeId, valueId, value);
}

int GroupFilterObject::CompareTo(LCID localeId, size_t valueId, ifc_dataobject *object2)
{
	DataValue value;
	int result;

	switch((ValueId)valueId)
	{
		case ValueId_Group:
			DATAVALUE_INIT_DISPATCHABLE(&value, groupData);
			break;
		case ValueId_NextFilterCount:
			DATAVALUE_INIT_SIZE(&value,NextFilters_GetCount());
			break;
		case ValueId_TrackCount:
			DATAVALUE_INIT_SIZE(&value, Objects_GetCount());
			break;
		case ValueId_Size:
			DATAVALUE_INIT_UINT64(&value, Size_Get());
			break;
		case ValueId_Length:
			DATAVALUE_INIT_UINT64(&value, Length_Get());
			break;
		default:
			if (NULL != groupData)
				return groupData->CompareTo(localeId, valueId, object2);
			else
				DATAVALUE_INIT(&value);
			break;
	}

	if (NULL == object2)
		result = COBJ_ERROR;
	else
		result = object2->Compare(localeId, valueId, &value);

	DATAVALUE_CLEAR(&value);

	if (COBJ_LESS_THAN == result)
		result = COBJ_GREATER_THAN;
	else if (COBJ_GREATER_THAN == result)
		result = COBJ_LESS_THAN;
		
	return result;
}

HRESULT GroupFilterObject::Add(ifc_dataobject *object)
{
	objectCount++;

	if (NULL != groupObject)
		groupObject->Add(object);

	return S_OK;
}

HRESULT GroupFilterObject::Subtract(ifc_dataobject *object)
{
	if (objectCount > 0)
		objectCount--;

	if (NULL != groupObject)
		groupObject->Subtract(object);

	return S_OK;
}

HRESULT GroupFilterObject::Reset()
{
	objectCount = 0;

	NextFilters_Reset();
	Size_Reset();
	Length_Reset();

	if (NULL != groupObject)
		groupObject->Reset(); 

	return S_OK;
}

HRESULT GroupFilterObject::IsUnknown()
{
	if (NULL == groupObject)
		return S_OK;

	return groupObject->IsUnknown();
}

HRESULT GroupFilterObject::AddGroup(ifc_groupobject *group)
{
	HRESULT hr;
	ifc_summarygroupobject *summaryGroup;

	if (NULL == groupObject)
		return E_NOTIMPL;

	if (FAILED(groupObject->QueryInterface(IFC_SummaryGroupObject, (void**)&summaryGroup)))
		return E_NOTIMPL;

	hr = summaryGroup->AddGroup(group);
	summaryGroup->Release();

	return hr;
}

HRESULT GroupFilterObject::SubtractGroup(ifc_groupobject *group)
{
	HRESULT hr;
	ifc_summarygroupobject *summaryGroup;

	if (NULL == groupObject)
		return E_NOTIMPL;

	if (FAILED(groupObject->QueryInterface(IFC_SummaryGroupObject, (void**)&summaryGroup)))
		return E_NOTIMPL;

	hr = summaryGroup->SubtractGroup(group);
	summaryGroup->Release();

	return hr;
}

HRESULT GroupFilterObject::GetGroup(ifc_dataobject **_group)
{
	if (NULL == _group)
		return E_POINTER;

	*_group = groupData;

	if (NULL != groupData)
		groupData->AddRef();

	return S_OK;
}

size_t GroupFilterObject::Objects_GetCount()
{
	return objectCount;
}

unsigned __int64 GroupFilterObject::Length_Get()
{
	return length;
}

void GroupFilterObject::Length_Add(unsigned __int64 _length)
{
	length += _length;
}

void GroupFilterObject::Length_Subtract(unsigned __int64 _length)
{
	length -= _length;
}

void GroupFilterObject::Length_Reset()
{
	length = 0;
}

unsigned __int64 GroupFilterObject::Size_Get()
{
	return size;
}

void GroupFilterObject::Size_Add(unsigned __int64 _size)
{
	size += _size;
}

void GroupFilterObject::Size_Subtract(unsigned __int64 _size)
{
	size -= _size;
}

void GroupFilterObject::Size_Reset()
{
	size = 0;
}

void GroupFilterObject::SearchIndex_Set(size_t _searchIndex)
{
	searchIndex = _searchIndex;
}

size_t GroupFilterObject::SearchIndex_Get()
{
	return searchIndex;
}

HRESULT GroupFilterObject::NextFilters_Reset()
{
	kh_clear(sizet_map, nextFilters);
	return S_FALSE;
}

size_t GroupFilterObject::NextFilters_GetCount()
{
	return kh_size(nextFilters);
}

HRESULT GroupFilterObject::NextFilters_Add(size_t groupId)
{
	HRESULT hr;
	int code;
	khint_t key;

	key = kh_put(sizet_map, nextFilters, groupId, &code);
	if (0 == code)
	{
		kh_val(nextFilters, key) = kh_val(nextFilters, key) + 1;
		hr = S_FALSE;
	}
	else
	{
		kh_val(nextFilters, key) = 1;
		hr = S_OK;
	}
		
	return hr;
}

HRESULT GroupFilterObject::NextFilters_Subtract(size_t groupId)
{
	HRESULT hr;
	khint_t key;
	size_t val;

	key = kh_get(sizet_map, nextFilters, groupId);
	if (kh_end(nextFilters) == key)
		return S_FALSE;

	val = kh_val(nextFilters, key);
	if (1 == val)
	{
		kh_del(sizet_map, nextFilters, key);
		hr = S_OK;
	}
	else
	{
		kh_val(nextFilters, key) = val - 1;
		hr = S_FALSE;
	}

	return hr;
}


#define CBCLASS GroupFilterObject
START_MULTIPATCH;
	START_PATCH(MPIID_GFO_DATAOBJECT)
		M_CB(MPIID_GFO_DATAOBJECT, ifc_dataobject, ADDREF, AddRef)
		M_CB(MPIID_GFO_DATAOBJECT, ifc_dataobject, RELEASE, Release)
		M_CB(MPIID_GFO_DATAOBJECT, ifc_dataobject, QUERYINTERFACE, QueryInterface)
		M_CB(MPIID_GFO_DATAOBJECT, ifc_dataobject, API_GETVALUE, GetValue)
		M_CB(MPIID_GFO_DATAOBJECT, ifc_dataobject, API_ISEQUAL, IsEqual)
		M_CB(MPIID_GFO_DATAOBJECT, ifc_dataobject, API_COMPARE, Compare)
		M_CB(MPIID_GFO_DATAOBJECT, ifc_dataobject, API_COMPARETO, CompareTo)
	NEXT_PATCH(MPIID_GFO_GROUPOBJECT)
		M_CB(MPIID_GFO_GROUPOBJECT, ifc_groupobject, ADDREF, AddRef)
		M_CB(MPIID_GFO_GROUPOBJECT, ifc_groupobject, RELEASE, Release)
		M_CB(MPIID_GFO_GROUPOBJECT, ifc_groupobject, QUERYINTERFACE, QueryInterface)
		M_CB(MPIID_GFO_GROUPOBJECT, ifc_groupobject, API_ADD, Add)
		M_CB(MPIID_GFO_GROUPOBJECT, ifc_groupobject, API_SUBTRACT, Subtract)
		M_CB(MPIID_GFO_GROUPOBJECT, ifc_groupobject, API_RESET, Reset)
		M_CB(MPIID_GFO_GROUPOBJECT, ifc_groupobject, API_ISUNKNOWN, IsUnknown)
	NEXT_PATCH(MPIID_GFO_SUMMARYGROUPOBJECT)
		M_CB(MPIID_GFO_SUMMARYGROUPOBJECT, ifc_summarygroupobject, ADDREF, AddRef)
		M_CB(MPIID_GFO_SUMMARYGROUPOBJECT, ifc_summarygroupobject, RELEASE, Release)
		M_CB(MPIID_GFO_SUMMARYGROUPOBJECT, ifc_summarygroupobject, QUERYINTERFACE, QueryInterface)
		M_CB(MPIID_GFO_SUMMARYGROUPOBJECT, ifc_summarygroupobject, API_ADDGROUP, AddGroup)
		M_CB(MPIID_GFO_SUMMARYGROUPOBJECT, ifc_summarygroupobject, API_SUBTRACTGROUP, SubtractGroup)
	END_PATCH
END_MULTIPATCH;

