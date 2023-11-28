#include "main.h"
#include "./ratingColumn.h"
#include "../ifc_dataprovider.h"

RatingColumn::RatingColumn(size_t _valueId, ifc_viewcolumninfo *_columnInfo)
	: Column(_valueId, _columnInfo)
{
}
	
RatingColumn::~RatingColumn()
{
}

HRESULT RatingColumn::CreateInfo(ifc_viewcolumninfo **instance)
{
	return ColumnInfo::CreateInstance("Rating", 
		  							  MAKEINTRESOURCE(IDS_COLUMN_RATING), 
									  170, 
									  COLUMN_WIDTH_MIN_DLU, 
									  0, 
									  ifc_viewcolumninfo::AlignMode_Center,
									  "PlayCount|LastPlayed|Artist,A|Album|Disc#|Track#",
									  RatingColumn::CreateInstance, 
									  (ColumnInfo**)instance);
}

HRESULT RatingColumn::CreateInstance(ifc_viewcolumninfo *columnInfo, ifc_dataprovider *provider, 
									 ifc_viewcolumn **instance)
{
	HRESULT hr;
	size_t valueId;
	const char *valueName = "Rating";

	if (NULL == instance)
		return E_POINTER;

	*instance = NULL;

	if (NULL == provider)
		return E_INVALIDARG;

	hr = provider->ResolveNames(&valueName, 1, &valueId);
	if (FAILED(hr))
		return hr;
	
	*instance = new (std::nothrow) RatingColumn(valueId, columnInfo);
	if (NULL == *instance)
		return E_OUTOFMEMORY;

	return S_OK;
}

HRESULT RatingColumn::Format(LCID localeId, ifc_dataobject *object, wchar_t *buffer, size_t bufferSize)
{
	HRESULT hr;
	DataValue value;

	if (NULL == object)
		return E_INVALIDARG;

	DATAVALUE_SET_TYPE(&value, DT_INT32);
	
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
			hr = Format_Rating(buffer, bufferSize, localeId, DATAVALUE_GET_INT32(&value));
	}

	DATAVALUE_CLEAR(&value);

	return hr;
}