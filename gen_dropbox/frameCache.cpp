#include "./frameCache.h"
#include "../nu/trace.h"

FrameCache::FrameCache()
: top(0), size(0), buffer(NULL), modified(FALSE), modifiedCallback(NULL), modifiedParam(0)
{
	InitializeCriticalSection(&lockCache);
}

FrameCache::~FrameCache()
{
	EnterCriticalSection(&lockCache);
	if (NULL != buffer)
	{
		free(buffer);
	}
	LeaveCriticalSection(&lockCache);

	DeleteCriticalSection(&lockCache);
}

void FrameCache::Reset()
{
	EnterCriticalSection(&lockCache);
	if (NULL != buffer)
		FillMemory(buffer, sizeof(BYTE) * size, ItemStateClear);
	LeaveCriticalSection(&lockCache);
}


void FrameCache::SetSize(INT newSize, BOOL bPreserve)
{
	if (newSize == size)
		return;

	EnterCriticalSection(&lockCache);

	if (newSize <= 0)
	{
		if (NULL != buffer)
		{
			free(buffer);
			buffer = NULL;
		}
		size = 0;
	}
	else
	{
		INT oldSize  = size;
		BYTE *data = (BYTE*)realloc(buffer, newSize);

		if( NULL != data)
		{
			buffer = data;
			size = newSize;
			
			if (oldSize > 0 && bPreserve)
			{
				if (oldSize < newSize)
					FillMemory(buffer + oldSize, sizeof(BYTE) * (newSize - oldSize), ItemStateClear);
			}
			else
			{
				FillMemory(buffer, sizeof(BYTE) * size, ItemStateClear);
			}
		}
	}

	LeaveCriticalSection(&lockCache);
}
INT FrameCache::GetSize()
{
	return size;
}

void FrameCache::SetTop(INT newTop, BOOL bPreserve)
{
	if (top == newTop)
		return;

	EnterCriticalSection(&lockCache);
	
	INT delta = newTop - top;
	top = newTop;
	
	if (size > 0 && NULL != buffer)
	{			
		BYTE *fillStart = buffer;
		INT fillLength = size;

		if (bPreserve)
		{
			if (delta > 0 && delta < size)
			{
				MoveMemory(buffer, buffer + delta, sizeof(BYTE) * (size - delta));
				fillStart += (size - delta);
				fillLength = delta;
			}
			else if (delta < 0 && abs(delta) < size)
			{
				delta = abs(delta);
				MoveMemory(buffer + delta, buffer, sizeof(BYTE) * (size - delta));
				fillLength = delta;
			}
		}

		if (fillLength > 0)
			FillMemory(fillStart, fillLength * sizeof(BYTE), ItemStateClear);
	}
	LeaveCriticalSection(&lockCache);
}


INT FrameCache::GetItemState(INT itemIndex)
{	
	INT itemState;
	
	EnterCriticalSection(&lockCache);

	if (0 == size || itemIndex < top || itemIndex > (top + size - 1))
	{
		itemState = ItemOutOfBounds;
	}
	else
	{
		itemState = buffer[itemIndex - top];
	}
	LeaveCriticalSection(&lockCache);
	return itemState;
}


BOOL FrameCache::SetItemState(INT itemIndex, INT itemState, BOOL markModified)
{
	BOOL result;
	
	EnterCriticalSection(&lockCache);

	if (0 == size || itemIndex < top || itemIndex > (top + size - 1))
	{
		result = FALSE;
	}
	else
	{
		INT bufferIndex = itemIndex - top;
		if (buffer[bufferIndex] != itemState)
		{
			buffer[bufferIndex] = itemState;
			if (markModified && !modified)
			{
				SetModified(TRUE);
			}
		}
		result = TRUE;
	}
	LeaveCriticalSection(&lockCache);
	return result;
}

BOOL FrameCache::SetItemStateEx(INT first, INT last, INT itemState, BOOL markModified)
{
	if (0 == size)
		return FALSE;

	EnterCriticalSection(&lockCache);

	if (first < top)
		first = top;
	if (last > (top + size - 1))
		last = top + size - 1;

	BOOL result;
	if (last >= first)
	{
		first -= top;
		last -= top;

		if (markModified)
		{
			for (;first <= last; first++)
			{
				if (buffer[first] != itemState)
				{
					buffer[first] = itemState;
					if (!modified)
					{
						SetModified(TRUE);
					}
				}
			}
		}
		else
		{
			FillMemory(buffer + first, (last - first + 1), itemState);
		}
		result = TRUE;
	}
	else
	{
		result = FALSE;
	}

	LeaveCriticalSection(&lockCache);
	return result;
}

BOOL FrameCache::GetModified()
{
	return modified;
}

void FrameCache::SetModified(BOOL bModified)
{
	EnterCriticalSection(&lockCache);
	if (modified != bModified)
	{
		modified = bModified;
		if (NULL != modifiedCallback)
		{
			modifiedCallback(modified, modifiedParam);
		}
	}
	LeaveCriticalSection(&lockCache);
}

void FrameCache::RegisterModifiedCallback(ModifiedCallback callback, ULONG_PTR param)
{
	EnterCriticalSection(&lockCache);
	modifiedCallback = callback;
	modifiedParam = param;
	LeaveCriticalSection(&lockCache);
}

BOOL FrameCache::Enumerate(INT itemState, FrameCache::EnumProc callback, ULONG_PTR param)
{
	return EnumerateEx(top, top + size - 1, itemState, callback, param);
}

BOOL FrameCache::EnumerateEx(INT first, INT last, INT itemState, FrameCache::EnumProc callback, ULONG_PTR param)
{
	if (NULL == callback || 0 == size)
		return FALSE;

	EnterCriticalSection(&lockCache);

	if (first < top)
		first = top;
	if (last > (top + size - 1))
		last = top + size - 1;

	if (last >= first)
	{
		INT block = -1;
		first -= top;
		last -= top;
		for(;first <= last; first++)
		{
			if (itemState == buffer[first])
			{
				if (-1 == block)
					block = first;
			}
			else if (-1 != block)
			{
				BOOL abort = !callback(block + top, first - 1 + top, param);
				block = -1;
				if (abort) break;
			}
		}

		if (-1 != block && first > last)
		{
			callback(block + top, last + top, param);
		}
	}

	LeaveCriticalSection(&lockCache);
	return TRUE;
}

