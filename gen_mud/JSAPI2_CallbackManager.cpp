#include "JSAPI2_CallbackManager.h"
#include "jsapi2_mudapi.h"

HANDLE DuplicateCurrentThread()
{
	HANDLE fakeHandle = GetCurrentThread();
	HANDLE copiedHandle = 0;
	HANDLE processHandle = GetCurrentProcess();
	DuplicateHandle(processHandle, fakeHandle, processHandle, &copiedHandle, 0, FALSE, DUPLICATE_SAME_ACCESS);
	return copiedHandle;
}

JSAPI2::CallbackManager JSAPI2::callbackManager;
JSAPI2::CallbackManager::CallbackManager()
: callbackGuard("JSAPI2::CallbackManager::callbackGuard")
{
}

void JSAPI2::CallbackManager::Register(JSAPI2::MUDAPI *me)
{
	/* benski> important note:
	even thought JSAPI2::Transport inherits from IUnknown,
	we don't call AddRef here!
	because this would introduce a circular reference.
	JSAPI2::TransportAPI will call Deregister during it's 
	destructor.
	*/
	Nullsoft::Utility::AutoLock lock(callbackGuard);
	callbacks.push_back(new MUDCallback(me));
}

void JSAPI2::CallbackManager::Deregister(JSAPI2::MUDAPI *me)
{
	/* benski> important note:
	even thought JSAPI2::Transport inherits from IUnknown,
	we don't call Release here!
	because this would introduce a circular reference.
	JSAPI2::TransportAPI will call Deregister during it's 
	destructor.
	*/
	Nullsoft::Utility::AutoLock lock(callbackGuard);
	for (size_t i=0;i!=callbacks.size();i++)
	{
		MUDCallback *callback = callbacks[i];
		if (callback->api == me)
		{
			delete callback;
			callbacks.eraseindex(i);
			i--;
		}
	}
}


/* --- OnStop --- */
struct OnStatusAPCData
{
	JSAPI2::MUDAPI *mud;
	int new_status;
};

static void CALLBACK CMGR_OnStatusAPC(ULONG_PTR param)
{
	OnStatusAPCData *data = (OnStatusAPCData *)param;
	data->mud->OnStatusChange(data->new_status);
	data->mud->Release();
	delete data;
}

void JSAPI2::CallbackManager::OnStatusChange(int new_status)
{
	DWORD threadId = GetCurrentThreadId();
	Nullsoft::Utility::AutoLock lock(callbackGuard);
	for (size_t i=0;i!=callbacks.size();i++)
	{
		MUDCallback *callback = callbacks[i];

		OnStatusAPCData *data = new OnStatusAPCData;
		data->mud = callback->api;
		data->new_status = new_status;
		data->mud->AddRef(); // so it doesn't disappear while we're switching threads
		if (threadId == callback->threadId)
		{
			// same thread! huzzah but I wonder how that happened :)
			CMGR_OnStatusAPC((ULONG_PTR)data);
		}
		else
		{
			// different thread, do an APC
			if (QueueUserAPC(CMGR_OnStatusAPC, callbacks[i]->threadHandle, (ULONG_PTR)data) == 0)
			{			
				data->mud->Release();
				delete data;
			}
		}
	}
}
/* --- --- */
