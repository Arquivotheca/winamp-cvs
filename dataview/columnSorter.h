#ifndef _NULLSOFT_WINAMP_DATAVIEW_COLUMN_SORTER_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_COLUMN_SORTER_HEADER

#include <wtypes.h>
#include "./ifc_viewcolumn.h"
#include "./ifc_viewcontents.h"
#include "./ifc_dataobjectlist.h"

#include "../nu/ptrlist.h"

class ColumnSorter
{
public:
	ColumnSorter(ifc_viewcolumn *column, SortOrder order, ifc_viewcontents *contents);
	~ColumnSorter();

public:
	HRESULT AddColumn(ifc_viewcolumn *column, SortOrder order);
	HRESULT Reorder(ifc_dataobjectlist *objectList, size_t *order, size_t count);
	HRESULT Reorder2(ifc_dataobjectlist *objectList, size_t *order, size_t count, size_t sortedCount);
	int Compare(ifc_dataobject *object1, ifc_dataobject *object2);

protected:
	HRESULT ParseSortRule(ifc_viewcontents *contents, const char *rule);

protected:
	typedef nu::PtrList<ifc_viewcolumn> ColumnList;
	
protected:
	LCID lcid;
	ColumnList sortRule;
};

#endif //_NULLSOFT_WINAMP_DATAVIEW_COLUMN_SORTER_HEADER