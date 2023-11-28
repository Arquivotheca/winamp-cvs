#include "main.h"
#include "./groupEnum.h"

GroupEnum::GroupEnum(ifc_groupprovider **_list, size_t _size)
	: ref(1), list(_list), size(_size), cursor(0)
{
}

GroupEnum::~GroupEnum()
{
	if (NULL != list)
	{
		while(size--)
		{
			list[size]->Release();
		}
	}
}

HRESULT GroupEnum::CreateInstance(ifc_groupprovider **providers, size_t count, GroupEnum **instance)
{
	size_t index, size;
	void *storage;
	ifc_groupprovider *provider;
	GroupEnum *self;
	
	if (NULL == instance) 
		return E_POINTER;

	*instance = NULL;

	size = sizeof(GroupEnum) + (sizeof(ifc_groupprovider**) * count);
	storage = malloc(size);
	if (NULL == storage)
		return E_OUTOFMEMORY;
	
	self = new(storage) GroupEnum((ifc_groupprovider**)(((BYTE*)storage) + sizeof(GroupEnum)), 0);
	if (NULL == self)
	{
		free(storage);
		return E_FAIL;
	}

	for (index = 0; index < count; index++)
	{
		provider = providers[index];
		if (NULL != provider)
		{
			self->list[self->size] = provider;
			provider->AddRef();
			self->size++;
		}
	}

	*instance = self;
	return S_OK;
}



size_t GroupEnum::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t GroupEnum::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int GroupEnum::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) 
		return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_GroupEnum))
		*object = static_cast<ifc_groupenum*>(this);
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

HRESULT GroupEnum::Next(ifc_groupprovider **buffer, size_t bufferMax, size_t *fetched)
{
	size_t available, copied, index;
	ifc_groupprovider **source;

	if (NULL == buffer)
		return E_POINTER;
	
	if (0 == bufferMax) 
		return E_INVALIDARG;

	if (cursor >= size)
	{
		if (NULL != fetched) 
			*fetched = 0;

		return S_FALSE;
	}

	available = size - cursor;
	copied = ((available > bufferMax) ? bufferMax : available);
	
	source = list + cursor;
	CopyMemory(buffer, source, copied * sizeof(ifc_viewcolumn*));
    
	for(index = 0; index < copied; index++)
		buffer[index]->AddRef();
	
	cursor += copied;

	if (NULL != fetched) 
		*fetched = copied;

	return (bufferMax == copied) ? S_OK : S_FALSE;
}

HRESULT GroupEnum::Reset(void)
{
	cursor = 0;
	return S_OK;
}

HRESULT GroupEnum::Skip(size_t count)
{
	cursor += count;
	if (cursor > size)
		cursor = size;
	
	return (cursor < size) ? S_OK : S_FALSE;
}


HRESULT GroupEnum::GetCount(size_t *count)
{
	if (NULL == count)
		return E_POINTER;
	
	*count = size;

	return S_OK;
}

#define CBCLASS GroupEnum
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
CB(API_NEXT, Next)
CB(API_RESET, Reset)
CB(API_SKIP, Skip)
CB(API_GETCOUNT, GetCount)
END_DISPATCH;
#undef CBCLASS