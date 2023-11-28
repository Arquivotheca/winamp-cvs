#include "main.h"
#include "./composerColumn.h"
#include "../ifc_dataprovider.h"

ComposerColumn::ComposerColumn(size_t _valueId, ifc_viewcolumninfo *_columnInfo)
	: Column(_valueId, _columnInfo)
{
}
	
ComposerColumn::~ComposerColumn()
{
}

HRESULT ComposerColumn::CreateInfo(ifc_viewcolumninfo **instance)
{
	return ColumnInfo::CreateInstance("Composer", 
		  							  MAKEINTRESOURCE(IDS_COLUMN_COMPOSER), 
									  170, 
									  COLUMN_WIDTH_MIN_DLU, 
									  0, 
									  ifc_viewcolumninfo::AlignMode_Left,
									  "Album,A|Disc#|Track#",
									  ComposerColumn::CreateInstance, 
									  (ColumnInfo**)instance);
}

HRESULT ComposerColumn::CreateInstance(ifc_viewcolumninfo *columnInfo, ifc_dataprovider *provider, 
									 ifc_viewcolumn **instance)
{
	HRESULT hr;
	size_t valueId;
	const char *valueName = "Composer";

	if (NULL == instance)
		return E_POINTER;

	*instance = NULL;

	if (NULL == provider)
		return E_INVALIDARG;

	hr = provider->ResolveNames(&valueName, 1, &valueId);
	if (FAILED(hr))
		return hr;
	
	*instance = new (std::nothrow) ComposerColumn(valueId, columnInfo);
	if (NULL == *instance)
		return E_OUTOFMEMORY;

	return S_OK;
}