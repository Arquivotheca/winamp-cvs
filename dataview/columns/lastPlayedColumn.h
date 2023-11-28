#ifndef _NULLSOFT_WINAMP_DATAVIEW_LAST_PLAYED_COLUMN_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_LAST_PLAYED_COLUMN_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./column.h"


class LastPlayedColumn : public Column
{
protected:
	LastPlayedColumn(size_t valueId, ifc_viewcolumninfo *columnInfo);
	~LastPlayedColumn();

public:
	static HRESULT CreateInfo(ifc_viewcolumninfo **instance);

	static HRESULT CreateInstance(ifc_viewcolumninfo *columnInfo,
								  ifc_dataprovider *provider,
								  ifc_viewcolumn **instance);

	HRESULT Format(LCID localeId, ifc_dataobject *object, wchar_t *buffer, size_t bufferSize);
};


#endif //_NULLSOFT_WINAMP_DATAVIEW_LAST_PLAYED_COLUMN_HEADER