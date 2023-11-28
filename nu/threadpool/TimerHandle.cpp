#include "TimerHandle.h"

TimerHandle::TimerHandle()
{
	timerHandle = CreateWaitableTimer(0, FALSE, 0);
}

TimerHandle::TimerHandle(HANDLE handle)
{
	timerHandle = handle;
}

void TimerHandle::Close()
{
	CloseHandle(timerHandle);
}

void TimerHandle::Poll(uint64_t milliseconds)
{
		/* MSDN notes about SetWaitableTimer: 100 nanosecond resolution, Negative values indicate relative time*/
	LARGE_INTEGER timeout;
	timeout.QuadPart = - ((int64_t)milliseconds * 1000LL /*to microseconds*/ * 10LL /* to 100 nanoseconds */);
	SetWaitableTimer(timerHandle, &timeout, (LONG)milliseconds, 0, 0, FALSE);
}

void TimerHandle::Wait(uint64_t milliseconds)
{
	/* MSDN notes about SetWaitableTimer: 100 nanosecond resolution, Negative values indicate relative time*/
	LARGE_INTEGER timeout;
	timeout.QuadPart = - ((int64_t)milliseconds * 1000LL /*to microseconds*/ * 10LL /* to 100 nanoseconds */);
	SetWaitableTimer(timerHandle, &timeout, 0, 0, 0, FALSE);
}

void TimerHandle::Cancel()
{
	CancelWaitableTimer(timerHandle);
}

TimerHandle::operator HANDLE()
{
	return timerHandle;
}