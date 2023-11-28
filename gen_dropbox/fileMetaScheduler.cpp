#include "fileMetaScheduler.h"
#include "./fileInfoInterface.h"
#include "./fileMetaInterface.h"
#include "../nu/threadname.h"
#include "../nu/trace.h"

#define METATHREAD_KILL		0
#define METATHREAD_WAKE		1

static DWORD CALLBACK Discovery_ThreadProc(LPVOID param)
{
	SetThreadName(-1, "MetaScheduler Discovery Thread");
	FileMetaScheduler* pScheduler = (FileMetaScheduler*)param;
	return (NULL != pScheduler) ? pScheduler->DiscoveryThread() : 1;
}

FileMetaScheduler::FileMetaScheduler() : 
	hThread(NULL), hWakeUp(NULL), hKill(NULL), queueLimit(0), idleState(TRUE)
{
	InitializeCriticalSection(&lockQueue);
	InitializeCriticalSection(&lockListeners);
	
}

FileMetaScheduler::~FileMetaScheduler()
{
	KillThread();
	TruncateQueue(0);
	DeleteCriticalSection(&lockQueue);
	DeleteCriticalSection(&lockListeners);
}


BOOL FileMetaScheduler::InitializeThread()
{
	if (NULL != hThread)
		return TRUE;

	DWORD threadId;
	if (NULL == hWakeUp) hWakeUp = CreateEvent(0, FALSE, FALSE, 0);
	if (NULL == hKill) hKill = CreateEvent(0, TRUE, FALSE, 0);
	if (NULL != hWakeUp && NULL != hKill)
	{
		hThread = CreateThread(NULL, 0, Discovery_ThreadProc, (LPVOID)this, 0, &threadId);
		if (NULL != hThread) 
			return TRUE;
	}

	KillThread();
	return FALSE;
}

void FileMetaScheduler::KillThread()
{
	if (NULL != hThread)
	{
		if (NULL != hKill)
		{
			SetEvent(hKill);
			WaitForSingleObject(hThread, INFINITE);

		}
		CloseHandle(hThread); 
		hThread = NULL;
	}

	if (NULL != hKill) 
	{ 
		CloseHandle(hKill); 
		hKill = NULL; 
	}

	if (NULL != hWakeUp) 
	{ 
		CloseHandle(hWakeUp);
		hWakeUp = NULL; 
	}
}

HRESULT FileMetaScheduler::ScheduleRead(IFileMetaReader *pReader)
{
	return ScheduleArrayRead(&pReader, 1, FALSE);
}

HRESULT FileMetaScheduler::ScheduleArrayRead(IFileMetaReader **pReaders, size_t readerCount, BOOL bReversePush)
{
	if (NULL == pReaders)
		return E_INVALIDARG;

	if (NULL == InitializeThread())
		return E_UNEXPECTED;
	
	if (readerCount < 1)
		return S_OK;

	BOOL bSizeChanged;
	
	EnterCriticalSection(&lockQueue);
	size_t before  = schedule.size();
	size_t startIndex = 0;
	
	if (queueLimit > 0)
	{
		if (readerCount > queueLimit)
		{
			startIndex = readerCount - queueLimit;
			readerCount = queueLimit;
		}

		IFileMetaReader *pr2;
		size_t limit = before;
		while(limit-- > (queueLimit -readerCount))
		{
			pr2 = schedule.back();
			schedule.pop_back();
			OnReadCompleted(pr2, E_ABORT);
			pr2->Release();
		}
	}
	
	if (!bReversePush)
		for(size_t i = startIndex; i < readerCount; i++) 
		{ 
			schedule.push_front(pReaders[i]); 
			pReaders[i]->AddRef(); 
		}
	else
		for(size_t i = readerCount; i-- > startIndex;) 
		{ 			
			schedule.push_front(pReaders[i]); 
			pReaders[i]->AddRef(); 
		}
	
	bSizeChanged = (before != schedule.size());
	
	if (schedule.size() > 0)
		idleState = FALSE;

	LeaveCriticalSection(&lockQueue);

	if (bSizeChanged) OnQueueSizeChanged();

	SetEvent(hWakeUp);

	return S_OK;
}

size_t FileMetaScheduler::GetQueueSize(void)
{
	return schedule.size();
}

void FileMetaScheduler::SetQueueLimit(size_t newLimit)
{	
	queueLimit = newLimit;
}

void FileMetaScheduler::TruncateQueue(size_t nMax)
{
	BOOL bTruncated = FALSE;
	IFileMetaReader *pReader;

	EnterCriticalSection(&lockQueue);
	while(schedule.size() > nMax)
	{
		pReader = schedule.back();
		schedule.pop_back();
		if (NULL != pReader)
		{	
			OnReadCompleted(pReader, E_ABORT);
			pReader->Release();
		}
		bTruncated = TRUE;
	}
	LeaveCriticalSection(&lockQueue);
	if (bTruncated) OnQueueSizeChanged();
	
}

void FileMetaScheduler::RegisterListener(FileMetaSchedulerListener *pListener)
{
	EnterCriticalSection(&lockListeners);
	listener = pListener;
	LeaveCriticalSection(&lockListeners);
}


void FileMetaScheduler::OnReadCompleted(IFileMetaReader *pReader, HRESULT hr)
{
	EnterCriticalSection(&lockListeners);
	if (NULL != listener)
		listener->ReadCompleted(this, pReader, hr);
	LeaveCriticalSection(&lockListeners);
}

void FileMetaScheduler::OnQueueSizeChanged()
{
	EnterCriticalSection(&lockListeners);
	if (NULL != listener)
		listener->QueueSizeChanged(this);
	LeaveCriticalSection(&lockListeners);
}

void FileMetaScheduler::OnIdle()
{
	EnterCriticalSection(&lockListeners);
	if (NULL != listener)
		listener->Idle(this);
	LeaveCriticalSection(&lockListeners);
}

BOOL FileMetaScheduler::PerformRead()
{
	for (;;)
	{
		if (WAIT_OBJECT_0 == WaitForSingleObject(hKill, 0))
			return FALSE;

		EnterCriticalSection(&lockQueue);
		if (schedule.empty()) 
		{
			idleState = TRUE;
			LeaveCriticalSection(&lockQueue);
			return TRUE;
		}
		
		IFileMetaReader *pReader;
		pReader = schedule.back();
		schedule.pop_back();
		LeaveCriticalSection(&lockQueue);

		OnQueueSizeChanged();

		if (NULL != pReader)
		{	
			HRESULT hr = pReader->Read();
			OnReadCompleted(pReader, hr);
			pReader->Release();
		}
		
	}

	return TRUE;
}

DWORD FileMetaScheduler::DiscoveryThread()
{
	HANDLE hEvents[] = { hKill, hWakeUp };
	SetThreadPriority(hThread, THREAD_PRIORITY_NORMAL);
	DWORD sleepMs = 10 * 1000;
	for(;;)
	{
		switch (WaitForMultipleObjectsEx(2, hEvents, FALSE, sleepMs, TRUE))
		{
			case WAIT_OBJECT_0: // kill switch
				return 0;

			case WAIT_OBJECT_0+1: // job
				sleepMs = (PerformRead()) ? (10 * 1000) : INFINITE;
				break;

			case WAIT_TIMEOUT:
				if (schedule.empty())
					OnIdle();
				else
					SetEvent(hWakeUp);
				break;
		}
	}
	return 0;
}

BOOL FileMetaScheduler::IsIdle()
{
	return idleState;
}
