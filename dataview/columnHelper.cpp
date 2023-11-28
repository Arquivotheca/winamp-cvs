#include "main.h"
#include "./columnHelper.h"

static int 
Column_SortNameCb(const void *element1, const void *element2)
{
	int compareResult;
	ifc_viewcolumn *column1, *column2;
	ifc_viewcolumninfo *info1, *info2;
	
	column1 = *((ifc_viewcolumn**)element1);
	column2 = *((ifc_viewcolumn**)element2);

	if (NULL == column1 || FAILED(column1->GetInfo(&info1)))
		info1 = NULL;

	if (NULL == column2 || FAILED(column2->GetInfo(&info2)))
		info2 = NULL;

	compareResult = ColumnInfo_CompareByName(info1, info2);

	SafeRelease(info1);
	SafeRelease(info2);

	return compareResult;

}

static int 
Column_FindNameCb(void *context, const void *target, const void *element)
{
	ifc_viewcolumn *column;
	
	column = *((ifc_viewcolumn**)element);

	if (NULL != column)
	{
		ifc_viewcolumninfo *info;
		if (SUCCEEDED(column->GetInfo(&info)))
		{
			int compareResult;

			compareResult = ColumnInfo_CompareNames((const char*)target, ((size_t)context), 
													info->GetName(), -1);
			
			info->Release();
			return compareResult;
		}
	}

	return -1;
}

const char*
Column_GetName(ifc_viewcolumn *column)
{
	const char *name;
	ifc_viewcolumninfo *info;

	if (NULL == column)
		return NULL;

	if (FAILED(column->GetInfo(&info)))
		return NULL;

	name = info->GetName();

	info->Release();

	return name;
}

int 
Column_CompareByName(ifc_viewcolumn *column1, ifc_viewcolumn *column2)
{
	ifc_viewcolumninfo *info1, *info2;
	int compareResults;
	
	if (NULL == column1 || FAILED(column1->GetInfo(&info1)))
		info1 = NULL;

	if (NULL == column2 || FAILED(column2->GetInfo(&info2)))
		info2 = NULL;

	compareResults = ColumnInfo_CompareByName(info1, info2);

	SafeRelease(info1);
	SafeRelease(info2);

	return compareResults;
}

void
Column_SortByName(ifc_viewcolumn **columns, size_t count)
{
	qsort(columns, count, sizeof(ifc_viewcolumn**), Column_SortNameCb);
}

ifc_viewcolumn ** 
Column_SearchByName(const char *name, ifc_viewcolumn **columns, size_t count)
{
	size_t length;

	if (FALSE != IS_STRING_EMPTY(name))
		return NULL;

	length = lstrlenA(name);
	
	return (ifc_viewcolumn**)bsearch_s(name, columns, count, sizeof(ifc_viewcolumn**),
										Column_FindNameCb, (void*)length);
}

ifc_viewcolumn **
Column_SearchByNameUnsorted(const char *name, ifc_viewcolumn **columns, size_t count)
{
	size_t index;
	size_t length;
	ifc_viewcolumn *column;
	ifc_viewcolumninfo *info;
	int compareResult;

	if (0 == count)
		return NULL;
	
	length = lstrlenA(name);
	
	for(index = 0; index < count; index++)
	{
		column = columns[index];
		
		if (NULL == column ||
			FAILED(column->GetInfo(&info)) || 
			NULL == info)
		{
			continue;
		}

		compareResult = ColumnInfo_CompareNames(name, length, info->GetName(), -1);
		info->Release();

		if (0 == compareResult)
			return &columns[index];
	}

	return NULL;
}
