#ifndef _NULLSOFT_WINAMP_DATAVIEW_TRACK_COUNT_COLUMN_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_TRACK_COUNT_COLUMN_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./integerColumn.h"


class TrackCountColumn : public IntegerColumn
{
protected:
	TrackCountColumn(size_t valueId, ifc_viewcolumninfo *columnInfo);
	~TrackCountColumn();

public:
	static HRESULT CreateInfo(ifc_viewcolumninfo **instance);

	static HRESULT CreateInstance(ifc_viewcolumninfo *columnInfo,
								  ifc_dataprovider *provider,
								  ifc_viewcolumn **instance);

};


#endif //_NULLSOFT_WINAMP_DATAVIEW_TRACK_COUNT_COLUMN_HEADER