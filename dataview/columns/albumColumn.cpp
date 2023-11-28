#include "main.h"
#include "./albumColumn.h"
#include "../ifc_dataprovider.h"

AlbumColumn::AlbumColumn(size_t _valueId, ifc_viewcolumninfo *_columnInfo)
	: Column(_valueId, _columnInfo)
{
}
	

AlbumColumn::~AlbumColumn()
{
}

HRESULT AlbumColumn::CreateInfo(ifc_viewcolumninfo **instance)
{
	return ColumnInfo::CreateInstance("Album", 
		  							  MAKEINTRESOURCE(IDS_COLUMN_ALBUM), 
									  170, 
									  COLUMN_WIDTH_MIN_DLU, 
									  0, 
									  ifc_viewcolumninfo::AlignMode_Left,
									  "AlbumArtist,A|Disc#|Track#|Title",
									  AlbumColumn::CreateInstance, 
									  (ColumnInfo**)instance);
}

HRESULT AlbumColumn::CreateInstance(ifc_viewcolumninfo *columnInfo, ifc_dataprovider *provider, 
									 ifc_viewcolumn **instance)
{
	HRESULT hr;
	size_t valueId;
	const char *valueName = "Album";

	if (NULL == instance)
		return E_POINTER;

	*instance = NULL;

	if (NULL == provider)
		return E_INVALIDARG;

	hr = provider->ResolveNames(&valueName, 1, &valueId);
	if (FAILED(hr))
		return hr;
	
	*instance = new (std::nothrow) AlbumColumn(valueId, columnInfo);
	if (NULL == *instance)
		return E_OUTOFMEMORY;

	return S_OK;
}
