#include "main.h"
#include "./yearColumn.h"
#include "../ifc_dataprovider.h"

YearColumn::YearColumn(size_t _valueId, ifc_viewcolumninfo *_columnInfo)
	: Column(_valueId, _columnInfo)
{
}
	
YearColumn::~YearColumn()
{
}

HRESULT YearColumn::CreateInfo(ifc_viewcolumninfo **instance)
{
	return ColumnInfo::CreateInstance("Year", 
		  							  MAKEINTRESOURCE(IDS_COLUMN_YEAR), 
									  170, 
									  COLUMN_WIDTH_MIN_DLU, 
									  0, 
									  ifc_viewcolumninfo::AlignMode_Right,
									  "Artist,A|Album|Disc#|Track#",
									  YearColumn::CreateInstance, 
									  (ColumnInfo**)instance);
}

HRESULT YearColumn::CreateInstance(ifc_viewcolumninfo *columnInfo, ifc_dataprovider *provider, 
									 ifc_viewcolumn **instance)
{
	HRESULT hr;
	size_t valueId;
	const char *valueName = "Year";

	if (NULL == instance)
		return E_POINTER;

	*instance = NULL;

	if (NULL == provider)
		return E_INVALIDARG;

	hr = provider->ResolveNames(&valueName, 1, &valueId);
	if (FAILED(hr))
		return hr;
	
	*instance = new (std::nothrow) YearColumn(valueId, columnInfo);
	if (NULL == *instance)
		return E_OUTOFMEMORY;

	return S_OK;
}

HRESULT YearColumn::Format(LCID localeId, ifc_dataobject *object, wchar_t *buffer, size_t bufferSize)
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
		{
			int year;
			SYSTEMTIME systemTime;

			year = DATAVALUE_GET_INT32(&value);
			if (year <= 0)
			{
				if (NULL == buffer)
					hr = E_POINTER;
				else
					*buffer = L'\0';
			}
			else		
			{
				systemTime.wYear = (unsigned short)year;
				systemTime.wMonth = 1;
				systemTime.wDayOfWeek = 0;
				systemTime.wDay = 1;
				systemTime.wHour = 0;
				systemTime.wMinute = 0;
				systemTime.wSecond = 0;
				systemTime.wMilliseconds = 0;

				hr = Format_Year(buffer, bufferSize, localeId, &systemTime, FALSE);
			}
		}
	}

	DATAVALUE_CLEAR(&value);

	return hr;
}