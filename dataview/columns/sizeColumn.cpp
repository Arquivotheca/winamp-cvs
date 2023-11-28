#include "main.h"
#include "./sizeColumn.h"
#include "../ifc_dataprovider.h"

SizeColumn::SizeColumn(size_t _valueId, ifc_viewcolumninfo *_columnInfo)
	: Column(_valueId, _columnInfo)
{
}
	
SizeColumn::~SizeColumn()
{
}

HRESULT SizeColumn::CreateInfo(ifc_viewcolumninfo **instance)
{
	return ColumnInfo::CreateInstance("Size", 
		  							  MAKEINTRESOURCE(IDS_COLUMN_SIZE), 
									  170, 
									  COLUMN_WIDTH_MIN_DLU, 
									  0, 
									  ifc_viewcolumninfo::AlignMode_Right,
									  "Artist,A|Album|Disc#|Track#",
									  SizeColumn::CreateInstance, 
									  (ColumnInfo**)instance);
}

HRESULT SizeColumn::CreateInstance(ifc_viewcolumninfo *columnInfo, ifc_dataprovider *provider, 
									 ifc_viewcolumn **instance)
{
	HRESULT hr;
	size_t valueId;
	const char *valueName = "Size";

	if (NULL == instance)
		return E_POINTER;

	*instance = NULL;

	if (NULL == provider)
		return E_INVALIDARG;

	hr = provider->ResolveNames(&valueName, 1, &valueId);
	if (FAILED(hr))
		return hr;
	
	*instance = new (std::nothrow) SizeColumn(valueId, columnInfo);
	if (NULL == *instance)
		return E_OUTOFMEMORY;

	return S_OK;
}

HRESULT SizeColumn::Format(LCID localeId, ifc_dataobject *object, wchar_t *buffer, size_t bufferSize)
{
	HRESULT hr;
	DataValue value;

	if (NULL == object)
		return E_INVALIDARG;

	DATAVALUE_SET_TYPE(&value, DT_UINT64);
	
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
			hr = Format_FileSize(buffer, bufferSize, localeId, DATAVALUE_GET_UINT64(&value), 4, 1);
	}

	DATAVALUE_CLEAR(&value);

	return hr;
}