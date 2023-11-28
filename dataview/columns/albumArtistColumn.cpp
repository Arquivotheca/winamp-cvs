#include "main.h"
#include "./albumArtistColumn.h"
#include "../ifc_dataprovider.h"

AlbumArtistColumn::AlbumArtistColumn(size_t _valueId, ifc_viewcolumninfo *_columnInfo)
	: Column(_valueId, _columnInfo)
{
}
	
AlbumArtistColumn::~AlbumArtistColumn()
{
}

HRESULT AlbumArtistColumn::CreateInfo(ifc_viewcolumninfo **instance)
{
	return ColumnInfo::CreateInstance("AlbumArtist", 
		  							  MAKEINTRESOURCE(IDS_COLUMN_ALBUM_ARTIST), 
									  170, 
									  COLUMN_WIDTH_MIN_DLU, 
									  0, 
									  ifc_viewcolumninfo::AlignMode_Left,
									  "Album,A|Disc#|Track#",
									  AlbumArtistColumn::CreateInstance, 
									  (ColumnInfo**)instance);
}

HRESULT AlbumArtistColumn::CreateInstance(ifc_viewcolumninfo *columnInfo, ifc_dataprovider *provider, 
									 ifc_viewcolumn **instance)
{
	HRESULT hr;
	size_t valueId;
	const char *valueName = "AlbumArtist";

	if (NULL == instance)
		return E_POINTER;

	*instance = NULL;

	if (NULL == provider)
		return E_INVALIDARG;

	hr = provider->ResolveNames(&valueName, 1, &valueId);
	if (FAILED(hr))
		return hr;
	
	*instance = new (std::nothrow) AlbumArtistColumn(valueId, columnInfo);
	if (NULL == *instance)
		return E_OUTOFMEMORY;

	return S_OK;
}