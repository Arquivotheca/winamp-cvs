#include "main.h"
#include "./stringGroup.h"

#include <strsafe.h>

static ifc_sortkey *
StringGroup_CreateSortKey(LCID localeId, const wchar_t *string)
{
	StringSortKey *key;

	if (FAILED(StringSortKey::CreateInstance(localeId, 
											 string, 
											 StringSortKey_Trim | 
											 StringSortKey_RemoveArticle | 
											 StringSortKey_IgnoreCase, 
											 &key)))
	{
		return NULL;
	}

	return key;
}

StringGroup::StringGroup(ifc_groupprovider *_provider, wchar_t *_string, ifc_sortkey *_sortKey)
	: ref(1), provider(_provider), string(_string), sortKey(_sortKey)
{
	if (NULL != provider)
		provider->AddRef();

	if (NULL != sortKey)
		sortKey->AddRef();
}

StringGroup::~StringGroup()
{
	String_Free(string);

	SafeRelease(provider);
	SafeRelease(sortKey);
}

HRESULT StringGroup::CreateInstance(ifc_groupprovider *provider, const wchar_t *string, ifc_sortkey *sortKey, StringGroup **instance)
{
	wchar_t *stringCopy;
	if (NULL == instance) 
		return E_POINTER;

	stringCopy = String_Duplicate(string);
	if (NULL == stringCopy && NULL != string)
	{
		*instance = NULL;
		return E_OUTOFMEMORY;
	}

	*instance = new (std::nothrow) StringGroup(provider, stringCopy, sortKey);
	if (NULL == *instance) 
	{
		String_Free(stringCopy);
		return E_OUTOFMEMORY;
	}

	return S_OK;
}

size_t StringGroup::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t StringGroup::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int StringGroup::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) 
		return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_DataObject))
		*object = static_cast<ifc_dataobject*>(this);
	else if (IsEqualIID(interface_guid, IFC_GroupObject))
		*object = static_cast<ifc_groupobject*>(this);
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


HRESULT StringGroup::GetValue(LCID localeId, size_t valueId, DataValue *value)
{
	if (NULL == value)
		return E_POINTER;

	switch((ValueId)valueId)
	{
		case ValueId_String:
			DATAVALUE_CHECK_TYPE_RETURN_ERR(value, DT_WSTR_BUFFER, VALUE_E_BADTYPE);
			if (S_OK == IsUnknown() && NULL != provider)
			{
				wchar_t buffer[512], format[64];
				if (SUCCEEDED(provider->GetEmptyText(buffer, ARRAYSIZE(buffer))) &&
					L'\0' != buffer[0])
				{
					if (NULL == ResourceString_CopyTo(format, ARRAYSIZE(format), MAKEINTRESOURCE(IDS_GROUPFILTER_EMPTYGROUP_FORMAT)) ||
						L'\0' == format[0])
					{
						StringCchCopy(format, ARRAYSIZE(format), L"[%%s]");
					}

					if (FAILED(StringCchPrintf(value->wstrBuffer.data, value->wstrBuffer.size,
												format, buffer)))
					{
						return VALUE_E_OUTOFMEMORY;
					}
					return S_OK;
				}
			}
			if (FAILED(StringCchCopyEx(value->wstrBuffer.data, value->wstrBuffer.size, string, 
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


HRESULT StringGroup::IsEqual(LCID localeId, ifc_dataobject *object2)
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

int StringGroup::Compare(LCID localeId, size_t valueId, DataValue *value)
{
	if (NULL == value)
		return COBJ_ERROR;

	switch((ValueId)valueId)
	{
		case ValueId_String:
			DATAVALUE_COMPARE_STRING(string, value, localeId, CompareString_IgnoreCaseTheSpace); 
		case ValueId_SortKey:
			DATAVALUE_COMPARE_SORTKEY(GetSortKey(localeId), value, localeId);
	}
	return COBJ_ERROR;
}


int StringGroup::CompareTo(LCID localeId, size_t valueId, ifc_dataobject *object2)
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
				DATAVALUE_INIT_WSTR(&value, string);
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

HRESULT StringGroup::Add(ifc_dataobject *object)
{
	return E_NOTIMPL;
}

HRESULT StringGroup::Subtract(ifc_dataobject *object)
{
	return E_NOTIMPL;
}

HRESULT StringGroup::Reset()
{
	return E_NOTIMPL;
}

HRESULT StringGroup::IsUnknown()
{
	return (FALSE != IS_STRING_EMPTY(string)) ? S_OK : S_FALSE;
}

ifc_sortkey *StringGroup::GetSortKey(LCID localeId)
{
	if (NULL == sortKey)
	{
		sortKey = StringGroup_CreateSortKey(localeId, string);
	}
	else if (sortKey->GetLocaleId() != localeId)
	{
		sortKey->Release();
		sortKey = StringGroup_CreateSortKey(localeId, string);
	}
	return sortKey;	
}

#define CBCLASS StringGroup
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
	END_PATCH
END_MULTIPATCH;
