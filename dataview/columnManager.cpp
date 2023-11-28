#include "main.h"
#include "./columnManager.h"
#include "./columnInfoEnum.h"


ColumnManager::ColumnManager()
	: ref(1)
{
}


ColumnManager::~ColumnManager()
{
	size_t index;

	index = columns.size();
	while(index--)
	{
		columns[index]->Release();
	}

}

HRESULT ColumnManager::CreateInstance(ColumnManager **instance)
{
	if (NULL == instance)
		return E_POINTER;

	*instance = new (std::nothrow) ColumnManager();
	if (NULL == *instance)
		return E_OUTOFMEMORY;

	
	return S_OK;
}

size_t ColumnManager::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t ColumnManager::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int ColumnManager::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) 
		return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_ViewColumnManager))
		*object = static_cast<ifc_viewcolumnmanager*>(this);
	else
	{
		*object = NULL;
		return E_NOINTERFACE;
	}

	if (NULL == *object)
		return E_UNEXPECTED;

	AddRef();
	return S_OK;
}

size_t ColumnManager::Register(ifc_viewcolumninfo **columnInfo, size_t count)
{
	size_t index, registered, added;
	ifc_viewcolumninfo *object, **begin;
	const char *columnName;
	
	if (NULL == columnInfo || 0 == count)
		return 0;

	added = 0;
	registered = columns.size();
	begin = columns.begin();

	for (index = 0; index < count; index++)
	{
		object = columnInfo[index];
		if (NULL != object)
		{
			columnName = object->GetName();
			if (FALSE == IS_STRING_EMPTY(columnName) &&
				NULL == ColumnInfo_SearchByName(columnName, begin, registered) &&
				NULL == ColumnInfo_SearchByNameUnsorted(columnName, begin + registered, added))
			{
				columns.push_back(object);
				object->AddRef();

				added++;
			}
		}
	}
	
	if (0 != added)
	{
		ColumnInfo_SortByName(columns.begin(), columns.size());
	}

	return added;
}

HRESULT ColumnManager::Unregister(const char *columnName)
{
	ifc_viewcolumninfo **cursor, **begin;

	begin = columns.begin();
	cursor = ColumnInfo_SearchByName(columnName, begin, columns.size());
	if (NULL != cursor)
	{
		size_t index;
		ifc_viewcolumninfo *columnInfo;

		columnInfo = *cursor;

		index = (size_t)(cursor - begin);
		columns.eraseindex(index);

		columnInfo->Release();

		return S_OK;
	}

	return S_FALSE;
}

HRESULT ColumnManager::Enumerate(ifc_viewcolumninfoenum **enumerator)
{
	return ColumnInfoEnum::CreateInstance(columns.begin(), columns.size(), (ColumnInfoEnum**)enumerator);
}

HRESULT ColumnManager::Find(const char *columnName, ifc_viewcolumninfo **columnInfo)
{
	ifc_viewcolumninfo **cursor, **begin;

	begin = columns.begin();
	cursor = ColumnInfo_SearchByName(columnName, begin, columns.size());
	if (NULL != cursor)
	{
		
		if (NULL != columnInfo)
		{
			*columnInfo = *cursor;
			(*columnInfo)->AddRef();
		}
		return S_OK;
	}

	if (NULL != columnInfo)
		*columnInfo = NULL;

	return S_FALSE;
}

#define CBCLASS ColumnManager
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
CB(API_REGISTER, Register)
CB(API_UNREGISTER, Unregister)
CB(API_ENUMERATE, Enumerate)
CB(API_FIND, Find)
END_DISPATCH;
#undef CBCLASS