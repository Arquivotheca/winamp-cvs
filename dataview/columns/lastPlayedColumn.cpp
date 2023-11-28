#include "main.h"
#include "./lastPlayedColumn.h"
#include "../ifc_dataprovider.h"

LastPlayedColumn::LastPlayedColumn(size_t _valueId, ifc_viewcolumninfo *_columnInfo)
	: Column(_valueId, _columnInfo)
{
}
	
LastPlayedColumn::~LastPlayedColumn()
{
}

HRESULT LastPlayedColumn::CreateInfo(ifc_viewcolumninfo **instance)
{
	return ColumnInfo::CreateInstance("LastPlayed", 
		  							  MAKEINTRESOURCE(IDS_COLUMN_LAST_PLAYED), 
									  170, 
									  COLUMN_WIDTH_MIN_DLU, 
									  0, 
									  ifc_viewcolumninfo::AlignMode_Right,
									  "Artist,A|Album|Disc#|Track#",
									  LastPlayedColumn::CreateInstance, 
									  (ColumnInfo**)instance);
}

HRESULT LastPlayedColumn::CreateInstance(ifc_viewcolumninfo *columnInfo, ifc_dataprovider *provider, 
									 ifc_viewcolumn **instance)
{
	HRESULT hr;
	size_t valueId;
	const char *valueName = "LastPlayed";

	if (NULL == instance)
		return E_POINTER;

	*instance = NULL;

	if (NULL == provider)
		return E_INVALIDARG;

	hr = provider->ResolveNames(&valueName, 1, &valueId);
	if (FAILED(hr))
		return hr;
	
	*instance = new (std::nothrow) LastPlayedColumn(valueId, columnInfo);
	if (NULL == *instance)
		return E_OUTOFMEMORY;

	return S_OK;
}

HRESULT LastPlayedColumn::Format(LCID localeId, ifc_dataobject *object, wchar_t *buffer, size_t bufferSize)
{
	HRESULT hr;
	DataValue value;

	if (NULL == object)
		return E_INVALIDARG;

	DATAVALUE_SET_TYPE(&value, DT_TIME64);
	
	hr = object->GetValue(localeId, valueId, &value);
	if (SUCCEEDED(hr))
	{
		if (S_FALSE == hr)
		{
			if (NULL == buffer)
				hr = E_POINTER;
			else
			{
				buffer[0] = L'\0';
				hr = S_OK;
			}

		}
		else
			hr = Format_DateTime(buffer, bufferSize, localeId, DATAVALUE_GET_TIME64(&value));
	}

	DATAVALUE_CLEAR(&value);

	return hr;
}