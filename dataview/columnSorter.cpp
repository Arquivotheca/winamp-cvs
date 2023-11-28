#include "main.h"
#include "./columnSorter.h"

#include <strsafe.h>


#define FORCE_ASCENDING_ORDER	((ifc_viewcolumn*)(intptr_t)1)
#define FORCE_DESCENDING_ORDER	((ifc_viewcolumn*)(intptr_t)2)
#define REVERSE_ORDER			((ifc_viewcolumn*)(intptr_t)3)

#define IS_ORDER_MODIFIER(_c)	(IS_INTRESOURCE(_c))



typedef struct ParserParameter
{
	ColumnSorter *sorter;
	ifc_viewcontents *contents;
	size_t	valueIndex;
	ifc_viewcolumn *column;
	SortOrder order;
	SortOrder prevOrder;
	char buffer[256];
} ParserParameter;

typedef struct ReorderParameter
{
	ColumnSorter *sorter;
	ifc_dataobjectlist *objectList;
	const void *lastRecord;
} ReorderParameter;


ColumnSorter::ColumnSorter(ifc_viewcolumn *column, SortOrder order, ifc_viewcontents *contents)
{
	if (NULL != column)
	{
		ifc_viewcolumninfo *info;

		AddColumn(column, order);

		if(SUCCEEDED(column->GetInfo(&info)))
		{
			ParseSortRule(contents, info->GetSortRule());
			info->Release();
		}

		if (2 == sortRule.size())
		{
			ifc_viewcolumn *primaryColumn;
			if (NULL != contents && 
				S_OK == contents->GetPrimaryColumn(&primaryColumn))
			{
				if (column != primaryColumn && 
					0 != Column_CompareByName(column, primaryColumn))
				{
					AddColumn(primaryColumn, SortOrder_Ascending);
				}
				primaryColumn->Release();
			}
		}

	}

	lcid = Plugin_GetUserLocaleId();
}

ColumnSorter::~ColumnSorter()
{
	size_t index;
	ifc_viewcolumn *column;

	index = sortRule.size();
	while(index--)
	{
		column = sortRule[index];
		if (FALSE == IS_ORDER_MODIFIER(column))
			column->Release();
	}
}

HRESULT ColumnSorter::AddColumn(ifc_viewcolumn *column, SortOrder order)
{
	if (NULL == column)
		return E_POINTER;

	
	if (SortOrder_Ascending == order)
		sortRule.push_back(FORCE_ASCENDING_ORDER);
	else if (SortOrder_Descending == order)
		sortRule.push_back(FORCE_DESCENDING_ORDER);
			
	column->AddRef();
	sortRule.push_back(column);

	return S_OK;
}

int ColumnSorter::Compare(ifc_dataobject *object1, ifc_dataobject *object2)
{
	size_t index, count;
	int result, direction;
	ifc_viewcolumn *column;
	
	direction = 1;
	count = sortRule.size();
	for(index = 0; index < count; index++)
	{
		column = sortRule[index];
		
		if (FORCE_ASCENDING_ORDER == column)
			direction = 1;
		else if (FORCE_DESCENDING_ORDER == column)
			direction = -1;
		else if (REVERSE_ORDER == column)
			direction = (1 == direction) ? -1 : 1;
		else
		{
			result = sortRule[index]->Compare(lcid, object1, object2);
			if (COBJ_EQUAL != result && COBJ_ERROR != result)
				return ((result - 2) * direction);
		}
	}

	return 0;
}


