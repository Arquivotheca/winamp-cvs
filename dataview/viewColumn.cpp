#include "main.h"
#include "./viewColumn.h"

#include <float.h>


ViewColumn::ViewColumn(const char *_name)
	: ref(1), name(NULL), base(NULL), visible(TRUE)
{
	LengthUnit_Set(&width, FLT_MAX, UnitType_Pixel);
	name = AnsiString_Duplicate(_name);
}

ViewColumn::~ViewColumn()
{
	SafeRelease(base);
	AnsiString_Free(name);
}

HRESULT ViewColumn::CreateInstance(const char *name, ViewColumn **instance)
{
	if (NULL == instance)
		return E_POINTER;

	if (NULL == name)
		return E_INVALIDARG;

	*instance = new (std::nothrow) ViewColumn(name);
	if (NULL == (*instance))
		return E_OUTOFMEMORY;

	return S_OK;
}

size_t ViewColumn::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t ViewColumn::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

const char *ViewColumn::GetName()
{
	return name;
}

HRESULT ViewColumn::SetBase(ifc_viewcolumn *_base)
{
	HRESULT result;
	ifc_viewcolumninfo *columnInfo;

	SafeRelease(base);

	base = _base;
	if (NULL == base)
		return S_OK;

	base->AddRef();
	result = E_UNEXPECTED;

	if (SUCCEEDED(base->GetInfo(&columnInfo)))
	{
		if (0 == ColumnInfo_CompareNames(name, -1, columnInfo->GetName(), -1))
			result = S_OK;

		columnInfo->Release();
	}

	if (S_OK != result)
	{
		base->Release();
		base = NULL;
	}

	return result;
}

HRESULT ViewColumn::GetBase(ifc_viewcolumn **_base)
{
	if (NULL == _base)
		return E_POINTER;

	*_base = base;
	
	if (NULL != base)
		base->AddRef();

	return S_OK;
}

BOOL ViewColumn::IsDisabled()
{
	return (NULL == base);
}

BOOL ViewColumn::IsVisible()
{
	return (NULL != base) ? visible : FALSE;
}

HRESULT ViewColumn::SetVisible(BOOL _visible)
{
	visible = (FALSE != _visible);
	return S_OK;
}

HRESULT ViewColumn::GetWidth(LengthUnit *length)
{
	if (FLT_MAX == width.value && UnitType_Pixel == width.type)
	{
		long lWidth;
		HRESULT hr;
		ifc_viewcolumninfo *columnInfo;

		if (NULL == base)
			return E_FAIL;
			
		hr = base->GetInfo(&columnInfo);
		if (FAILED(hr))
			return hr;
		
		hr = columnInfo->GetWidth(&lWidth);
		if (SUCCEEDED(hr))
		{
			width.value = (float)abs(lWidth);
			width.type = (lWidth > 0) ? UnitType_Pixel : UnitType_Dlu;
		}

		columnInfo->Release();
		
		if (FAILED(hr))
			return hr;
	}

	if (FALSE == LengthUnit_Copy(length, &width))
		return E_POINTER;

	return S_OK;
}

HRESULT ViewColumn::SetWidth(const LengthUnit *length)
{
	if (FALSE == LengthUnit_Copy(&width, length))
		return E_POINTER;

	return S_OK;
}

HRESULT ViewColumn::GetMinWidth(LengthUnit *length)
{
	ifc_viewcolumninfo *columnInfo;
	long lWidth;
	HRESULT hr;

	if (NULL == length)
		return E_POINTER;
	
	if (NULL == base)
		return E_UNEXPECTED;

	hr = base->GetInfo(&columnInfo);
	if (FAILED(hr))
		return hr;
	
	hr = columnInfo->GetMinWidth(&lWidth);
	if (SUCCEEDED(hr))
	{
		length->value = (float)abs(lWidth);
		length->type = (lWidth > 0) ? UnitType_Pixel : UnitType_Dlu;
	}		
	
	columnInfo->Release();

	return hr;
}

HRESULT ViewColumn::GetMaxWidth(LengthUnit *length)
{
	ifc_viewcolumninfo *columnInfo;
	long lWidth;
	HRESULT hr;

	if (NULL == length)
		return E_POINTER;
	
	if (NULL == base)
		return E_UNEXPECTED;

	hr = base->GetInfo(&columnInfo);
	if (FAILED(hr))
		return hr;
	
	hr = columnInfo->GetMaxWidth(&lWidth);
	if (SUCCEEDED(hr))
	{
		length->value = (float)abs(lWidth);
		length->type = (lWidth > 0) ? UnitType_Pixel : UnitType_Dlu;
	}		
	
	columnInfo->Release();

	return hr;
}

HRESULT ViewColumn::GetDisplayName(wchar_t *buffer, size_t bufferSize)
{
	HRESULT hr;
	ifc_viewcolumninfo *columnInfo;

	if (NULL == base)
		return E_UNEXPECTED;

	hr = base->GetInfo(&columnInfo);
	if (FAILED(hr))
		return hr;

	hr = columnInfo->GetDisplayName(buffer, bufferSize);

	columnInfo->Release();

	return hr;
}

ifc_viewcolumninfo::AlignMode ViewColumn::GetAlignMode()
{
	ifc_viewcolumninfo *columnInfo;
	ifc_viewcolumninfo::AlignMode alignMode;

	if (NULL == base || FAILED(base->GetInfo(&columnInfo)))
		return ifc_viewcolumninfo::AlignMode_Default;

	alignMode = columnInfo->GetAlignMode();

	columnInfo->Release();

	return alignMode;
}

const char *ViewColumn::GetSortRule()
{
	ifc_viewcolumninfo *columnInfo;
	const char *sortRule;

	if (NULL == base || FAILED(base->GetInfo(&columnInfo)))
		return NULL;

	sortRule = columnInfo->GetSortRule();

	columnInfo->Release();

	return sortRule;
}

HRESULT ViewColumn::Format(LCID localeId, ifc_dataobject *object, wchar_t *buffer, size_t bufferSize)
{
	if (NULL == base)
		return E_UNEXPECTED;

	return base->Format(localeId, object, buffer, bufferSize);
}

int ViewColumn::Compare(LCID localeId, ifc_dataobject *object1, ifc_dataobject *object2)
{
	if (NULL == base)
		return COBJ_ERROR;

	return base->Compare(localeId, object1, object2);
}