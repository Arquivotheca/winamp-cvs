#include "main.h"
#include "./discNumberColumn.h"
#include "../ifc_dataprovider.h"

DiscNumberColumn::DiscNumberColumn(size_t _valueId, ifc_viewcolumninfo *_columnInfo)
: IntegerColumn(_valueId, _columnInfo, L"", L"")
{
}
	
DiscNumberColumn::~DiscNumberColumn()
{
}

HRESULT DiscNumberColumn::CreateInfo(ifc_viewcolumninfo **instance)
{
	return ColumnInfo::CreateInstance("Disc#", 
		  							  MAKEINTRESOURCE(IDS_COLUMN_DISC_NUMBER), 
									  50, 
									  COLUMN_WIDTH_MIN_DLU, 
									  0, 
									  ifc_viewcolumninfo::AlignMode_Right,
									  "Track#,A|Title|Artist|Album",
									  DiscNumberColumn::CreateInstance, 
									  (ColumnInfo**)instance);
}

HRESULT DiscNumberColumn::CreateInstance(ifc_viewcolumninfo *columnInfo, ifc_dataprovider *provider, 
									 ifc_viewcolumn **instance)
{
	HRESULT hr;
	size_t valueId;
	const char *valueName = "Disc#";

	if (NULL == instance)
		return E_POINTER;

	*instance = NULL;

	if (NULL == provider)
		return E_INVALIDARG;

	hr = provider->ResolveNames(&valueName, 1, &valueId);
	if (FAILED(hr))
		return hr;
	
	*instance = new (std::nothrow) DiscNumberColumn(valueId, columnInfo);
	if (NULL == *instance)
		return E_OUTOFMEMORY;

	return S_OK;
}