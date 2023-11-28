#include "main.h"
#include "./actionContextEnum.h"


ActionContextEnum::ActionContextEnum()
	: ref(1), cursor(0)
{
}

ActionContextEnum::~ActionContextEnum()
{
	Clear();
}

HRESULT ActionContextEnum::CreateInstance(ActionContextEnum **instance)
{
	if (NULL == instance)
		return E_POINTER;
	
	*instance = new (std::nothrow) ActionContextEnum();
	if (NULL == *instance)
		return E_POINTER;

	return S_OK;
}


size_t ActionContextEnum::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t ActionContextEnum::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int ActionContextEnum::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) 
		return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_ViewActionContextEnum))
		*object = static_cast<ifc_viewactioncontextenum*>(this);
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

HRESULT ActionContextEnum::Next(Dispatchable **buffer, size_t bufferMax, size_t *fetched)
{
	size_t available, copied, index, size;
	Dispatchable **source;

	if (NULL == buffer)
		return E_POINTER;
	
	if (0 == bufferMax) 
		return E_INVALIDARG;

	size = list.size();

	if (cursor >= size)
	{
		if (NULL != fetched) 
			*fetched = 0;

		return S_FALSE;
	}

	available = size - cursor;
	copied = ((available > bufferMax) ? bufferMax : available);
	
	source = list.begin() + cursor;
	CopyMemory(buffer, source, copied * sizeof(ifc_viewcolumninfo*));
    
	for(index = 0; index < copied; index++)
		buffer[index]->AddRef();
	
	cursor += copied;

	if (NULL != fetched) 
		*fetched = copied;

	return (bufferMax == copied) ? S_OK : S_FALSE;
}

HRESULT ActionContextEnum::Reset(void)
{
	cursor = 0;
	return S_OK;
}

HRESULT ActionContextEnum::Skip(size_t count)
{
	size_t size;

	size = list.size();

	cursor += count;
	if (cursor > size)
		cursor = size;
	
	return (cursor < size) ? S_OK : S_FALSE;
}

HRESULT ActionContextEnum::GetCount(size_t *count)
{
	if (NULL == count)
		return E_POINTER;
	
	*count = list.size();

	return S_OK;
}

HRESULT ActionContextEnum::Find(const GUID *contextId, Dispatchable **instance)
{
	size_t index;
	GUID searchId;
	Dispatchable *context;

	if (NULL == instance)
		return E_POINTER;

	*instance = NULL;

	if (NULL == contextId)
		return E_INVALIDARG;

	searchId = *contextId;

	index = list.size();
	while(index--)
	{
		context = list[index];
		if (NULL != context)
		{
			if (SUCCEEDED(context->QueryInterface(searchId, (void**)instance)) && 
				NULL != *instance)
			{
				return S_OK;
			}
		}
	}

	return S_FALSE;
}


HRESULT ActionContextEnum::Add(Dispatchable *context)
{
	size_t index;
	
	if (NULL == context)
		return E_INVALIDARG;

	index = list.size();
	while(index--)
	{
		if (context == list[index])
			return S_FALSE;
	}

	context->AddRef();
	list.push_back(context);

	return S_OK;
}

HRESULT ActionContextEnum::Remove(Dispatchable *context)
{
	size_t index;

	if (NULL == context)
		return E_INVALIDARG;

	index = list.size();
	while(index--)
	{
		if (context == list[index])
		{
			list.eraseindex(index);
			SafeRelease(context);
			return S_OK;
		}
	}
	return S_FALSE;
	
}

HRESULT ActionContextEnum::Clear()
{
	size_t index;
	Dispatchable *context;

	index = list.size();
	while(index--)
	{
		context = list[index];
		SafeRelease(context);
	}

	list.clear();
	return S_OK;
}

#define CBCLASS ActionContextEnum
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
CB(API_NEXT, Next)
CB(API_RESET, Reset)
CB(API_SKIP, Skip)
CB(API_GETCOUNT, GetCount)
CB(API_FIND, Find)
END_DISPATCH;
#undef CBCLASS