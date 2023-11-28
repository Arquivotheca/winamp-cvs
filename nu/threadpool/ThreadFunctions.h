#pragma once
#include "api_threadpool.h"
#include "../PtrMap.h"
#include "../AutoLock.h"
#include "../PtrDeque2.h"

class ThreadFunctions
{
public:
	struct Data : public nu::PtrDequeNode
	{
		api_threadpool::ThreadPoolFunc func;
		void *user_data;
		intptr_t id;
	};
	ThreadFunctions(int create_function_list=1);
	~ThreadFunctions();
	void Add(HANDLE handle, api_threadpool::ThreadPoolFunc func, void *user_data, intptr_t id);
	bool Get(HANDLE handle, api_threadpool::ThreadPoolFunc *func, void **user_data, intptr_t *id);
	void QueueFunction(api_threadpool::ThreadPoolFunc func, void *user_data, intptr_t id);
	bool PopFunction(api_threadpool::ThreadPoolFunc *func, void **user_data, intptr_t *id);

	typedef PtrMap<HANDLE, ThreadFunctions::Data> DataMap;
	DataMap data;
	Nullsoft::Utility::LockGuard guard;

	typedef nu::PtrDeque2<ThreadFunctions::Data> FuncList;
	FuncList functions_list;
	CRITICAL_SECTION functions_guard;
	HANDLE functions_semaphore;
};
