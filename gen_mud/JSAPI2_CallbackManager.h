#pragma once
#include "../nu/AutoLock.h"
#include "../nu/PtrList.h"
HANDLE DuplicateCurrentThread();
namespace JSAPI2
{
	template <class API> struct CallbackInfo
	{
		CallbackInfo()
		{
			api = 0;
			threadId = 0;
			threadHandle = 0;
		}

		CallbackInfo(API *me)
		{
			api = me;
			threadId = GetCurrentThreadId();
			threadHandle = DuplicateCurrentThread();
		}

		~CallbackInfo()
		{
			CloseHandle(threadHandle);
			threadHandle = 0;
		}
		API *api;
		DWORD threadId;
		HANDLE threadHandle;
	};

	class MUDAPI;
	class CallbackManager
	{
	public:
		CallbackManager();
		
	public:
		/** stuff for Winamp to call to trigger callbacks 
		** these are primarily responsible for getting over to the correct thread
		** to keep that particular logic out of the various functions
		*/
		void OnStatusChange(int new_status);
		
	public:
		/* stuff for other JSAPI2 classes to call */
		void Register(JSAPI2::MUDAPI *me);
		void Deregister(JSAPI2::MUDAPI *me);

	private:
		/* MUD API callbacks */
		typedef CallbackInfo<JSAPI2::MUDAPI> MUDCallback;
		typedef nu::PtrList<MUDCallback> CallbackList;
		CallbackList callbacks;

		Nullsoft::Utility::LockGuard callbackGuard;
	};

	extern CallbackManager callbackManager;
}