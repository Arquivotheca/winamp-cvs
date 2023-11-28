#include "main.h"
#include "./integerColumn.h"

#include <strsafe.h>

IntegerColumn::IntegerColumn(size_t _valueId, ifc_viewcolumninfo *_columnInfo, 
							 const wchar_t *_replaceUnknown, const wchar_t *_replaceZero)
	: Column(_valueId, _columnInfo), replaceZero(NULL), replaceUnknown(NULL)
{
	
	replaceZero = ResourceString_Duplicate(_replaceZero);

	if (NULL == _replaceUnknown)
		_replaceUnknown = L"";

	replaceUnknown = ResourceString_Duplicate(_replaceUnknown);
	
	
}
	
IntegerColumn::~IntegerColumn()
{
	ResourceString_Free(replaceZero);
	ResourceString_Free(replaceUnknown);
}


HRESULT IntegerColumn::Format(LCID localeId, ifc_dataobject *object, wchar_t *buffer, size_t bufferSize)
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
			hr = StringCchCopy(buffer, bufferSize, replaceUnknown);
		}
		else
		{
			if (0 == DATAVALUE_GET_INT32(&value) && NULL != replaceZero)
				hr = StringCchCopy(buffer, bufferSize, replaceZero);
			else
				hr = Format_NumberInt(buffer, bufferSize, localeId, DATAVALUE_GET_INT32(&value), FALSE);
		}
	}
	
	DATAVALUE_CLEAR(&value);
	return hr;
}