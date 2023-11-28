#include "main.h"
#include "./albumArtistIndexColumn.h"
#include "../ifc_dataprovider.h"

#include <strsafe.h>

AlbumArtistIndexColumn::AlbumArtistIndexColumn(size_t _valueId, ifc_viewcolumninfo *_columnInfo)
	: Column(_valueId, _columnInfo)
{
}
	
AlbumArtistIndexColumn::~AlbumArtistIndexColumn()
{
}

HRESULT AlbumArtistIndexColumn::CreateInfo(ifc_viewcolumninfo **instance)
{
	return ColumnInfo::CreateInstance("AlbumArtistIndex", 
		  							  MAKEINTRESOURCE(IDS_COLUMN_ALBUM_ARTIST_INDEX), 
									  60, 
									  COLUMN_WIDTH_MIN_DLU, 
									  0, 
									  ifc_viewcolumninfo::AlignMode_Left,
									  "Album,A|Disc#|Track#",
									  AlbumArtistIndexColumn::CreateInstance, 
									  (ColumnInfo**)instance);
}

HRESULT AlbumArtistIndexColumn::CreateInstance(ifc_viewcolumninfo *columnInfo, ifc_dataprovider *provider, 
									 ifc_viewcolumn **instance)
{
	HRESULT hr;
	size_t valueId;
	const char *valueName = "AlbumArtistIndex";

	if (NULL == instance)
		return E_POINTER;

	*instance = NULL;

	if (NULL == provider)
		return E_INVALIDARG;

	hr = provider->ResolveNames(&valueName, 1, &valueId);
	if (FAILED(hr))
		return hr;
	
	*instance = new (std::nothrow) AlbumArtistIndexColumn(valueId, columnInfo);
	if (NULL == *instance)
		return E_OUTOFMEMORY;

	return S_OK;
}

HRESULT AlbumArtistIndexColumn::Format(LCID localeId, ifc_dataobject *object, wchar_t *buffer, size_t bufferSize)
{
	HRESULT hr;
	DataValue value;

	if (NULL == object)
		return E_INVALIDARG;

	DATAVALUE_SET_TYPE(&value, DT_WCHAR);
	hr = object->GetValue(localeId, valueId, &value);
	if (FAILED(hr))
		return hr;

	if (NULL == buffer)
		hr = E_POINTER;
	else if (bufferSize < 2)
		hr = STRSAFE_E_INSUFFICIENT_BUFFER;
	else
	{
		if (S_FALSE != hr)
			buffer[0] = DATAVALUE_GET_WCHAR(&value);
		else
		{
			buffer[0] = L'\0';
			hr = S_OK;
		}

		if (L'\0' != buffer[0])
			buffer[1] = L'\0';
	}

	DATAVALUE_CLEAR(&value);
	return hr;
}