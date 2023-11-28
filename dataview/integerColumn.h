#ifndef _NULLSOFT_WINAMP_DATAVIEW_INTEGER_COLUMN_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_INTEGER_COLUMN_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./column.h"


class _declspec(novtable) IntegerColumn : public Column
{

protected:
	IntegerColumn(size_t valueId, ifc_viewcolumninfo *columnInfo, 
				  const wchar_t *replaceUnknown,
				  const wchar_t *replaceZero);
				  

	virtual ~IntegerColumn();

public:
	HRESULT Format(LCID localeId, ifc_dataobject *object, wchar_t *buffer, size_t bufferSize);

private:
	wchar_t *replaceUnknown;
	wchar_t *replaceZero;
	

};


#endif //_NULLSOFT_WINAMP_DATAVIEW_INTEGER_COLUMN_HEADER