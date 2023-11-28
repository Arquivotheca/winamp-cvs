#include "main.h"
#include "./summaryGroup.h"

#include <strsafe.h>


static ifc_sortkey *
SummaryGroup_CreateSortKey(LCID localeId, const wchar_t *string)
{
	StringSortKey *key;

	if (FAILED(StringSortKey::CreateInstance(localeId, 
											 string, 
											 StringSortKey_IgnoreCase, 
											 &key)))
	{
		return NULL;
	}

	return key;
}


SummaryGroup::SummaryGroup()
	: ref(1), groupCount(0), string(NULL), sortKey(NULL)
{
}

SummaryGroup::~SummaryGroup()
{
	String_Free(string);
	SafeRelease(sortKey);
}

HRESULT SummaryGroup::CreateInstance(SummaryGroup **instance)
{
	if (NULL == instance) 
		return E_POINTER;

	*instance = new (std::nothrow) SummaryGroup();
	if (NULL == *instance) 
	{
		return E_OUTOFMEMORY;
	}

	return S_OK;
}


size_t SummaryGroup::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t SummaryGroup::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int SummaryGroup::QueryInterface(GUID interface_guid, void **object)
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


HRESULT SummaryGroup::GetValue(LCID localeId, size_t valueId, DataValue *value)
{
	if (NULL == value)
		return E_POINTER;

	switch((ValueId)valueId)
	{
		case ValueId_String:
			DATAVALUE_CHECK_TYPE_RETURN_ERR(value, DT_WSTR_BUFFER, VALUE_E_BADTYPE);
			if (FAILED(StringCchCopyEx(value->wstrBuffer.data, value->wstrBuffer.size, GetString(), 
										NULL, NULL, STRSAFE_IGNORE_NULLS)))
			{
				return VALUE_E_OUTOFMEMORY;
			}
			return S_OK;
		case ValueId_SortKey:
			DATAVALUE_SET_DISPATCHABLE_RETURN_ERR(value, GetSortKey(localeId), VALUE_E_BADTYPE);
			return S_OK;
	}

	return VALUE_E_NOTFOUND;
}


HRESULT SummaryGroup::IsEqual(LCID localeId, ifc_dataobject *object2)
{
	DataValue value;
	ifc_sortkey *key;

	key = GetSortKey(localeId);
	if (NULL == key)
		return E_FAIL;

	DATAVALUE_INIT_DISPATCHABLE(&value, key);
	if (COBJ_EQUAL != object2->Compare(key->GetLocaleId(), ValueId_SortKey, &value))
		return S_FALSE;
	
	return S_OK;
}

int SummaryGroup::Compare(LCID localeId, size_t valueId, DataValue *value)
{
	if (NULL == value)
		return COBJ_ERROR;

	switch((ValueId)valueId)
	{
		case ValueId_String:
			DATAVALUE_COMPARE_STRING(GetString(), value, localeId, CompareString_IgnoreCase); 
		case ValueId_SortKey:
			DATAVALUE_COMPARE_SORTKEY(GetSortKey(localeId), value, localeId);
	}
	return COBJ_ERROR;
}


int SummaryGroup::CompareTo(LCID localeId, size_t valueId, ifc_dataobject *object2)
{	
	DataValue value;
	int result;
		
	if (NULL == object2)
		return COBJ_ERROR;

	DATAVALUE_INIT(&value);
	
	switch((ValueId)valueId)
	{
		case ValueId_String:
		case ValueId_SortKey:
		{
			ifc_sortkey *key = GetSortKey(localeId);
			if (NULL != key)
			{
				DATAVALUE_INIT_BYTE_BUFFER(&value, (unsigned char*)key->GetValue(), key->GetSize());
				valueId = ValueId_SortKey;
			}
			else
			{
				DATAVALUE_INIT_WSTR(&value, GetString());
				valueId = ValueId_String;
			}
			break;
		}
	}

	if (DT_EMPTY == value.type)
		return COBJ_ERROR;
	
	result = object2->Compare(localeId, valueId, &value);
	
	DATAVALUE_CLEAR(&value);

	if (COBJ_LESS_THAN == result)
		result = COBJ_GREATER_THAN;
	else if (COBJ_GREATER_THAN == result)
		result = COBJ_LESS_THAN;
		
	return result;
}

HRESULT SummaryGroup::Add(ifc_dataobject *object)
{
	return S_OK;
}

HRESULT SummaryGroup::Subtract(ifc_dataobject *object)
{
	return S_OK;
}

HRESULT SummaryGroup::Reset()
{
	groupCount = 0;
	InvalidateString();
	return S_OK;
}