static int __cdecl
ClumnSorter_ReorderCb(void *context, const void *element1, const void *element2)
{
	const ReorderParameter *param;
	ifc_dataobject *object1, *object2;
	
	param = (const ReorderParameter*)context;
	
	object1 = param->objectList->GetItem(*(const size_t*)element1);
	object2 = param->objectList->GetItem(*(const size_t*)element2);

	//wchar_t buffer1[256], buffer2[256];
	//DataValue value;
	//
	//DATAVALUE_INIT_WSTR_BUFFER(&value, buffer1, ARRAYSIZE(buffer1));
	//object1->GetValue(1033, 2, &value);
	//DATAVALUE_CLEAR(&value);

	//DATAVALUE_INIT_WSTR_BUFFER(&value, buffer2, ARRAYSIZE(buffer2));
	//object2->GetValue(1033, 2, &value);
	//DATAVALUE_CLEAR(&value);
	
	
	int result = param->sorter->Compare(object1, object2);
	return result;
	
}

HRESULT ColumnSorter::Reorder(ifc_dataobjectlist *objectList, size_t *order, size_t count)
{
	ReorderParameter param;

	if (NULL == order)
		return E_INVALIDARG;

	if (NULL == objectList)
		return E_INVALIDARG;
	
	param.sorter = this;
	param.objectList = objectList;

	qsort_s(order, count, sizeof(size_t), ClumnSorter_ReorderCb, &param);

	return S_OK;
}

static int ColumnSorter_SearchAndInsertCb(void *context, const void *key, const void *element)
{
	
	const ReorderParameter *param;
	size_t index;
	ifc_dataobject *object, *record;
	int result;

	param = (const ReorderParameter*)context;

	object = (ifc_dataobject*)key;
	index = *(const size_t*)element;
	record = param->objectList->GetItem(index);

	//wchar_t buffer1[256], buffer2[256];
	//DataValue value;
	//
	//DATAVALUE_INIT_WSTR_BUFFER(&value, buffer1, ARRAYSIZE(buffer1));
	//object->GetValue(1033, 2, &value);
	//DATAVALUE_CLEAR(&value);

	//DATAVALUE_INIT_WSTR_BUFFER(&value, buffer2, ARRAYSIZE(buffer2));
	//record->GetValue(1033, 2, &value);
	//DATAVALUE_CLEAR(&value);
	
	result = param->sorter->Compare(object, record);

	if (result < 0)
		return -1;

	if (result >= 0)
	{
		if (element == param->lastRecord)
			return 0;

		record = param->objectList->GetItem(*((const size_t*)element + 1));
	/*	DATAVALUE_INIT_WSTR_BUFFER(&value, buffer2, ARRAYSIZE(buffer2));
		record->GetValue(1033, 2, &value);
		DATAVALUE_CLEAR(&value);*/

		result = param->sorter->Compare(object, record);
		if (result < 0)
			return 0;
	}
	
	return 1;
}

HRESULT ColumnSorter::Reorder2(ifc_dataobjectlist *objectList, size_t *order, size_t count, size_t sortedCount)
{
	ReorderParameter param;

	if (NULL == order)
		return E_INVALIDARG;

	if (NULL == objectList)
		return E_INVALIDARG;
	
	if (count < 2)
		return S_OK;

	if (sortedCount >= count)
		return S_OK;

	if (sortedCount < count /2)
		sortedCount = 0;
	
	param.sorter = this;
	param.objectList = objectList;
	param.lastRecord = &order[sortedCount - 1];

	qsort_s(order + sortedCount, count - sortedCount, sizeof(size_t), ClumnSorter_ReorderCb, &param);

	if (sortedCount > 0)
	{
		ifc_dataobject *object;
		size_t *insertPos, insertIndex;
		size_t *insertCopy, insertCount, searchCount;
		BOOL freeInsertCopy;
		insertCount = count - sortedCount;
	
		if (insertCount <= 1024)
		{
			__try
			{
				insertCopy = (size_t*)_alloca(insertCount * sizeof(size_t));
			}
			__except(STATUS_STACK_OVERFLOW == GetExceptionCode())
			{
				_resetstkoflw();
				insertCopy = NULL;
			}
		}
		else insertCopy = NULL;

		if (NULL == insertCopy)
		{
			insertCopy = (size_t*)malloc(insertCount * sizeof(size_t));
			if (NULL == insertCopy)
				return E_OUTOFMEMORY;
			
			freeInsertCopy = TRUE;
		}
		else
			freeInsertCopy = FALSE;

		
		memcpy(insertCopy, &order[sortedCount], insertCount * sizeof(size_t));
		searchCount = sortedCount;

		while(insertCount--)
		{
			object = objectList->GetItem(insertCopy[insertCount]);
			insertPos = (size_t*)bsearch_s(object, order, searchCount, sizeof(size_t), 
											ColumnSorter_SearchAndInsertCb, &param);

			if (NULL != insertPos)
				insertIndex = insertPos - order + 1;
			else
				insertIndex = 0;

			if (insertIndex < searchCount)
				memmove(&order[insertIndex + insertCount + 1], &order[insertIndex], sizeof(size_t) * (searchCount - insertIndex));
			
			order[insertIndex + insertCount] = insertCopy[insertCount];
			searchCount = insertIndex;
		}

		if (FALSE != freeInsertCopy)
			free(insertCopy);
	}

	return S_OK;
}

