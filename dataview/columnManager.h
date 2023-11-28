#ifndef _NULLSOFT_WINAMP_DATAVIEW_COLUMN_MANAGER_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_COLUMN_MANAGER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "./ifc_viewcolumnmanager.h"

#include "../nu/ptrlist.h"

class ColumnManager : public ifc_viewcolumnmanager
{

protected:
	ColumnManager();
	~ColumnManager();

public:
	static HRESULT CreateInstance(ColumnManager **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_viewcolumnmanager */
	size_t Register(ifc_viewcolumninfo **columnInfo, size_t count);
	HRESULT Unregister(const char *columnName);
	HRESULT Enumerate(ifc_viewcolumninfoenum **enumerator);
	HRESULT Find(const char *columnName, ifc_viewcolumninfo **columnInfo);

protected:
	typedef nu::PtrList<ifc_viewcolumninfo> ColumnList;

protected:
	size_t ref;
	ColumnList columns;

protected:
	RECVS_DISPATCH;
};


#endif //_NULLSOFT_WINAMP_DATAVIEW_COLUMN_MANAGER_HEADER