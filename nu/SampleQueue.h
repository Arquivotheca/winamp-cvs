#pragma once
#include <bfc/platform/types.h>
#include "../nu/PtrDeque2.h"
#include "../nu/AutoLock.h"

template <class SampleData>
class SampleQueue
{
public:
	void PushFree(SampleData *new_sample)
	{
		queue_guard.Lock();
		free_queue.push_front(new_sample);
		queue_guard.Unlock();
	}

	void PushProcessed(SampleData *new_sample)
	{
		queue_guard.Lock();
		processed_queue.push_front(new_sample);
		queue_guard.Unlock();
	}

	// will return 0 if none ready
	SampleData *PopProcessed()
	{
		SampleData *sample=0;
		queue_guard.Lock();
		if (!processed_queue.empty())
		{
			sample = processed_queue.back();
			processed_queue.pop_back();
		}
		queue_guard.Unlock();
		return sample;
	}

	SampleData *PopFree()
	{
		SampleData *sample=0;
		queue_guard.Lock();
		if (!free_queue.empty())
		{
			sample = free_queue.back();
			free_queue.pop_back();
		}
		queue_guard.Unlock();
		if (!sample)
			sample = new SampleData;
		return sample;
	}

	void Trim()
	{
		queue_guard.Lock();
		free_queue.deleteAll();
		processed_queue.deleteAll();
		queue_guard.Unlock();
	}

private:
	nu::PtrDeque2<SampleData> free_queue;
	nu::PtrDeque2<SampleData> processed_queue;

	Nullsoft::Utility::LockGuard queue_guard;
};
