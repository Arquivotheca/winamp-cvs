//#include "main.h"
#include "./dispatchCallback.h"
#include <new.h>

DispatchCallback::DispatchCallback() 
	: ref(1), dispatch(NULL), threadId(0), threadHandle(NULL)
{
}

DispatchCallback::~DispatchCallback()
{
	if (NULL != dispatch)
		dispatch->Release();

	if (NULL != threadHandle)
		CloseHandle(threadHandle);
}


HRESULT DispatchCallback::CreateInstance(IDispatch *dispatch, DispatchCallback **instance)
{
	DispatchCallback *self;
	HANDLE processHandle;

	if (NULL == instance) 
		return E_POINTER;

	*instance = NULL;

	if (NULL == dispatch)
		return E_INVALIDARG;
	
	self = new DispatchCallback();
	if (NULL == self)
		return E_OUTOFMEMORY;

	self->dispatch = dispatch;
	self->dispatch->AddRef();
	self->threadId = GetCurrentThreadId();

	
	processHandle = GetCurrentProcess();

	if (FALSE == DuplicateHandle(processHandle, 
								 GetCurrentThread(), 
								 processHandle, 
								 &self->threadHandle, 
								 0, 
								 FALSE, 
								 DUPLICATE_SAME_ACCESS))
	{
		
		self->threadHandle = NULL;
		delete(self);

		return E_FAIL;
	}
	
	*instance = self;
	return S_OK;
}

unsigned long DispatchCallback::AddRef()
{
	return InterlockedIncrement((long*)&ref);
}

