#include "main.h"
#include "./artistColumn.h"
#include "../ifc_dataprovider.h"

ArtistColumn::ArtistColumn(size_t _valueId, ifc_viewcolumninfo *_columnInfo)
	: Column(_valueId, _columnInfo)
{
}
	
ArtistColumn::~ArtistColumn()
{
}

HRESULT ArtistColumn::CreateInfo(ifc_viewcolumninfo **instance)
{
	return ColumnInfo::CreateInstance("Artist", 
		  							  MAKEINTRESOURCE(IDS_COLUMN_ARTIST), 
									  170, 
									  COLUMN_WIDTH_MIN_DLU, 
									  0, 
									  ifc_viewcolumninfo::AlignMode_Left,
									  "Album,A|Disc#|Track#|Title",
									  ArtistColumn::CreateInstance, 
									  (ColumnInfo**)instance);
}

HRESULT ArtistColumn::CreateInstance(ifc_viewcolumninfo *columnInfo, ifc_dataprovider *provider, 
									 ifc_viewcolumn **instance)
{
	HRESULT hr;
	size_t valueId;
	const char *valueName = "Artist";

	if (NULL == instance)
		return E_POINTER;

	*instance = NULL;

	if (NULL == provider)
		return E_INVALIDARG;

	hr = provider->ResolveNames(&valueName, 1, &valueId);
	if (FAILED(hr))
		return hr;
	
	*instance = new (std::nothrow) ArtistColumn(valueId, columnInfo);
	if (NULL == *instance)
		return E_OUTOFMEMORY;

	return S_OK;
}