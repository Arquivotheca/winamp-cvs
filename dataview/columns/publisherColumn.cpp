#include "main.h"
#include "./publisherColumn.h"
#include "../ifc_dataprovider.h"

PublisherColumn::PublisherColumn(size_t _valueId, ifc_viewcolumninfo *_columnInfo)
	: Column(_valueId, _columnInfo)
{
}
	
PublisherColumn::~PublisherColumn()
{
}

HRESULT PublisherColumn::CreateInfo(ifc_viewcolumninfo **instance)
{
	return ColumnInfo::CreateInstance("Publisher", 
		  							  MAKEINTRESOURCE(IDS_COLUMN_PUBLISHER), 
									  170, 
									  COLUMN_WIDTH_MIN_DLU, 
									  0, 
									  ifc_viewcolumninfo::AlignMode_Left,
									  "Artist,A|Album|Disc#|Track#|Title",
									  PublisherColumn::CreateInstance, 
									  (ColumnInfo**)instance);
}

HRESULT PublisherColumn::CreateInstance(ifc_viewcolumninfo *columnInfo, ifc_dataprovider *provider, 
									 ifc_viewcolumn **instance)
{
	HRESULT hr;
	size_t valueId;
	const char *valueName = "Publisher";

	if (NULL == instance)
		return E_POINTER;

	*instance = NULL;

	if (NULL == provider)
		return E_INVALIDARG;

	hr = provider->ResolveNames(&valueName, 1, &valueId);
	if (FAILED(hr))
		return hr;
	
	*instance = new (std::nothrow) PublisherColumn(valueId, columnInfo);
	if (NULL == *instance)
		return E_OUTOFMEMORY;

	return S_OK;
}