unsigned long DispatchCallback::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((long*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

IDispatch *DispatchCallback::GetDispatch()
{
	return dispatch;
}

unsigned long DispatchCallback::GetThreadId()
{
	return threadId;
}

HANDLE DispatchCallback::GetThreadHandle()
{
	return threadHandle;
}


DispatchCallbackEnum::DispatchCallbackEnum()
	: ref(1), buffer(NULL), size(0), cursor(0) 
{
}

DispatchCallbackEnum::~DispatchCallbackEnum()
{
	if (NULL != buffer)
	{
		while(size--)
		{
			buffer[size]->Release();
		}
	}
}


HRESULT DispatchCallbackEnum::CreateInstance(DispatchCallback **objects, size_t count, DispatchCallbackEnum **instance)
{
	size_t index, size;
	void *storage;
	DispatchCallback *callback;
	DispatchCallbackEnum *enumerator;
	
	if (NULL == instance) 
		return E_POINTER;

	*instance = NULL;

	size = sizeof(DispatchCallbackEnum) + (sizeof(DispatchCallback**) * count);
	storage = malloc(size);
	if (NULL == storage)
		return E_OUTOFMEMORY;
	
	enumerator = new(storage) DispatchCallbackEnum();
	if (NULL == enumerator)
	{
		free(storage);
		return E_FAIL;
	}

	enumerator->buffer = (DispatchCallback**)(((BYTE*)enumerator) + sizeof(DispatchCallback));
	
	for (index = 0; index < count; index++)
	{
		callback = objects[index];
		if (NULL != callback)
		{
			enumerator->buffer[enumerator->size] = callback;
			callback->AddRef();
			enumerator->size++;
		}
	}

	*instance = enumerator;
	return S_OK;
}

unsigned long DispatchCallbackEnum::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

unsigned long DispatchCallbackEnum::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

HRESULT DispatchCallbackEnum::Next(DispatchCallback **objects, size_t bufferMax, size_t *fetched)
{
	size_t available, copied, index;
	DispatchCallback **source;

	if (NULL == objects)
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
	
	source = buffer + cursor;
	CopyMemory(objects, source, copied * sizeof(DispatchCallback*));
    
	for(index = 0; index < copied; index++)
		objects[index]->AddRef();
	
	cursor += copied;

	if (NULL != fetched) 
		*fetched = copied;

	return (bufferMax == copied) ? S_OK : S_FALSE;
}

HRESULT DispatchCallbackEnum::Reset(void)
{
	cursor = 0;
	return S_OK;
}

HRESULT DispatchCallbackEnum::Skip(size_t count)
{
	cursor += count;
	if (cursor > size)
		cursor = size;
	
	return (cursor < size) ? S_OK : S_FALSE;
}

HRESULT DispatchCallbackEnum::GetCount(size_t *count)
{
	if (NULL == count)
		return E_POINTER;
	
	*count = size;

	return S_OK;
}

HRESULT DispatchCallbackEnum::Notify(DispatchCallbackNotifyFunc notifyCb, DispatchCallbackFreeFunc freeCb, void *param)
{	
	HRESULT hr;
	size_t index;
	unsigned long threadId;
	DispatchCallback *callback;
	DispatchCallbackApc *apc;

	threadId = GetCurrentThreadId();
	
	if (NULL == buffer)
		return E_UNEXPECTED;

	hr = DispatchCallbackApc::CreateInstance(notifyCb, freeCb, param, &apc);
	if (FAILED(hr))
		return hr;
	
	for (index = 0; index < size; index++)
	{
		callback = buffer[index];

		if (callback->GetThreadId() == threadId)
			apc->Call(callback->GetDispatch());
		else
			apc->Queue(callback->GetThreadHandle(), callback->GetDispatch());
	}

	apc->Release();
	return hr;
}


DispatchCallbackStore::DispatchCallbackStore()
{
	InitializeCriticalSection(&lock);
}

DispatchCallbackStore::~DispatchCallbackStore()
{
	UnregisterAll();
	DeleteCriticalSection(&lock);
}

void DispatchCallbackStore::Lock()
{
	EnterCriticalSection(&lock);
}

void DispatchCallbackStore::Unlock()
{
	LeaveCriticalSection(&lock);
}

CRITICAL_SECTION *DispatchCallbackStore::GetLock()
{
	return &lock;
}

HRESULT DispatchCallbackStore::Register(IDispatch *dispatch)
{
	size_t index;
	HRESULT hr;
	DispatchCallback *callback;
	
	if (NULL == dispatch)
		return E_INVALIDARG;

	Lock();
	
	hr = S_OK;

	index = list.size();
	while(index--)
	{
		callback = list[index];
		if (callback->GetDispatch() == dispatch)
		{			
			hr = S_FALSE;
			break;
		}
	}

	if (S_OK == hr)
	{
		hr = DispatchCallback::CreateInstance(dispatch, &callback);
		if (SUCCEEDED(hr))
			list.push_back(callback);
	}
	
	Unlock();

	return hr;
}

HRESULT DispatchCallbackStore::Unregister(IDispatch *dispatch)
{
	size_t index;
	HRESULT hr;
	DispatchCallback *callback;
	
	if (NULL == dispatch)
		return E_INVALIDARG;

	Lock();
	
	hr = S_FALSE;

	index = list.size();
	while(index--)
	{
		callback = list[index];
		if (callback->GetDispatch() == dispatch)
		{
			list.eraseindex(index);
			callback->Release();
			hr = S_OK;
			break;
		}
	}
	
	Unlock();

	return hr;
}

void DispatchCallbackStore::UnregisterAll()
{
	size_t index;
	DispatchCallback *callback;
	
	Lock();
	
	index = list.size();
	while(index--)
	{
		callback = list[index];
		callback->Release();
	}
	
	list.clear();
	
	Unlock();
}


HRESULT DispatchCallbackStore::Enumerate(DispatchCallbackEnum **enumerator)
{
	HRESULT hr;

	if (NULL == enumerator)
		return E_POINTER;

	Lock();

	hr = DispatchCallbackEnum::CreateInstance(list.begin(), list.size(), enumerator);
		
	Unlock();

	return hr;
}


HRESULT DispatchCallbackStore::RegisterFromDispParam(DISPPARAMS *pdispparams, unsigned int position, 
													 unsigned int *puArgErr)
{
	VARIANTARG  varg;
	HRESULT hr;

	VariantInit(&varg);
	hr = DispGetParam(pdispparams, position, VT_DISPATCH, &varg, puArgErr);
	if (SUCCEEDED(hr))
	{
		hr = Register(V_DISPATCH(&varg));
		VariantClear(&varg);
	}

	return hr;
}

HRESULT DispatchCallbackStore::UnregisterFromDispParam(DISPPARAMS *pdispparams, unsigned int position, 
													   unsigned int *puArgErr)
{
	VARIANTARG  varg;
	HRESULT hr;

	VariantInit(&varg);
	hr = DispGetParam(pdispparams, position, VT_DISPATCH, &varg, puArgErr);
	if (SUCCEEDED(hr))
	{
		hr = Unregister(V_DISPATCH(&varg));
		VariantClear(&varg);
	}

	return hr;
}

HRESULT DispatchCallbackStore::Notify(DispatchCallbackNotifyFunc notifyCb, DispatchCallbackFreeFunc freeCb, void *param)
{	
	HRESULT hr;
	DispatchCallbackEnum *enumerator;

	hr = Enumerate(&enumerator);
	if (SUCCEEDED(hr))
	{
		hr = enumerator->Notify(notifyCb, freeCb, param);
		enumerator->Release();
	}

	return hr;
}


DispatchCallbackApc::DispatchCallbackApc()
	: ref(1), notifyCb(NULL), freeCb(NULL), param(NULL)
{
}

DispatchCallbackApc::~DispatchCallbackApc()
{
	if (NULL != freeCb)
		freeCb(param);
}

HRESULT DispatchCallbackApc::CreateInstance(DispatchCallbackNotifyFunc notifyCb, DispatchCallbackFreeFunc freeCb,
								  void *param, DispatchCallbackApc **instance)
{

	DispatchCallbackApc *self;

	if (NULL == instance) 
		return E_POINTER;

	*instance = NULL;

	if (NULL == notifyCb)
		return E_INVALIDARG;

	self = new DispatchCallbackApc();
	if (NULL == self)
		return E_OUTOFMEMORY;

	self->notifyCb = notifyCb;
	self->freeCb = freeCb;
	self->param = param;

	*instance = self;

	return S_OK;
}

unsigned long DispatchCallbackApc::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

unsigned long DispatchCallbackApc::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

HRESULT DispatchCallbackApc::Call(IDispatch *dispatch)
{	
	if (NULL == notifyCb)
		return E_UNEXPECTED;

	notifyCb(dispatch, param);
	return S_OK;
}

HRESULT DispatchCallbackApc::Queue(HANDLE threadHandle, IDispatch *dispatch)
{
	DispatchCallbackApcParam *apcParam;
	
	if (NULL == threadHandle)
		return E_INVALIDARG;

	apcParam = new DispatchCallbackApcParam(dispatch, this);
	if (NULL == apcParam)
		return E_OUTOFMEMORY;

	if (0 == QueueUserAPC(QueueApcCallback, threadHandle, (ULONG_PTR)apcParam))
	{
		unsigned long errorCode;

		errorCode = GetLastError();
		free(apcParam);

		return HRESULT_FROM_WIN32(errorCode);
	}

	return S_OK;
}

void CALLBACK DispatchCallbackApc::QueueApcCallback(ULONG_PTR user)
{
	DispatchCallbackApcParam *apcParam;
	DispatchCallbackApc *apc;

	apcParam = (DispatchCallbackApcParam*)user;
	if (NULL == apcParam)
		return;
	
	apc = apcParam->GetApc();
	if (NULL != apc)
		apc->Call(apcParam->GetDispatch()), 

	delete(apcParam);
}

DispatchCallbackApcParam::DispatchCallbackApcParam(IDispatch *_dispatch, DispatchCallbackApc *_apc)
	: dispatch(_dispatch), apc(_apc)
{
	if (NULL != dispatch)
		dispatch->AddRef();

	if (NULL != apc)
		apc->AddRef();
}

DispatchCallbackApcParam::~DispatchCallbackApcParam()
{
	if (NULL != dispatch)
		dispatch->Release();

	if (NULL != apc)
		apc->Release();
}


IDispatch *DispatchCallbackApcParam::GetDispatch()
{
	return dispatch;
}

DispatchCallbackApc *DispatchCallbackApcParam::GetApc()
{
	return apc;
}
