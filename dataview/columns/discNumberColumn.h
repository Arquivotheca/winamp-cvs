#ifndef _NULLSOFT_WINAMP_DATAVIEW_DISC_NUMBER_COLUMN_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_DISC_NUMBER_COLUMN_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./integerColumn.h"


class DiscNumberColumn : public IntegerColumn
{
protected:
	DiscNumberColumn(size_t valueId, ifc_viewcolumninfo *columnInfo);
	~DiscNumberColumn();

public:
	static HRESULT CreateInfo(ifc_viewcolumninfo **instance);

	static HRESULT CreateInstance(ifc_viewcolumninfo *columnInfo,
								  ifc_dataprovider *provider,
								  ifc_viewcolumn **instance);
};


#endif //_NULLSOFT_WINAMP_DATAVIEW_DISC_NUMBER_COLUMN_HEADER