static StringParserResponse __stdcall 
ColumnSorter_ValueParserCb(const char *keyword, unsigned int length, const void *context)
{
	ParserParameter *self;

	self = (ParserParameter*)context;
	if (NULL == self)
		return StringParserResponse_Abort;
	
	switch (self->valueIndex)
	{
		case 0:
			if (0 == ColumnInfo_CompareNames(keyword, length, "Primary", -1))
			{
				if (S_OK != self->contents->GetPrimaryColumn(&self->column))
					return StringParserResponse_Abort;
			}
			else if (FAILED(StringCchCopyNA(self->buffer, ARRAYSIZE(self->buffer), keyword, length)) ||
					 S_OK != self->contents->FindColumn(self->buffer, &self->column))
			{
				return StringParserResponse_Abort;
			}
			break;
		case 1:
			if (1 == length)
			{
				switch(keyword[0])
				{
					case L'A':
					case L'a':
						self->order = SortOrder_Ascending;
						break;
					case L'D':
					case L'd':
						self->order = SortOrder_Descending;
						break;
				}
			}
			else if (CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, NORM_IGNORECASE, "Asc", -1, keyword, length) ||
					 CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, NORM_IGNORECASE, "Ascending", -1, keyword, length))
			{
				self->order = SortOrder_Ascending;
			}
			else if (CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, NORM_IGNORECASE, "Desc", -1, keyword, length) ||
					 CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, NORM_IGNORECASE, "Descending", -1, keyword, length))
			{
				self->order = SortOrder_Descending;
			}
			break;
	}

	self->valueIndex++;
	return StringParserResponse_Continue;
}

static StringParserResponse __stdcall 
ColumnSorter_BlockParserCb(const char *keyword, unsigned int length, const void *context)
{
	ParserParameter *self;
	
	self = (ParserParameter*)context;
	if (NULL == self)
		return StringParserResponse_Continue;

	self->valueIndex = 0;
	self->column = NULL;
	self->order = SortOrder_Undefined;

	AnsiString_ParseKeywords(keyword, length, ',', TRUE, ColumnSorter_ValueParserCb, self);

	if (NULL != self->column)
	{
		self->sorter->AddColumn(self->column, self->order);
		self->column->Release();
	}
	
	return StringParserResponse_Continue;
}


HRESULT ColumnSorter::ParseSortRule(ifc_viewcontents *contents, const char *rule)
{
	ParserParameter param;

	if (NULL == contents)
		return E_INVALIDARG;

	if (IS_STRING_EMPTY(rule))
		return S_OK;

	param.sorter = this;
	param.contents = contents;
	
	AnsiString_ParseKeywords(rule, -1, '|', TRUE, ColumnSorter_BlockParserCb, &param);
		
	return S_OK;
}