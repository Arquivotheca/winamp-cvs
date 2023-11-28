#include "main.h"
#include "./trackNumberColumn.h"
#include "../ifc_dataprovider.h"

TrackNumberColumn::TrackNumberColumn(size_t _valueId, ifc_viewcolumninfo *_columnInfo)
	: IntegerColumn(_valueId, _columnInfo, L"", L"")
{
}
	
TrackNumberColumn::~TrackNumberColumn()
{
}

HRESULT TrackNumberColumn::CreateInfo(ifc_viewcolumninfo **instance)
{
	return ColumnInfo::CreateInstance("Track#", 
		  							  MAKEINTRESOURCE(IDS_COLUMN_TRACK_NUMBER), 
									  50, 
									  COLUMN_WIDTH_MIN_DLU, 
									  0, 
									  ifc_viewcolumninfo::AlignMode_Right,
									  "Title,A|Artist|Album|Disc#",
									  TrackNumberColumn::CreateInstance, 
									  (ColumnInfo**)instance);
}

HRESULT TrackNumberColumn::CreateInstance(ifc_viewcolumninfo *columnInfo, ifc_dataprovider *provider, 
									 ifc_viewcolumn **instance)
{
	HRESULT hr;
	size_t valueId;
	const char *valueName = "Track#";

	if (NULL == instance)
		return E_POINTER;

	*instance = NULL;

	if (NULL == provider)
		return E_INVALIDARG;

	hr = provider->ResolveNames(&valueName, 1, &valueId);
	if (FAILED(hr))
		return hr;
	
	*instance = new (std::nothrow) TrackNumberColumn(valueId, columnInfo);
	if (NULL == *instance)
		return E_OUTOFMEMORY;

	return S_OK;
}
