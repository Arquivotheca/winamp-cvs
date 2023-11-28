#include "main.h"
#include "./selectionEnum.h"


SelectionEnum::SelectionEnum(IndexRange *_list, size_t _count)
	: ref(1), list(_list), count(_count), cursor(0)
{
}

SelectionEnum::~SelectionEnum()
{
}

HRESULT SelectionEnum::CreateInstance(IndexRange *list, size_t count, SelectionEnum **instance)
{
	if (NULL == instance)
		return E_POINTER;

	if (NULL == list && 0 != count)
		return E_INVALIDARG;

	*instance = new (std::nothrow) SelectionEnum(list, count);
	if (NULL == *instance)
		return E_POINTER;

	return S_OK;
}



size_t SelectionEnum::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t SelectionEnum::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int SelectionEnum::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) 
		return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_ViewSelectionEnum))
		*object = static_cast<ifc_viewselectionenum*>(this);
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

HRESULT SelectionEnum::Next(IndexRange *buffer, size_t bufferMax, size_t *fetched)
{
	size_t available, copied;

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

	available = count - cursor;
	copied = ((available > bufferMax) ? bufferMax : available);
		
	CopyMemory(buffer, list + cursor, copied * sizeof(IndexRange));
    
	cursor += copied;

	if (NULL != fetched) 
		*fetched = copied;

	return (bufferMax == copied) ? S_OK : S_FALSE;
}

HRESULT SelectionEnum::Reset(void)
{
	cursor = 0;
	return S_OK;
}

HRESULT SelectionEnum::Skip(size_t _count)
{
	cursor += _count;
	if (cursor > count)
		cursor = count;
	
	return (cursor < count) ? S_OK : S_FALSE;
}


HRESULT SelectionEnum::GetCount(size_t *_count)
{
	if (NULL == _count)
		return E_POINTER;
	
	*_count = count;

	return S_OK;
}

#define CBCLASS SelectionEnum
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