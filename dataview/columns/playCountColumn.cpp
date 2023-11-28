#include "main.h"
#include "./playCountColumn.h"
#include "../ifc_dataprovider.h"

PlayCountColumn::PlayCountColumn(size_t _valueId, ifc_viewcolumninfo *_columnInfo)
	: IntegerColumn(_valueId, _columnInfo, L"", L"")
{
}
	
PlayCountColumn::~PlayCountColumn()
{
}

HRESULT PlayCountColumn::CreateInfo(ifc_viewcolumninfo **instance)
{
	return ColumnInfo::CreateInstance("PlayCount", 
		  							  MAKEINTRESOURCE(IDS_COLUMN_PLAY_COUNT), 
									  170, 
									  COLUMN_WIDTH_MIN_DLU, 
									  0, 
									  ifc_viewcolumninfo::AlignMode_Right,
									  "LastPlayed|Artist,A|Album|Disc#|Track#",
									  PlayCountColumn::CreateInstance, 
									  (ColumnInfo**)instance);
}

HRESULT PlayCountColumn::CreateInstance(ifc_viewcolumninfo *columnInfo, ifc_dataprovider *provider, 
									 ifc_viewcolumn **instance)
{
	HRESULT hr;
	size_t valueId;
	const char *valueName = "PlayCount";

	if (NULL == instance)
		return E_POINTER;

	*instance = NULL;

	if (NULL == provider)
		return E_INVALIDARG;

	hr = provider->ResolveNames(&valueName, 1, &valueId);
	if (FAILED(hr))
		return hr;
	
	*instance = new (std::nothrow) PlayCountColumn(valueId, columnInfo);
	if (NULL == *instance)
		return E_OUTOFMEMORY;

	return S_OK;
}
