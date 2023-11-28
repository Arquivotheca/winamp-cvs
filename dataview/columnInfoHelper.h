#ifndef _NULLSOFT_WINAMP_DATAVIEW_COLUMN_INFO_HELPER_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_COLUMN_INFO_HELPER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "./ifc_viewcolumninfo.h"

int 
ColumnInfo_CompareNames(const char *name1, 
						int length1, 
						const char *name2, 
						int length2);

int 
ColumnInfo_CompareByName(ifc_viewcolumninfo *columnInfo1, 
						 ifc_viewcolumninfo *columnInfo2);

void
ColumnInfo_SortByName(ifc_viewcolumninfo **columns, 
					  size_t count);

ifc_viewcolumninfo ** 
ColumnInfo_SearchByName(const char *name, 
						ifc_viewcolumninfo **columns, 
						size_t count);

ifc_viewcolumninfo **
ColumnInfo_SearchByNameUnsorted(const char *name, 
								ifc_viewcolumninfo **columns, 
								size_t count);


#endif //_NULLSOFT_WINAMP_DATAVIEW_COLUMN_INFO_HELPER_HEADER
