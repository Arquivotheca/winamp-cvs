/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author: Ben Allison benski@nullsoft.com
 ** Created:
 **/


#include "PtrList.h"
#include <memory.h>
#include <string.h>
#include <assert.h>

using namespace nu;

PtrListBase::PtrListBase() : callbacks(0), numPtrs(0), allocSize(0)
{
	callbacks = new void * [32];
	allocSize = 32;
}

PtrListBase::PtrListBase(const PtrListBase &copy) : callbacks(0)
{
	numPtrs = copy.numPtrs;
	allocSize = copy.allocSize;

	if (allocSize)
	{
		callbacks = new void * [allocSize];
		memcpy(callbacks, copy.callbacks, allocSize*sizeof(void *));
	}

}
PtrListBase::~PtrListBase()
{
	delete[] callbacks;
}

void PtrListBase::freeAll()
{
	for (size_t i = 0;i != numPtrs;i++)
	{
		free(callbacks[i]);
	}
	clear();
}

void PtrListBase::eraseindex(size_t index)
{
	assert(index < numPtrs);
	if (numPtrs == 1)
		callbacks[index] = 0;
	else
	{
    memmove(&callbacks[index], &callbacks[index+1], (numPtrs-index-1)*sizeof(void *));
		//callbacks[index] = callbacks[numPtrs - 1];
		//callbacks[numPtrs - 1] = 0;
	}

	numPtrs--;
}

void PtrListBase::erase(void *callback)
{
	for (size_t i = 0;i != numPtrs;i++)
	{
		if (callbacks[i] == callback)
		{
			eraseindex(i);
			break;
		}
	}
}

void PtrListBase::eraseRange(size_t first, size_t last)
{
	if (last >= numPtrs)
		last = numPtrs - 1;
	
	if (first > last)
		return;

	if (last < (numPtrs - 1))
	{
		 memmove(&callbacks[first], &callbacks[last+1], (numPtrs-last-1)*sizeof(void *));
	}
	numPtrs -= ((last - first) + 1);
}

void PtrListBase::eraseAll(void *callback)
{
	size_t i = 0;
	while (i < numPtrs)
	{
		if (callbacks[i] == callback)
			eraseindex(i);
		else
			i++;
	}
}

void PtrListBase::reserve(size_t reserveSize)
{
	if (reserveSize < 32)
		reserveSize = 32;

	if (reserveSize > allocSize)
	{
		void **newTable = new void * [reserveSize];
		memcpy(newTable, callbacks, numPtrs*sizeof(void *));
		allocSize = reserveSize;
		delete[] callbacks;
		callbacks = newTable;
	}
}

void PtrListBase::push_back(void *callback)
{
	if (numPtrs == allocSize)
    reserve(allocSize*2);
	
	callbacks[numPtrs++] = callback;
}

void PtrListBase::push_front(void *callback)
{
	if (numPtrs == allocSize)
	{
		void **newTable = new void * [allocSize*2];
		memcpy(&newTable[1], callbacks, numPtrs*sizeof(void *));
		allocSize *= 2;
		delete[] callbacks;
		callbacks = newTable;
	}
	else
		memmove(&callbacks[1], callbacks, numPtrs*sizeof(void *));

	numPtrs++;
	callbacks[0]=callback;
}

bool PtrListBase::contains(void *callback)
{
	for (size_t i = 0; i != numPtrs;i++)
	{
		if (callbacks[i] == callback)
			return true;
	}
	return false;
}

void PtrListBase::pop_back()
{
	numPtrs--;
}

void PtrListBase::pop_front()
{
	eraseindex(0);
}

void PtrListBase::insertBefore(size_t index, void *callback)
{
if (numPtrs == allocSize)
    reserve(allocSize*2);

  memmove(&callbacks[index+1], &callbacks[index], (numPtrs-index)*sizeof(void *));
  callbacks[index]=callback;
  numPtrs++;
}

void PtrListBase::insertBefore(size_t index, void *first, size_t count)
{
	if (0 == count) return;
	if ((numPtrs + count) >= allocSize)
	{
		size_t x =allocSize;
		do
		{
			x*=2;
		}
		while ((numPtrs + count) >= x);
		reserve(x);
	}
	
	if (index >= numPtrs) index = numPtrs;
	else
		memmove(&callbacks[index+count], &callbacks[index], (numPtrs-index)*sizeof(void *));
		
	memcpy(&callbacks[index], first, count * sizeof(void*));
	numPtrs += count;
}

void PtrListBase::own(PtrListBase *victim)
{
	delete[] callbacks;
	callbacks = victim->callbacks;
	victim->callbacks=0;
	numPtrs = victim->numPtrs;
	victim->numPtrs=0;
	allocSize = victim->allocSize;
	victim->allocSize=0;
}

bool PtrListBase::findItem(void *callback, size_t *index)
{
	for (size_t i = 0; i != numPtrs;i++)
	{
		if (callbacks[i] == callback)
		{
			*index = i;
			return true;
		}
	}
	return false;
}

bool PtrListBase::findItem_reverse(void *callback, size_t *index)
{
	size_t i = numPtrs;
	while (i--)
	{
		if (callbacks[i] == callback)
		{
			*index = i;
			return true;
		}
	}
	return false;
}
