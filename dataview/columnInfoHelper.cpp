#include "main.h"
#include "./columnInfoHelper.h"


static int 
ColumnInfo_SortNameCb(const void *element1, const void *element2)
{
	const char *name1, *name2;
	
	name1 = (NULL != element1) ? (*((ifc_viewcolumninfo**)element1))->GetName() : NULL;
	name2 = (NULL != element2) ? (*((ifc_viewcolumninfo**)element2))->GetName() : NULL;
	
	return ColumnInfo_CompareNames(name1, -1, name2, -1);
}

static int 
ColumnInfo_FindNameCb(void *context, const void *target, const void *element)
{
	const char *name;

	name = (NULL != element) ? (*((ifc_viewcolumninfo**)element))->GetName() : NULL;

	return ColumnInfo_CompareNames((const char*)target, ((size_t)context), name, -1);
}

int 
ColumnInfo_CompareNames(const char *name1, int length1, const char *name2, int length2)
{
	if (NULL == name1 || NULL == name2)
		return ((NULL != name1) ? 1 : ((NULL == name2) ? 0 : -1));

	return CompareStringA(CSTR_INVARIANT, NORM_IGNORECASE, name1, length1, name2, length2) - 2;
}

int 
ColumnInfo_CompareByName(ifc_viewcolumninfo *columnInfo1, ifc_viewcolumninfo *columnInfo2)
{
	const char *name1, *name2;

	name1 = (NULL != columnInfo1) ? columnInfo1->GetName() : NULL;
	name2 = (NULL != columnInfo2) ? columnInfo2->GetName() : NULL;

	return ColumnInfo_CompareNames(name1, -1, name2, -1);
}

void
ColumnInfo_SortByName(ifc_viewcolumninfo **columns, size_t count)
{
	if (NULL == columns)
		return;

	qsort(columns, count, sizeof(ifc_viewcolumninfo**), ColumnInfo_SortNameCb);
}

ifc_viewcolumninfo ** 
ColumnInfo_SearchByName(const char *name, ifc_viewcolumninfo **columns, size_t count)
{
	size_t length;

	if (FALSE != IS_STRING_EMPTY(name))
		return NULL;

	length = lstrlenA(name);

	return (ifc_viewcolumninfo**)bsearch_s(name, columns, count, sizeof(ifc_viewcolumninfo**),
										ColumnInfo_FindNameCb, (void*)length);
}

ifc_viewcolumninfo **
ColumnInfo_SearchByNameUnsorted(const char *name, ifc_viewcolumninfo **columns, size_t count)
{
	size_t index;
	size_t length;

	if (FALSE != IS_STRING_EMPTY(name) || 
		0 == count || 
		NULL == columns)
	{
		return NULL;
	}
	
	length = lstrlenA(name);
	
	for(index = 0; index < count; index++)
	{
		if (0 == ColumnInfo_CompareNames(name, length, columns[index]->GetName(), -1))
			return &columns[index];
	}

	return NULL;
}
