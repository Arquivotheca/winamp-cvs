#include "main.h"
#include "./genreGroupProvider.h"
#include "./stringGroup.h"
#include "./summaryGroup.h"

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
	{ "Genre",			StringGroup::ValueId_String },
	{ "Genre_SortKey",	StringGroup::ValueId_SortKey },
};

static int
GenreGroupProvider_FindValueInfoCb(const void *target, const void *element)
{
	ValueInfo *valueInfo;
	valueInfo = (ValueInfo*)element;

	return CompareStringA(CSTR_INVARIANT, NORM_IGNORECASE, (const char*)target, -1, valueInfo->name, -1) - 2;
}


GenreGroupProvider::GenreGroupProvider()
	: ref(1), artistId(VALUEID_UNKNOWN), artistKeyId(VALUEID_UNKNOWN)
{
}

GenreGroupProvider::~GenreGroupProvider()
{
}

HRESULT GenreGroupProvider::CreateInstance(ifc_groupprovider **instance)
{
	if (NULL == instance) 
		return E_POINTER;

	*instance = new (std::nothrow) GenreGroupProvider();
	if (NULL == *instance)
		return E_OUTOFMEMORY;

	return S_OK;
}

size_t GenreGroupProvider::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t GenreGroupProvider::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int GenreGroupProvider::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) 
		return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_GroupProvider))
		*object = static_cast<ifc_groupprovider*>(this);
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

const char *GenreGroupProvider::GetName()
{
	return "Genre";
}

HRESULT GenreGroupProvider::GetDisplayName(wchar_t *buffer, size_t bufferSize)
{
	if(NULL == ResourceString_CopyTo(buffer, bufferSize, MAKEINTRESOURCE(IDS_GROUP_GENRE)))
	{
		return E_OUTOFMEMORY;
	}

	return S_OK;
}

HRESULT GenreGroupProvider::Bind(ifc_dataprovider *provider)
{
	const char *valueName[] = { "Genre", "Genre_SortKey" };
	size_t valueId[ARRAYSIZE(valueName)];

	HRESULT hr;

	if (NULL == provider)
		return E_POINTER;
	
	hr = provider->ResolveNames(valueName, ARRAYSIZE(valueName), valueId);
	artistId = valueId[0];
	artistKeyId = valueId[1];

	if (VALUEID_UNKNOWN == artistId)
		return VALUE_E_UNKNOWNNAME;

	return S_OK;
}

HRESULT GenreGroupProvider::ResolveNames(const char **names, size_t count, size_t *valueIds)
{
	size_t index, valueId, unresolved;

	if (NULL == names || NULL == valueIds)
		return E_POINTER;

	unresolved = 0;

	for (index = 0; index < count; index++)
	{
		valueId = ResolveName(names[index]);
		if (VALUEID_UNKNOWN == valueId)
			unresolved++;

		valueIds[index] = valueId;
	}

	return (0 != unresolved) ? VALUE_E_UNKNOWNNAME : S_OK;

}

HRESULT GenreGroupProvider::CreateGroup(LCID localeId, ifc_dataobject *object, ifc_dataobject **group)
{
	HRESULT hr;
	wchar_t buffer[512];
	
	if (NULL == group)
		return E_POINTER;

	if (NULL == object)
		return E_INVALIDARG;
	
	DataValue value;

	DATAVALUE_INIT_WSTR_BUFFER(&value, buffer, ARRAYSIZE(buffer));
	hr = object->GetValue(localeId, artistId, &value);
	if (SUCCEEDED(hr))
	{
		ifc_sortkey *sortKey;

		if (S_FALSE == hr)
			buffer[0] = L'\0';

		DATAVALUE_CLEAR(&value);
		DATAVALUE_INIT_DISPATCHABLE(&value, NULL);

		hr = object->GetValue(localeId, artistKeyId, &value);
		if (SUCCEEDED(hr))
		{
			sortKey = (ifc_sortkey*)DATAVALUE_GET_DISPATCHABLE(&value);
		}
		else
			sortKey = NULL;

		hr = StringGroup::CreateInstance(this, buffer, sortKey, (StringGroup**)group);

		DATAVALUE_CLEAR(&value);
	}
	else
	{
		DATAVALUE_CLEAR(&value);
		hr = E_FAIL;
	}

	return hr;
}

int GenreGroupProvider::CompareObjects(LCID localeId, ifc_dataobject *object1, ifc_dataobject *object2)
{
	int result;

	if (NULL == object1)
		return COBJ_ERROR;

	result = object1->CompareTo(localeId, artistKeyId, object2);
	if (COBJ_ERROR == result)
		result = object1->CompareTo(localeId, artistId, object2);

	return result;
}

HRESULT GenreGroupProvider::GetCounterText(wchar_t *buffer, size_t bufferSize)
{
	if(NULL == ResourceString_CopyTo(buffer, bufferSize, MAKEINTRESOURCE(IDS_GROUP_GENRE_COUNTER)))
	{
		return E_OUTOFMEMORY;
	}

	return S_OK;
}

HRESULT GenreGroupProvider::GetEmptyText(wchar_t *buffer, size_t bufferSize)
{
	if(NULL == ResourceString_CopyTo(buffer, bufferSize, MAKEINTRESOURCE(IDS_GROUP_GENRE_EMPTY)))
	{
		return E_OUTOFMEMORY;
	}

	return S_OK;
}

HRESULT GenreGroupProvider::CreateSummaryGroup(ifc_dataobject **group)
{
	return SummaryGroup::CreateInstance((SummaryGroup**)group);
}

size_t GenreGroupProvider::ResolveName(const char *name)
{
	ValueInfo *valueInfo;

	if (NULL == name)
		return VALUEID_UNKNOWN;

	valueInfo = (ValueInfo*)bsearch(name, valueInfoMap, ARRAYSIZE(valueInfoMap), 
							sizeof(ValueInfo), GenreGroupProvider_FindValueInfoCb);

	if (NULL == valueInfo)
		return VALUEID_UNKNOWN;


	return valueInfo->id;
}

#define CBCLASS GenreGroupProvider
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
CB(API_GETNAME, GetName)
CB(API_GETDISPLAYNAME, GetDisplayName)
CB(API_BIND, Bind)
CB(API_RESOLVENAMES, ResolveNames)
CB(API_CREATEGROUP, CreateGroup)
CB(API_COMPAREOBJECTS, CompareObjects)
CB(API_GETEMPTYTEXT, GetEmptyText)
CB(API_GETCOUNTERTEXT, GetCounterText)
CB(API_CREATESUMMARYGROUP, CreateSummaryGroup)
END_DISPATCH;
#undef CBCLASS