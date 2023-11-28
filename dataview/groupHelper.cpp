#include "main.h"
#include "./groupHelper.h"


static int 
GroupProvider_SortNameCb(const void *element1, const void *element2)
{
	const char *name1, *name2;
	
	name1 = (NULL != element1) ? (*((ifc_groupprovider**)element1))->GetName() : NULL;
	name2 = (NULL != element2) ? (*((ifc_groupprovider**)element2))->GetName() : NULL;
	
	return GroupProvider_CompareNames(name1, -1, name2, -1);
}

static int 
GroupProvider_FindNameCb(void *context, const void *target, const void *element)
{
	const char *name;

	name = (NULL != element) ? (*((ifc_groupprovider**)element))->GetName() : NULL;

	return GroupProvider_CompareNames((const char*)target, ((size_t)context), name, -1);
}

int 
GroupProvider_CompareNames(const char *name1, int length1, const char *name2, int length2)
{
	if (NULL == name1 || NULL == name2)
		return ((NULL != name1) ? 1 : ((NULL == name2) ? 0 : -1));

	return CompareStringA(CSTR_INVARIANT, NORM_IGNORECASE, name1, length1, name2, length2) - 2;
}

int 
GroupProvider_CompareByName(ifc_groupprovider *provider1, ifc_groupprovider *provider2)
{
	const char *name1, *name2;

	name1 = (NULL != provider1) ? provider1->GetName() : NULL;
	name2 = (NULL != provider2) ? provider2->GetName() : NULL;

	return GroupProvider_CompareNames(name1, -1, name2, -1);
}

void
GroupProvider_SortByName(ifc_groupprovider **providers, size_t count)
{
	if (NULL == providers)
		return;

	qsort(providers, count, sizeof(ifc_groupprovider**), GroupProvider_SortNameCb);
}

ifc_groupprovider ** 
GroupProvider_SearchByName(const char *name, ifc_groupprovider **providers, size_t count)
{
	size_t length;

	if (FALSE != IS_STRING_EMPTY(name))
		return NULL;

	length = lstrlenA(name);

	return (ifc_groupprovider**)bsearch_s(name, providers, count, sizeof(ifc_groupprovider**),
										GroupProvider_FindNameCb, (void*)length);
}

ifc_groupprovider **
GroupProvider_SearchByNameUnsorted(const char *name, ifc_groupprovider **providers, size_t count)
{
	size_t index;
	size_t length;

	if (FALSE != IS_STRING_EMPTY(name) || 
		0 == count || 
		NULL == providers)
	{
		return NULL;
	}
	
	length = lstrlenA(name);
	
	for(index = 0; index < count; index++)
	{
		if (0 == GroupProvider_CompareNames(name, length, providers[index]->GetName(), -1))
			return &providers[index];
	}

	return NULL;
}
