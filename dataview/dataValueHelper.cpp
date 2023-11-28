#include "main.h"
#include "./dataValueHelper.h"


HRESULT 
DataValue_InitStackWstrBuffer(DataValue *value, size_t size)
{
	if (NULL == value)
		return E_POINTER;

	DATAVALUE_SET_TYPE(value, DT_WSTR_BUFFER);
	value->wstrBuffer.data = (wchar_t*)_malloca(sizeof(wchar_t) * size);
	if (NULL == value->wstrBuffer.data)
	{
		DATAVALUE_SET_TYPE(value, DT_EMPTY);
		return E_OUTOFMEMORY;
	}

	value->wstrBuffer.size = size;
	return S_OK;
}

HRESULT 
DataValue_FreeStackWstrBuffer(DataValue *value)
{
	if (NULL == value)
		return E_POINTER;

	if (DT_WSTR_BUFFER == value->type)
	{
		_freea(value->wstrBuffer.data);

		value->wstrBuffer.data = NULL;
		value->wstrBuffer.size = 0;
		
		DATAVALUE_SET_TYPE(value, DT_EMPTY);
	}
		
	return S_OK;
}