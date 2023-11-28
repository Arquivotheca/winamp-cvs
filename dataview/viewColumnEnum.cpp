#include "main.h"
#include "./viewColumnEnum.h"


ViewColumnEnum::ViewColumnEnum(ViewColumn **_list, size_t _size)
	: ref(1), list(_list), size(_size), cursor(0)
{
}

ViewColumnEnum::~ViewColumnEnum()
{
	if (NULL != list)
	{
		while(size--)
		{
			list[size]->Release();
		}
	}
}

HRESULT ViewColumnEnum::CreateInstance(ViewColumn **list, size_t count, ViewColumnEnum **instance)
{
	size_t index, size;
	void *storage;
	ViewColumn *column;
	ViewColumnEnum *self;
	
	if (NULL == instance) 
		return E_POINTER;

	*instance = NULL;

	size = sizeof(ViewColumnEnum) + (sizeof(ViewColumn**) * count);
	storage = malloc(size);
	if (NULL == storage)
		return E_OUTOFMEMORY;
	
	self = new(storage) ViewColumnEnum((ViewColumn**)(((BYTE*)storage) + sizeof(ViewColumnEnum)), 0);
	if (NULL == self)
	{
		free(storage);
		return E_FAIL;
	}

	for (index = 0; index < count; index++)
	{
		column = list[index];
		if (NULL != column)
		{
			self->list[self->size] = column;
			column->AddRef();
			self->size++;
		}
	}

	*instance = self;
	return S_OK;
}



size_t ViewColumnEnum::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t ViewColumnEnum::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

HRESULT ViewColumnEnum::Next(ViewColumn **buffer, size_t bufferMax, size_t *fetched)
{
	size_t available, copied, index;
	ViewColumn **source;

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
	CopyMemory(buffer, source, copied * sizeof(ViewColumn*));
    
	for(index = 0; index < copied; index++)
		buffer[index]->AddRef();
	
	cursor += copied;

	if (NULL != fetched) 
		*fetched = copied;

	return (bufferMax == copied) ? S_OK : S_FALSE;
}

HRESULT ViewColumnEnum::Reset(void)
{
	cursor = 0;
	return S_OK;
}

HRESULT ViewColumnEnum::Skip(size_t count)
{
	cursor += count;
	if (cursor > size)
		cursor = size;
	
	return (cursor < size) ? S_OK : S_FALSE;
}


HRESULT ViewColumnEnum::GetCount(size_t *count)
{
	if (NULL == count)
		return E_POINTER;
	
	*count = size;

	return S_OK;
}