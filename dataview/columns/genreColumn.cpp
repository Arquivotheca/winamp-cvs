#include "main.h"
#include "./genreColumn.h"
#include "../ifc_dataprovider.h"

GenreColumn::GenreColumn(size_t _valueId, ifc_viewcolumninfo *_columnInfo)
	: Column(_valueId, _columnInfo)
{
}
	
GenreColumn::~GenreColumn()
{
}

HRESULT GenreColumn::CreateInfo(ifc_viewcolumninfo **instance)
{
	return ColumnInfo::CreateInstance("Genre", 
		  							  MAKEINTRESOURCE(IDS_COLUMN_GENRE), 
									  170, 
									  COLUMN_WIDTH_MIN_DLU, 
									  0, 
									  ifc_viewcolumninfo::AlignMode_Left,
									  "AlbumArtist,A|Artist|Album|Disc#|Track#",
									  GenreColumn::CreateInstance, 
									  (ColumnInfo**)instance);
}

HRESULT GenreColumn::CreateInstance(ifc_viewcolumninfo *columnInfo, ifc_dataprovider *provider, 
									 ifc_viewcolumn **instance)
{
	HRESULT hr;
	size_t valueId;
	const char *valueName = "Genre";

	if (NULL == instance)
		return E_POINTER;

	*instance = NULL;

	if (NULL == provider)
		return E_INVALIDARG;

	hr = provider->ResolveNames(&valueName, 1, &valueId);
	if (FAILED(hr))
		return hr;
	
	*instance = new (std::nothrow) GenreColumn(valueId, columnInfo);
	if (NULL == *instance)
		return E_OUTOFMEMORY;

	return S_OK;
}