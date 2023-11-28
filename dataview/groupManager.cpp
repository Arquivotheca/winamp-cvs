#include "main.h"
#include "./groupManager.h"
#include "./groupEnum.h"

GroupManager::GroupManager()
	: ref(1)
{
}

GroupManager::~GroupManager()
{
	size_t index;

	index = list.size();
	while(index--)
	{
		list[index]->Release();
	}
}

HRESULT GroupManager::CreateInstance(GroupManager **instance)
{
	if (NULL == instance)
		return E_POINTER;

	*instance = new (std::nothrow) GroupManager();
	if (NULL == *instance)
		return E_OUTOFMEMORY;

	return S_OK;
}

size_t GroupManager::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t GroupManager::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int GroupManager::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) 
		return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_GroupManager))
		*object = static_cast<ifc_groupmanager*>(this);
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

size_t GroupManager::Register(ifc_groupprovider **providers, size_t count)
{
	size_t index, registered, added;
	ifc_groupprovider *object, **begin;
	const char *providerName;
	
	if (NULL == providers || 0 == count)
		return 0;

	added = 0;
	registered = list.size();
	begin = list.begin();

	for (index = 0; index < count; index++)
	{
		object = providers[index];
		if (NULL != object)
		{
			providerName = object->GetName();
			if (FALSE == IS_STRING_EMPTY(providerName) &&
				NULL == GroupProvider_SearchByName(providerName, begin, registered) &&
				NULL == GroupProvider_SearchByNameUnsorted(providerName, begin + registered, added))
			{
				list.push_back(object);
				object->AddRef();

				added++;
			}
		}
	}
	
	if (0 != added)
	{
		GroupProvider_SortByName(list.begin(), list.size());
	}

	return added;
}

HRESULT GroupManager::Unregister(const char *providerName)
{
	ifc_groupprovider **cursor, **begin;

	begin = list.begin();
	cursor = GroupProvider_SearchByName(providerName, begin, list.size());
	if (NULL != cursor)
	{
		size_t index;
		ifc_groupprovider *provider;

		provider = *cursor;

		index = (size_t)(cursor - begin);
		list.eraseindex(index);

		provider->Release();

		return S_OK;
	}

	return S_FALSE;
}

HRESULT GroupManager::Enumerate(ifc_groupenum **enumerator)
{
	return GroupEnum::CreateInstance(list.begin(), list.size(), (GroupEnum**)enumerator);
}

HRESULT GroupManager::Find(const char *providerName, ifc_groupprovider **provider)
{
	ifc_groupprovider **cursor, **begin;

	begin = list.begin();
	cursor = GroupProvider_SearchByName(providerName, begin, list.size());
	if (NULL != cursor)
	{
		if (NULL != provider)
		{
			*provider = *cursor;
			(*provider)->AddRef();
		}
		return S_OK;
	}

	if (NULL != provider)
		*provider = NULL;

	return S_FALSE;
}

#define CBCLASS GroupManager
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