HRESULT SummaryGroup::IsUnknown()
{
	return S_OK;
}

HRESULT SummaryGroup::AddGroup(ifc_groupobject *group)
{
	if (NULL != group && S_OK != group->IsUnknown())
	{
		groupCount++;
		InvalidateString();
	}
	
	return S_OK;
}

HRESULT SummaryGroup::SubtractGroup(ifc_groupobject *group)
{
	if (NULL != group && S_OK != group->IsUnknown() && 0 != groupCount)
	{
		groupCount--;
		InvalidateString();
	}

	return S_OK;
}

void SummaryGroup::InvalidateString()
{
	if (NULL != string)
	{
		String_Free(string);
		string = NULL;

		SafeRelease(sortKey);
	}
}

const wchar_t *SummaryGroup::GetString()
{
	if (NULL == string)
	{
		wchar_t buffer[256], format[128];
		
		if (NULL == ResourceString_CopyTo(format, ARRAYSIZE(format), MAKEINTRESOURCE(IDS_GROUPFILTER_SUMMARY_FORMAT)) ||
		L'\0' == format[0])
		{
			StringCchCopy(format, ARRAYSIZE(format), L"All (%u)");
		}

		if (FAILED(StringCchPrintf(buffer, ARRAYSIZE(buffer), format, groupCount)))
			buffer[0] = L'\0';

		string = String_Duplicate(buffer);
	}

	return string;
}

ifc_sortkey *SummaryGroup::GetSortKey(LCID localeId)
{
	if (NULL == sortKey)
	{
		sortKey = SummaryGroup_CreateSortKey(localeId, GetString());
	}
	else if (sortKey->GetLocaleId() != localeId)
	{
		sortKey->Release();
		sortKey = SummaryGroup_CreateSortKey(localeId, GetString());
	}
	return sortKey;	
}



#define CBCLASS SummaryGroup
START_MULTIPATCH;
	START_PATCH(MPIID_SG_DATAOBJECT)
		M_CB(MPIID_SG_DATAOBJECT, ifc_dataobject, ADDREF, AddRef)
		M_CB(MPIID_SG_DATAOBJECT, ifc_dataobject, RELEASE, Release)
		M_CB(MPIID_SG_DATAOBJECT, ifc_dataobject, QUERYINTERFACE, QueryInterface)
		M_CB(MPIID_SG_DATAOBJECT, ifc_dataobject, API_GETVALUE, GetValue)
		M_CB(MPIID_SG_DATAOBJECT, ifc_dataobject, API_ISEQUAL, IsEqual)
		M_CB(MPIID_SG_DATAOBJECT, ifc_dataobject, API_COMPARE, Compare)
		M_CB(MPIID_SG_DATAOBJECT, ifc_dataobject, API_COMPARETO, CompareTo)
	NEXT_PATCH(MPIID_SG_GROUPOBJECT)
		M_CB(MPIID_SG_GROUPOBJECT, ifc_groupobject, ADDREF, AddRef)
		M_CB(MPIID_SG_GROUPOBJECT, ifc_groupobject, RELEASE, Release)
		M_CB(MPIID_SG_GROUPOBJECT, ifc_groupobject, QUERYINTERFACE, QueryInterface)
		M_CB(MPIID_SG_GROUPOBJECT, ifc_groupobject, API_ADD, Add)
		M_CB(MPIID_SG_GROUPOBJECT, ifc_groupobject, API_SUBTRACT, Subtract)
		M_CB(MPIID_SG_GROUPOBJECT, ifc_groupobject, API_RESET, Reset)
		M_CB(MPIID_SG_GROUPOBJECT, ifc_groupobject, API_ISUNKNOWN, IsUnknown)
	NEXT_PATCH(MPIID_SG_SUMMARYGROUPOBJECT)
		M_CB(MPIID_SG_SUMMARYGROUPOBJECT, ifc_summarygroupobject, ADDREF, AddRef)
		M_CB(MPIID_SG_SUMMARYGROUPOBJECT, ifc_summarygroupobject, RELEASE, Release)
		M_CB(MPIID_SG_SUMMARYGROUPOBJECT, ifc_summarygroupobject, QUERYINTERFACE, QueryInterface)
		M_CB(MPIID_SG_SUMMARYGROUPOBJECT, ifc_summarygroupobject, API_ADDGROUP, AddGroup)
		M_CB(MPIID_SG_SUMMARYGROUPOBJECT, ifc_summarygroupobject, API_SUBTRACTGROUP, SubtractGroup)
	END_PATCH
END_MULTIPATCH;
