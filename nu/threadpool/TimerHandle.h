#pragma once
#if !defined(_WIN32_WINNT) || (_WIN32_WINNT < 0x400)
#error Must define _WIN32_WINNT >= 0x400 to use TimerHandle
#endif
#include <windows.h>
#include <bfc/platform/types.h>
/*
TimerHandle() constructor will make a new timer handle
TimerHandle(existing_handle) will "take over" an existing handle
~TimerHandle() DOES NOT CloseHandle as this object is meant as a helper
call Close() to kill the timer handle

The timer will be "one shot" auto-reset.  
Because it is meant to be compatible with the threadpool, manual-reset timers and periodic timers
are not recommended!!  You will have re-entrancy problems
If you want "periodic" behavior, call Wait() at the end of your ThreadPoolFunc
*/
class TimerHandle
{
public:
	TimerHandle();
	TimerHandle(HANDLE handle);
	void Close();
	void Wait(uint64_t milliseconds);
	void Poll(uint64_t milliseconds); // only use on a reserved thread!!! 
	/* TODO: WaitUntil method for absolute times */
	void Cancel();
	operator HANDLE();	
	
private:
	HANDLE timerHandle;
};