#ifndef _NULLSOFT_WINAMP_DATAVIEW_VIEW_COLUMN_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_VIEW_COLUMN_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_viewcolumn.h"
#include "./ifc_viewcolumninfo.h"
#include "./lengthUnit.h"


class ViewColumn
{
protected:
	ViewColumn(const char *name);
	~ViewColumn();

public:
	static HRESULT CreateInstance(const char *name, ViewColumn **instance);

public:
	size_t AddRef();
	size_t Release();

	const char *GetName();

	HRESULT SetBase(ifc_viewcolumn *base);
	HRESULT GetBase(ifc_viewcolumn **base);
	
	BOOL IsDisabled();

	BOOL IsVisible();
	HRESULT SetVisible(BOOL visible);

	HRESULT GetWidth(LengthUnit *length);
	HRESULT SetWidth(const LengthUnit *length);

	// ifc_viewcolumninfo wrappers
	HRESULT GetMinWidth(LengthUnit *length);
	HRESULT GetMaxWidth(LengthUnit *length);
	HRESULT GetDisplayName(wchar_t *buffer, size_t bufferSize);
	ifc_viewcolumninfo::AlignMode GetAlignMode();
	const char *GetSortRule();

	// ifc_viewcolumn wrappers
	HRESULT Format(LCID localeId, ifc_dataobject *object, wchar_t *buffer, size_t bufferSize);
	int Compare(LCID localeId, ifc_dataobject *object1, ifc_dataobject *object2);

protected:
	size_t ref;
	char *name;
	ifc_viewcolumn *base;
	LengthUnit width;
	BOOL visible;

};

#endif //_NULLSOFT_WINAMP_DATAVIEW_VIEW_COLUMN_HEADER