#include "main.h"
#include "./filteredObjectEnum.h"

FilteredObjectEnum::FilteredObjectEnum(ifc_dataobject **_objects, const size_t *_map, size_t _count) 
	: ref(1), cursor(0), objects(_objects), map(_map), count(_count)
{
}

FilteredObjectEnum::~FilteredObjectEnum()
{ 
}

HRESULT FilteredObjectEnum::CreateInstance(ifc_dataobject **objects, const size_t *map, size_t count,
										   FilteredObjectEnum **instance)
{
	if (NULL == instance) 
		return E_POINTER;

	*instance = NULL;

	if (NULL == objects || NULL == map)
		return E_INVALIDARG;

	*instance = new (std::nothrow) FilteredObjectEnum(objects, map, count);
	if (NULL == *instance)
		return E_OUTOFMEMORY;

	return S_OK;
}


size_t FilteredObjectEnum::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t FilteredObjectEnum::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int FilteredObjectEnum::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_DataObjectEnum))
		*object = static_cast<ifc_dataobjectenum*>(this);
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

HRESULT FilteredObjectEnum::Next(ifc_dataobject **buffer, size_t bufferMax, size_t *fetched)
{
	size_t limit, index;
		
	if (NULL == buffer)
		return E_POINTER;
	
	if (0 == bufferMax) 
		return E_INVALIDARG;

	if (cursor >= count)
	{
		if (NULL != fetched) 
			*fetched = 0;
		return S_FALSE;
	}

	limit = cursor + bufferMax;
	if (limit > count)
		limit = count;

	for(index = 0; cursor < limit; cursor++, index++)
	{
		buffer[index] = objects[map[cursor]];
		buffer[index]->AddRef();
	}
    	
	if (NULL != fetched) 
		*fetched = index;

	return (bufferMax == index) ? S_OK : S_FALSE;
}

HRESULT FilteredObjectEnum::Reset(void)
{
	cursor = 0;
	return S_OK;
}

HRESULT FilteredObjectEnum::Skip(size_t _count)
{
	cursor += _count;
	if (cursor > count)
		cursor = count;
	
	return (cursor < count) ? S_OK : S_FALSE;
}

HRESULT FilteredObjectEnum::GetCount(size_t *_count)
{
	if (NULL == _count)
		return E_POINTER;

	(*_count) = count;
	return S_OK;
}

#define CBCLASS FilteredObjectEnum
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