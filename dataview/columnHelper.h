#ifndef _NULLSOFT_WINAMP_DATAVIEW_COLUMN_HELPER_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_COLUMN_HELPER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "./ifc_viewcolumn.h"

const char*
Column_GetName(ifc_viewcolumn *column);

int 
Column_CompareByName(ifc_viewcolumn *column1, 
					 ifc_viewcolumn *column2);

void
Column_SortByName(ifc_viewcolumn **columns, 
				  size_t count);

ifc_viewcolumn ** 
Column_SearchByName(const char *name, 
					ifc_viewcolumn **columns, 
					size_t count);

ifc_viewcolumn **
Column_SearchByNameUnsorted(const char *name, 
							ifc_viewcolumn **columns, 
							size_t count);


#endif //_NULLSOFT_WINAMP_DATAVIEW_COLUMN_HELPER_HEADER