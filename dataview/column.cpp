#include "main.h"
#include "./column.h"
#include "./ifc_dataobject.h"

Column::Column(size_t _valueId, ifc_viewcolumninfo *_columnInfo)
	: ref(1), valueId(_valueId), columnInfo(_columnInfo)
{
	if (NULL != columnInfo)
		columnInfo->AddRef();
}
	

Column::~Column()
{
	SafeRelease(columnInfo);
}

size_t Column::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t Column::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int Column::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) 
		return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_ViewColumn))
		*object = static_cast<ifc_viewcolumn*>(this);
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

HRESULT Column::GetInfo(ifc_viewcolumninfo **info)
{
	if (NULL == info)
		return E_POINTER;
	
	*info = columnInfo;
	if (NULL != columnInfo)
		columnInfo->AddRef();

	return S_OK;
}

HRESULT Column::Format(LCID localeId, ifc_dataobject *object, wchar_t *buffer, size_t bufferSize)
{
	HRESULT hr;
	DataValue value;

	if (NULL == object)
		return E_INVALIDARG;

	DATAVALUE_INIT_WSTR_BUFFER(&value, buffer, bufferSize);
	hr = object->GetValue(localeId, valueId, &value);
	if (S_FALSE == hr)
	{
		if (NULL == buffer)
			return E_POINTER;

		buffer[0] = L'\0';
		hr = S_OK;
	}

	DATAVALUE_CLEAR(&value);
	return hr;
}

int Column::Compare(LCID localeId, ifc_dataobject *object1, ifc_dataobject *object2)
{
	if (NULL == object1)
		return 0;

	return object1->CompareTo(localeId, valueId, object2);
}

#define CBCLASS Column
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
CB(API_GETINFO, GetInfo)
CB(API_FORMAT, Format)
CB(API_COMPARE, Compare)
END_DISPATCH;
#undef CBCLASS