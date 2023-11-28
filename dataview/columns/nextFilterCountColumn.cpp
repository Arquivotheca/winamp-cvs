#include "main.h"
#include "./nextFilterCountColumn.h"
#include "../ifc_dataprovider.h"

NextFilterCountColumn::NextFilterCountColumn(size_t _valueId, ifc_viewcolumninfo *_columnInfo)
	: IntegerColumn(_valueId, _columnInfo, L"", NULL)
{
}
	
NextFilterCountColumn::~NextFilterCountColumn()
{
}

HRESULT NextFilterCountColumn::CreateInfo(ifc_viewcolumninfo **instance)
{
	return ColumnInfo::CreateInstance("NextFilterCount", 
		  							  MAKEINTRESOURCE(IDS_COLUMN_NEXT_FILTER_COUNT), 
									  70, 
									  COLUMN_WIDTH_MIN_DLU, 
									  0, 
									  ifc_viewcolumninfo::AlignMode_Right,
									  "Primary,A",
									  NextFilterCountColumn::CreateInstance, 
									  (ColumnInfo**)instance);
}

HRESULT NextFilterCountColumn::CreateInstance(ifc_viewcolumninfo *columnInfo, ifc_dataprovider *provider, 
									 ifc_viewcolumn **instance)
{
	HRESULT hr;
	size_t valueId;
	const char *valueName = "NextFilterCount";

	if (NULL == instance)
		return E_POINTER;

	*instance = NULL;

	if (NULL == provider)
		return E_INVALIDARG;

	hr = provider->ResolveNames(&valueName, 1, &valueId);
	if (FAILED(hr))
		return hr;
	
	*instance = new (std::nothrow) NextFilterCountColumn(valueId, columnInfo);
	if (NULL == *instance)
		return E_OUTOFMEMORY;

	return S_OK;
}
