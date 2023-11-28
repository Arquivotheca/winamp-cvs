#ifndef _NULLSOFT_WINAMP_DATAVIEW_GROUP_HELPER_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_GROUP_HELPER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "./ifc_groupprovider.h"

int 
GroupProvider_CompareNames(const char *name1, 
						   int length1, 
						   const char *name2, 
						   int length2);

int 
GroupProvider_CompareByName(ifc_groupprovider *provider1, 
							ifc_groupprovider *provider2);

void
GroupProvider_SortByName(ifc_groupprovider **columns, 
						 size_t count);

ifc_groupprovider ** 
GroupProvider_SearchByName(const char *name, 
						   ifc_groupprovider **providers, 
						   size_t count);

ifc_groupprovider **
GroupProvider_SearchByNameUnsorted(const char *name, 
								   ifc_groupprovider **providers, 
								   size_t count);


#endif //_NULLSOFT_WINAMP_DATAVIEW_GROUP_HELPER_HEADER