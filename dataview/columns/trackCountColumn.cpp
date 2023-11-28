#include "main.h"
#include "./trackCountColumn.h"
#include "../ifc_dataprovider.h"

TrackCountColumn::TrackCountColumn(size_t _valueId, ifc_viewcolumninfo *_columnInfo)
	: IntegerColumn(_valueId, _columnInfo, L"", NULL)
{
}
	
TrackCountColumn::~TrackCountColumn()
{
}

HRESULT TrackCountColumn::CreateInfo(ifc_viewcolumninfo **instance)
{
	return ColumnInfo::CreateInstance("TrackCount", 
		  							  MAKEINTRESOURCE(IDS_COLUMN_TRACK_COUNT), 
									  50, 
									  COLUMN_WIDTH_MIN_DLU, 
									  0, 
									  ifc_viewcolumninfo::AlignMode_Right,
									  "Primary, A",
									  TrackCountColumn::CreateInstance, 
									  (ColumnInfo**)instance);
}

HRESULT TrackCountColumn::CreateInstance(ifc_viewcolumninfo *columnInfo, ifc_dataprovider *provider, 
									 ifc_viewcolumn **instance)
{
	HRESULT hr;
	size_t valueId;
	const char *valueName = "TrackCount";

	if (NULL == instance)
		return E_POINTER;

	*instance = NULL;

	if (NULL == provider)
		return E_INVALIDARG;

	hr = provider->ResolveNames(&valueName, 1, &valueId);
	if (FAILED(hr))
		return hr;
	
	*instance = new (std::nothrow) TrackCountColumn(valueId, columnInfo);
	if (NULL == *instance)
		return E_OUTOFMEMORY;

	return S_OK;
}
